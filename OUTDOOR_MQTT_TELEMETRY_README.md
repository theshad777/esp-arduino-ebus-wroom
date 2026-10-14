# Outdoor MQTT telemetry-only patch

Base: stable DS18B20 + GPIO18 relay support.

This patch adds only MQTT publication of the DS18B20 temperature. It does not
subscribe to MQTT, does not accept commands, and does not change the relay.

Topics (using configured rootTopic, or ebus/<unique_id>/ by default):

- outdoor/temperature  retained, QoS 1
- outdoor/availability retained, QoS 1
- outdoor/state retained, QoS 1

Startup is delayed by 5 seconds to keep MQTT out of the critical boot path.
Publishing happens every 10 seconds.

Status gains:
- MQTT_State
- MQTT_Topic

Expected MQTT_State progression when enabled and configured:
waiting -> connecting -> connected

Other possible states:
disabled, no_server, disconnected, error, init_error, start_error, task_error

No platformio.ini is included intentionally: keep the current working ESPOTA
settings and GPIO definitions unchanged.
