# Minimal Outdoor Relay bench patch (GPIO18)

This patch is based on the known-good DS18B20 + Status version and intentionally contains **no OutdoorMqtt client**.

Changes:
- GPIO18 controls BC337/TQ2-5V.
- Relay initializes LOW (de-energized) at boot.
- Status `Outdoor_Temperature` adds:
  - `Relay_Enabled`
  - `Relay_GPIO`
- No automatic temperature logic.
- No HTTP/MQTT command can turn the relay on.
- Existing DS18B20 reader on GPIO16 remains unchanged.

Expected after boot:
- `State: ok` for DS18B20 (if sensor is healthy)
- `Relay_Enabled: false`
- `Relay_GPIO: 18`

The boiler/NTC output should remain disconnected during this bench test.
