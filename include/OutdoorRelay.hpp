#pragma once
// GPIO18 controls BC337 through the user's 1 kOhm base resistor.
// Called at boot: LOW => relay deenergized => boiler's outdoor NTC is disconnected.
void initOutdoorRelay();
bool isOutdoorRelayEnabled();
// Internal API for a later supervised bench test. Not exposed over HTTP/MQTT.
// True energizes relay (NO closes); false deenergizes relay (NO opens).
bool setOutdoorRelay(bool enabled);
