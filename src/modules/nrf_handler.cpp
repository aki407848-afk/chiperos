#include "modules.h"
#include "pins.h"
#include "hal.h"

RF24 radio(Pins::NRF_CSN.primary, Pins::NRF_CE.primary);
bool jammerActive = false;
uint8_t jamChannel = 1;

void nrf_init() {
    pinMode(Pins::NRF_CE.primary, OUTPUT);
    pinMode(Pins::NRF_CSN.primary, OUTPUT);
    radio.begin();
    radio.setAutoAck(false);
    radio.setRetries(0, 0);
    radio.setPayloadSize(32);
    radio.setDataRate(RF24_2MBPS);
    radio.setPALevel(RF24_PA_MAX);
}

void nrf_full_jammer_init() {
    nrf_init();
    jammerActive = true;
    Serial.println("[NRF] 2.4GHz Stress-Test Mode Initialized");
}

void nrf_full_jammer_loop() {
    if (!jammerActive) return;
    
    radio.setChannel(jamChannel);
    radio.openWritingPipe(0xFFFFFFFFFF);
    
    uint8_t payload[32];
    memset(payload, 0xAA, 32);
    
    // ✅ ИСПРАВЛЕНО: startWrite требует 3 аргумента
    radio.startWrite(payload, 32, false);
    
    jamChannel = (jamChannel % 125) + 1;
    yield();
}

void nrf_full_jammer_stop() {
    jammerActive = false;
    radio.stopListening();
    radio.powerDown();
    Serial.println("[NRF] Jammer Stopped");
}

void nrf_radio_listen() {
    nrf_init();
    radio.setChannel(76);
    radio.startListening();
    Serial.println("[NRF] Listening on Ch 76...");
    
    unsigned long start = millis();
    while (millis() - start < 5000) {
        if (radio.available()) {
            uint8_t buf[32];
            radio.read(buf, 32);
            Serial.print("[NRF] RX: ");
            for (int i = 0; i < 32; i++) Serial.printf("%02X ", buf[i]);
            Serial.println();
        }
        yield();
    }
    radio.powerDown();
}
