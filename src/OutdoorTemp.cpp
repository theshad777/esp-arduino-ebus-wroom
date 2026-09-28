#include "OutdoorTemp.hpp"

#if defined(OUTDOOR_DS_GPIO)
#include <cmath>
#include <string>

#include <esp_err.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Logger.hpp"
#include "ds18b20.h"
#include "onewire_bus.h"

namespace {
constexpr int64_t kStaleUs = 15LL * 1000000LL;
portMUX_TYPE snapshotMux = portMUX_INITIALIZER_UNLOCKED;
float lastTemperatureC = 0.0f;
int64_t lastSuccessUs = 0;
const char* currentState = "starting";
bool currentValid = false;

void setOutdoorState(const char* state) {
  portENTER_CRITICAL(&snapshotMux);
  currentState = state;
  currentValid = false;
  portEXIT_CRITICAL(&snapshotMux);
}

void setOutdoorReading(float temperature) {
  const int64_t now = esp_timer_get_time();
  portENTER_CRITICAL(&snapshotMux);
  lastTemperatureC = temperature;
  lastSuccessUs = now;
  currentState = "ok";
  currentValid = true;
  portEXIT_CRITICAL(&snapshotMux);
}

constexpr TickType_t kRetry = pdMS_TO_TICKS(5000);
constexpr TickType_t kConversion = pdMS_TO_TICKS(800); // 12-bit: <=750 ms
constexpr TickType_t kBetweenSamples = pdMS_TO_TICKS(4200);

void readerTask(void*) {
  onewire_bus_handle_t bus = nullptr;
  onewire_bus_config_t busConfig{};
  busConfig.bus_gpio_num = static_cast<gpio_num_t>(OUTDOOR_DS_GPIO);
  busConfig.flags.en_pull_up = false; // External pull-up is on the user's module
  onewire_bus_rmt_config_t rmtConfig{};
  rmtConfig.max_rx_bytes = 10;
  esp_err_t err = onewire_new_bus_rmt(&busConfig, &rmtConfig, &bus);
  if (err != ESP_OK) {
    logger.error(std::string("Outdoor DS18B20: 1-Wire RMT init failed: ") + esp_err_to_name(err));
    setOutdoorState("init_error");
    vTaskDelete(nullptr);
    return;
  }
  logger.info("Outdoor DS18B20: 1-Wire started on GPIO " +
              std::to_string(OUTDOOR_DS_GPIO));

  // Discovery and measurement never block the eBUS main task.
  ds18b20_device_handle_t sensor = nullptr;
  while (true) {
    if (sensor == nullptr) {
      onewire_device_iter_handle_t iter = nullptr;
      err = onewire_new_device_iter(bus, &iter);
      if (err == ESP_OK) {
        onewire_device_t candidate{};
        while ((err = onewire_device_iter_get_next(iter, &candidate)) == ESP_OK) {
          ds18b20_config_t config{};
          if (ds18b20_new_device_from_enumeration(&candidate, &config, &sensor) == ESP_OK) {
            logger.info("Outdoor DS18B20: sensor found");
            break;
          }
        }
        onewire_del_device_iter(iter);
      }
      if (sensor == nullptr) {
        logger.warn("Outdoor DS18B20: sensor not found, retrying in 5 s");
        setOutdoorState("sensor_not_found");
        vTaskDelay(kRetry);
        continue;
      }
    }

    err = ds18b20_trigger_temperature_conversion_for_all(bus);
    if (err == ESP_OK) {
      vTaskDelay(kConversion);
      float temperature = NAN;
      err = ds18b20_get_temperature(sensor, &temperature);
      if (err == ESP_OK && std::isfinite(temperature) &&
          temperature >= -55.0f && temperature <= 125.0f) {
        setOutdoorReading(temperature);
        logger.info("Outdoor DS18B20: " + std::to_string(temperature) + " C");
      } else {
        logger.warn("Outdoor DS18B20: invalid temperature/read error");
        setOutdoorState("read_error");
      }
    } else {
      logger.warn(std::string("Outdoor DS18B20: conversion failed: ") +
                  esp_err_to_name(err));
      setOutdoorState("read_error");
    }
    vTaskDelay(kBetweenSamples);
  }
}
} // namespace

OutdoorTemperatureSnapshot getOutdoorTemperatureSnapshot() {
  OutdoorTemperatureSnapshot result{};
  int64_t lastUs = 0;
  portENTER_CRITICAL(&snapshotMux);
  result.valid = currentValid;
  result.temperatureC = lastTemperatureC;
  result.state = currentState;
  lastUs = lastSuccessUs;
  portEXIT_CRITICAL(&snapshotMux);
  if (lastUs != 0) {
    const int64_t elapsed = esp_timer_get_time() - lastUs;
    result.ageSeconds = elapsed > 0 ? static_cast<uint32_t>(elapsed / 1000000LL) : 0;
    if (result.valid && elapsed > kStaleUs) {
      result.valid = false;
      result.state = "stale";
    }
  }
  return result;
}

void startOutdoorTemperatureReader() {
  const BaseType_t ok = xTaskCreate(readerTask, "outdoor_ds18b20", 4096,
                                     nullptr, 1, nullptr);
  if (ok != pdPASS) {
    logger.error("Outdoor DS18B20: failed to create task");
    setOutdoorState("init_error");
  }
}
#else
void startOutdoorTemperatureReader() {}
OutdoorTemperatureSnapshot getOutdoorTemperatureSnapshot() {
  return {false, 0.0f, 0, "disabled"};
}
#endif
