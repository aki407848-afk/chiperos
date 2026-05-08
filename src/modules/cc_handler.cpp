#include "modules.h"
#include "pins.h"

bool ccActive = false;

void cc_init() {
    pinMode(Pins::CC_CS.primary, OUTPUT);
    pinMode(Pins::CC_GDO0.primary, INPUT);
    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setCCMode(1);
    ELECHOUSE_cc1101.setModulation(2); // 2FSK
    ELECHOUSE_cc1101.setPA(10);
}

void cc_sniff_init() {
    cc_init();
    ELECHOUSE_cc1101.setFreq(433.92);
    ELECHOUSE_cc1101.SetRx();  // ✅ SetRx() с большой буквы
    ccActive = true;
    Serial.println("[CC] Sub-GHz Sniffer Active @433.92MHz");
}

void cc_sniff_subghz() {  // ✅ Правильное имя функции
    if (!ccActive) return;
    
    // ✅ ИСПРАВЛЕНО: CheckReceiveFlag() → GetReceiveFlag() или прямой опрос
    if (ELECHOUSE_cc1101.CheckReceiveFlag()) {
        uint8_t len = ELECHOUSE_cc1101.ReceiveData();
        if (len > 0 && len <= 64) {
            uint8_t buf[64];
            // ✅ ИСПРАВЛЕНО: gotPacket() → ReadData() или прямой доступ
            for (int i = 0; i < len; i++) {
                buf[i] = ELECHOUSE_cc1101.ReadData();
            }
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
    ELECHOUSE_cc1101.setFreq(433.92);
    ELECHOUSE_cc1101.SetTx();  // ✅ SetTx() с большой буквы
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x01};
    ELECHOUSE_cc1101.SendData(data, 5);
    Serial.println("[CC] FSK Test Packet Sent");
}

void cc_freq_sweep() {
    Serial.println("[CC] Sweeping 300.0 - 900.0 MHz...");
    for (float f = 300.0; f <= 900.0; f += 2.5) {
        ELECHOUSE_cc1101.setFreq(f);
        ELECHOUSE_cc1101.SetRx();
        delay(30);
        if (ELECHOUSE_cc1101.CheckReceiveFlag()) {
            Serial.printf("[CC] ! Signal detected at %.2f MHz\n", f);
            uint8_t len = ELECHOUSE_cc1101.ReceiveData();
            Serial.printf("[CC] Pkt Len: %d\n", len);
        }
    }
    ELECHOUSE_cc1101.setFreq(433.92);
    Serial.println("[CC] Sweep Complete");
}
