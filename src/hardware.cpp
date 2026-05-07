#include "hardware.h"
#include <SPI.h>

HWStatus hw;

bool tryPin(int pin, bool output) {
    if (pin < 0) return false;
    pinMode(pin, output ? OUTPUT : INPUT_PULLUP);
    if (output) {
        digitalWrite(pin, LOW); delay(2);
        bool r = (digitalRead(pin) == LOW);
        pinMode(pin, INPUT);
        return r;
    } else {
        bool r = (digitalRead(pin) == HIGH);
        pinMode(pin, OUTPUT); digitalWrite(pin, LOW); delay(2);
        pinMode(pin, INPUT);
        return r;
    }
}

bool pingSPI(int csPin) {
    if (csPin < 0) return false;
    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);
    SPI.begin();
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);
    byte status = SPI.transfer(0x00);
    digitalWrite(csPin, HIGH);
    SPI.endTransaction();
    pinMode(csPin, INPUT);
    return (status != 0xFF && status != 0x00);
}

void detectHardware() {
    Serial.println("[HW] Scanning...");
    
    if (tryPin(Pins::IR_TX.primary) && tryPin(Pins::IR_RX.primary, false)) {
        hw.ir = true; hw.irPins = "P:" + String(Pins::IR_TX.primary) + "/" + String(Pins::IR_RX.primary);
    } else if (tryPin(Pins::IR_TX.backup) && tryPin(Pins::IR_RX.backup, false)) {
        hw.ir = true; hw.irPins = "B:" + String(Pins::IR_TX.backup) + "/" + String(Pins::IR_RX.backup);
    }

    if (pingSPI(Pins::NRF_CSN.primary)) {
        hw.nrf24 = true; hw.nrfPins = "P:" + String(Pins::NRF_CE.primary) + "/" + String(Pins::NRF_CSN.primary);
    } else if (pingSPI(Pins::NRF_CSN.backup)) {
        hw.nrf24 = true; hw.nrfPins = "B:" + String(Pins::NRF_CE.backup) + "/" + String(Pins::NRF_CSN.backup);
    }

    if (pingSPI(Pins::CC1101_CS.primary)) {
        hw.cc1101 = true; hw.ccPins = "P:" + String(Pins::CC1101_CS.primary) + "/" + String(Pins::CC1101_GDO0.primary);
    } else if (pingSPI(Pins::CC1101_CS.backup)) {
        hw.cc1101 = true; hw.ccPins = "B:" + String(Pins::CC1101_CS.backup) + "/" + String(Pins::CC1101_GDO0.backup);
    }

    pinMode(Pins::SD_CS.primary, OUTPUT); digitalWrite(Pins::SD_CS.primary, HIGH);
    if (SD.begin(Pins::SD_CS.primary)) {
        hw.sd = true; hw.sdPins = "P:" + String(Pins::SD_CS.primary);
    } else {
        pinMode(Pins::SD_CS.backup, OUTPUT); digitalWrite(Pins::SD_CS.backup, HIGH);
        if (SD.begin(Pins::SD_CS.backup)) {
            hw.sd = true; hw.sdPins = "B:" + String(Pins::SD_CS.backup);
        }
    }
    Serial.println("[HW] Done.");
}
