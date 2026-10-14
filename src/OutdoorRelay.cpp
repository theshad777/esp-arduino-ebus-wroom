#include "OutdoorRelay.hpp"
#include <driver/gpio.h>
#include <esp_err.h>
#include "Logger.hpp"

namespace {
volatile bool relayEnabled = false;
}

void initOutdoorRelay() {
#if defined(OUTDOOR_RELAY_GPIO)
  const gpio_num_t pin = static_cast<gpio_num_t>(OUTDOOR_RELAY_GPIO);
  // Set output latch LOW *before* switching pin to output mode.
  gpio_set_level(pin, 0);
  gpio_config_t config{};
  config.pin_bit_mask = 1ULL << OUTDOOR_RELAY_GPIO;
  config.mode = GPIO_MODE_OUTPUT;
  config.pull_up_en = GPIO_PULLUP_DISABLE;
  config.pull_down_en = GPIO_PULLDOWN_ENABLE;
  config.intr_type = GPIO_INTR_DISABLE;
  const esp_err_t err = gpio_config(&config);
  relayEnabled = false;
  if (err != ESP_OK) logger.error("Outdoor relay: GPIO initialization failed");
  else logger.info("Outdoor relay: GPIO initialized LOW, relay OFF");
#endif
}

bool setOutdoorRelay(bool enabled) {
#if defined(OUTDOOR_RELAY_GPIO)
  const esp_err_t err = gpio_set_level(static_cast<gpio_num_t>(OUTDOOR_RELAY_GPIO), enabled ? 1 : 0);
  if (err != ESP_OK) return false;
  relayEnabled = enabled;
  return true;
#else
  (void)enabled;
  return false;
#endif
}
bool isOutdoorRelayEnabled() { return relayEnabled; }
