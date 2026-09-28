# DS18B20 on Status — bench test

Overlay these files onto `C:\Users\Shad\GIT\esp-arduino-ebus-wroom` in branch `feature/outdoor-temp-relay`.

Changed files: `include/OutdoorTemp.hpp`, `src/OutdoorTemp.cpp`, `src/main.cpp`, `static/status.html`.

Run `pio run -e esp32-wroom` first; this patch has NOT been compiled here. It does not add relay control or change eBUS UART pins.

After deployment: visit `http://<ESP-IP>/status`; a section `Outdoor_Temperature` appears. `State=ok` and `Temperature_C` indicate a current reading. Invalid/missing/stale readings show JSON null and a state, never an old temperature as current. The page refreshes every 10 seconds while visible.

Existing remote log page: `http://<ESP-IP>/logs`. The existing `Outdoor DS18B20: ... C` log lines can also be read there, if the HTTP log facility is running and accessible.

Leave J2 disconnected from the boiler and relay power disconnected. Before any OTA deployment, make a safe backup and verify the ESP's current OTA/partition layout.
