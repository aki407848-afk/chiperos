#pragma once

struct PinPair { int primary; int backup; };

namespace Pins {
    // ST7789 1.14"
    constexpr PinPair TFT_CS  = {10, 34};
    constexpr PinPair TFT_DC  = {13, 33};
    constexpr PinPair TFT_RST = {14, 32};
    constexpr PinPair TFT_BL  = {15, 2};

    // Buttons (INPUT_PULLUP)
    constexpr PinPair BTN_UP   = {35, 36};
    constexpr PinPair BTN_DOWN = {37, 38};
    constexpr PinPair BTN_OK   = {39, 40};

    // IR Module
    constexpr PinPair IR_TX = {21, 41};
    constexpr PinPair IR_RX = {22, 42};

    // NRF24L01 (SPI Shared: MOSI=11, MISO=12, SCK=13)
    constexpr PinPair NRF_CE  = {16, 17};
    constexpr PinPair NRF_CSN = {5, 18};

    // CC1101 (SPI Shared)
    constexpr PinPair CC1101_CS   = {4, 6};
    constexpr PinPair CC1101_GDO0 = {7, 8};

    // SD Card (SPI Shared)
    constexpr PinPair SD_CS = {9, 46};
}
