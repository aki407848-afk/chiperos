#pragma once
#include <Arduino.h>
#include "pins.h"

struct HWStatus {
    bool wifi = true, ble = true, ir = false, nrf24 = false, cc1101 = false, sd = false;
    String irPins = "", nrfPins = "", ccPins = "", sdPins = "";
};

extern HWStatus hw;
void detectHardware();
bool tryPin(int pin, bool output = true);
bool pingSPI(int csPin);
