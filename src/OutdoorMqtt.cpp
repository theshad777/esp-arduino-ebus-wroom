#include "OutdoorMqtt.hpp"

#include "ConfigManager.hpp"
#include "Logger.hpp"
#include "OutdoorTemp.hpp"

#if defined(OUTDOOR_DS_GPIO) && !defined(EBUS_INTERNAL)

#include <cstdio>
#include <string>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <mqtt_client.h>

namespace {
esp_mqtt_client_handle_t client = nullptr;
volatile bool connected = false;
volatile bool taskStarted = false;
const char* mqttState = "not_started";

std::string brokerUri;
std::string username;
std::string password;
std::string clientId;
std::string availabilityTopic;
std::string temperatureTopic;
std::string stateTopic;

void publishRetained(const std::string& topic, const char* payload) {
  if (client == nullptr || !connected) return;
  esp_mqtt_client_publish(client, topic.c_str(), payload, 0, 1, 1);
}

void mqttEvent(void*, esp_event_base_t, int32_t eventId, void*) {
  switch (eventId) {
    case MQTT_EVENT_CONNECTED:
      connected = true;
      mqttState = "connected";
      logger.info("Outdoor MQTT connected");
      break;
    case MQTT_EVENT_DISCONNECTED:
      connected = false;
      mqttState = "disconnected";
      break;
    case MQTT_EVENT_ERROR:
      mqttState = "error";
      logger.warn("Outdoor MQTT error");
      break;
    default:
      break;
  }
}

bool initClient(const char* uniqueId) {
  if (!configManager.readBool("mqttEnabled")) {
    mqttState = "disabled";
    return false;
  }

  const std::string server = configManager.readString("mqttServer");
  if (server.empty()) {
    mqttState = "no_server";
    logger.warn("Outdoor MQTT: mqttServer is empty");
    return false;
  }

  username = configManager.readString("mqttUser");
  password = configManager.readString("mqttPass");

  std::string root = configManager.readString("rootTopic", "");
  if (root.empty()) root = std::string("ebus/") + uniqueId + "/";
  if (root.back() != '/') root += '/';

  availabilityTopic = root + "outdoor/availability";
  temperatureTopic = root + "outdoor/temperature";
  stateTopic = root + "outdoor/state";

  // Accept either a complete URI (mqtt://host:port) or a plain host[:port].
  brokerUri = server.find("://") != std::string::npos
                  ? server
                  : std::string("mqtt://") + server;
  clientId = std::string("ebus-outdoor-") + uniqueId;

  esp_mqtt_client_config_t cfg{};
  cfg.broker.address.uri = brokerUri.c_str();
  cfg.credentials.client_id = clientId.c_str();
  if (!username.empty()) cfg.credentials.username = username.c_str();
  if (!password.empty())
    cfg.credentials.authentication.password = password.c_str();

  cfg.session.last_will.topic = availabilityTopic.c_str();
  cfg.session.last_will.msg = "offline";
  cfg.session.last_will.qos = 1;
  cfg.session.last_will.retain = 1;

  client = esp_mqtt_client_init(&cfg);
  if (client == nullptr) {
    mqttState = "init_error";
    logger.error("Outdoor MQTT: esp_mqtt_client_init failed");
    return false;
  }

  esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqttEvent, nullptr);
  const esp_err_t err = esp_mqtt_client_start(client);
  if (err != ESP_OK) {
    mqttState = "start_error";
    logger.error("Outdoor MQTT: esp_mqtt_client_start failed");
    return false;
  }

  mqttState = "connecting";
  logger.info("Outdoor MQTT telemetry topic: " + temperatureTopic);
  return true;
}

void telemetryTask(void* arg) {
  const char* uniqueId = static_cast<const char*>(arg);

  // Keep MQTT startup out of the critical early-boot path. Wi-Fi, HTTP,
  // ESPOTA, eBUS and the DS18B20 reader get several seconds to settle first.
  vTaskDelay(pdMS_TO_TICKS(5000));

  if (!initClient(uniqueId)) {
    taskStarted = false;
    vTaskDelete(nullptr);
    return;
  }

  for (;;) {
    if (connected) {
      const auto sample = getOutdoorTemperatureSnapshot();
      publishRetained(availabilityTopic, sample.valid ? "online" : "offline");
      publishRetained(stateTopic, sample.state);

      if (sample.valid) {
        char value[32];
        std::snprintf(value, sizeof(value), "%.3f",
                      static_cast<double>(sample.temperatureC));
        publishRetained(temperatureTopic, value);
      } else {
        // Clear a retained temperature rather than leaving a stale value.
        publishRetained(temperatureTopic, "");
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
}  // namespace

void startOutdoorMqtt(const char* uniqueId) {
  if (taskStarted) return;
  taskStarted = true;
  mqttState = "waiting";

  if (xTaskCreate(telemetryTask, "outdoor_mqtt", 4096,
                  const_cast<char*>(uniqueId), 1, nullptr) != pdPASS) {
    taskStarted = false;
    mqttState = "task_error";
    logger.error("Outdoor MQTT: telemetry task creation failed");
  }
}

const char* getOutdoorMqttState() { return mqttState; }

const char* getOutdoorMqttTemperatureTopic() {
  return temperatureTopic.empty() ? "" : temperatureTopic.c_str();
}

#else

void startOutdoorMqtt(const char*) {}
const char* getOutdoorMqttState() { return "disabled"; }
const char* getOutdoorMqttTemperatureTopic() { return ""; }

#endif
