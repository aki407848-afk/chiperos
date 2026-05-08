#pragma once
#include <Arduino.h>

struct PinPair { int primary; int backup; };

namespace Pins {
    // ST7789 1.14"
    constexpr PinPair PIN_TFT_CS  = {10, 34};
    constexpr PinPair PIN_TFT_DC  = {13, 33};
    constexpr PinPair PIN_TFT_RST = {14, 32};
    constexpr PinPair PIN_TFT_BL  = {15, 2};

    // Buttons (INPUT_PULLUP, active LOW)
    constexpr PinPair BTN_UP   = {35, 36};
    constexpr PinPair BTN_DOWN = {37, 38};
    constexpr PinPair BTN_OK   = {39, 40};
    constexpr PinPair BTN_BACK = {41, 42};

    // IR Module
    constexpr PinPair IR_TX = {21, 43};
    constexpr PinPair IR_RX = {22, 44};

    // NRF24L01 (SPI Shared: MOSI=11, MISO=12, SCK=13)
    constexpr PinPair NRF_CE  = {16, 17};
    constexpr PinPair NRF_CSN = {5, 18};

    // CC1101 (SPI Shared)
    constexpr PinPair CC_CS   = {4, 6};
    constexpr PinPair CC_GDO0 = {7, 8};

    // SD Card (SPI Shared)
    constexpr PinPair SD_CS = {9, 46};

    // Blocked Pins (USB/Strapping - NEVER USE)
    constexpr int BLOCKED[] = {0, 19, 20, 45, 47, 48};
    
    inline bool isBlocked(int p) {
        for (int b : BLOCKED) if (p == b) return true;
        return false;
    }
}
