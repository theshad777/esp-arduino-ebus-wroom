#pragma once

#include <cstdint>

// Bench-test only. No relay control and no boiler interaction.
struct OutdoorTemperatureSnapshot {
  bool valid;               // false until a current valid reading is available
  float temperatureC;
  uint32_t ageSeconds;      // age of the last successful reading (0 before first reading)
  const char* state;        // starting / ok / sensor_not_found / read_error / init_error / stale
};

void startOutdoorTemperatureReader();
OutdoorTemperatureSnapshot getOutdoorTemperatureSnapshot();
