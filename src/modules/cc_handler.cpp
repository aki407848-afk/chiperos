#include "modules.h"
#include "pins.h"

bool ccActive = false;

void cc_init() {
    pinMode(Pins::CC_CS.primary, OUTPUT);
    pinMode(Pins::CC_GDO0.primary, INPUT);
    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setCCMode(1);
    ELECHOUSE_cc1101.setModulation(2);
    ELECHOUSE_cc1101.setPA(10);
    ELECHOUSE_cc1101.setMHZ(433.92);
}

void cc_sniff_init() {
    cc_init();
    ELECHOUSE_cc1101.SetRx();
    ccActive = true;
    Serial.println("[CC] Sub-GHz Sniffer Active @433.92MHz");
}

void cc_sniff_subghz() {
    if (!ccActive) return;
    
    if (ELECHOUSE_cc1101.CheckReceiveFlag()) {
        uint8_t buf[64];
        // ✅ ИСПРАВЛЕНО: ReceiveData требует буфер
        uint8_t len = ELECHOUSE_cc1101.ReceiveData(buf);
        
        if (len > 0 && len <= 64) {
            Serial.print("[CC] RX Len:"); Serial.print(len);
            Serial.print(" Data: ");
            for (int i = 0; i < len; i++) Serial.printf("%02X ", buf[i]);
            Serial.println();
        }
    }
    yield();
}

void cc_sniff_stop() {
    ccActive = false;
    ELECHOUSE_cc1101.SetTx();
    Serial.println("[CC] Sniffer Stopped");
}

void cc_fsk_tx_test() {
    cc_init();
    ELECHOUSE_cc1101.SetTx();
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x01};
    ELECHOUSE_cc1101.SendData(data, 5);
    Serial.println("[CC] FSK Test Packet Sent");
}

void cc_freq_sweep() {
    Serial.println("[CC] Sweeping...");
    float freqs[] = {315.0, 433.92, 868.0, 915.0};
    
    for (float f : freqs) {
        ELECHOUSE_cc1101.setMHZ(f);
        ELECHOUSE_cc1101.SetRx();
        delay(50);
        if (ELECHOUSE_cc1101.CheckReceiveFlag()) {
            uint8_t buf[64];
            uint8_t len = ELECHOUSE_cc1101.ReceiveData(buf);  // ✅ Исправлено
            Serial.printf("[CC] ! Signal at %.2f MHz, Len: %d\n", f, len);
        }
    }
    ELECHOUSE_cc1101.setMHZ(433.92);
    Serial.println("[CC] Sweep Complete");
}
