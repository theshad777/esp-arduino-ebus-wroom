# DS18B20 bench test (ESP32-WROOM)

- Baseline: branch esp32-wroom-port, commit 7cc7166. Apply changes ONLY in a new feature branch.
- GPIO16: DS18B20 DATA; sensor supplied from ESP32 3V3 and GND. Existing external pull-up module must pull DATA up to 3.3 V, **not 5 V**.
- eBUS GPIO17 TX and GPIO34 RX are unchanged. Relay is **not controlled** and remains physically disconnected.
- Adds Espressif `onewire_bus` RMT backend and `ds18b20` components to `src/idf_component.yml`.
- Sample interval ~5 seconds. Log messages in the existing logger (`Outdoor DS18B20: ... C`).
- An 800ms task delay permits a 12-bit conversion without blocking the main task. Hardware RMT handles 1-Wire slots.
- If no sensor is found at boot, retries every 5 seconds. A missing sensor after successful discovery logs a read error; this bench-test module does not yet re-enumerate after disconnect.
- This is a **source patch, not a validated firmware build**: external ESP-IDF dependencies and local PlatformIO build have not been run here.
- Before flashing, compare `git diff`, build `pio run -e esp32-wroom` and keep a backup of current working firmware. Do not connect the boiler output while bench testing.
