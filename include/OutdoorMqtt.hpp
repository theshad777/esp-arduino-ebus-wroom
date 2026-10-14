#pragma once

// WROOM telemetry-only MQTT publisher for the outdoor DS18B20.
// It never subscribes to MQTT and never changes the relay state.
void startOutdoorMqtt(const char* uniqueId);
const char* getOutdoorMqttState();
const char* getOutdoorMqttTemperatureTopic();
