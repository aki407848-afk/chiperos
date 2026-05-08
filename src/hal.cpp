#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <LittleFS.h>
#include "hal.h"

// ======== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ========
SafeState safeState;
ModuleStatus mods;
int activeIR_TX = -1, activeIR_RX = -1;
int activeNRF_CE = -1, activeNRF_CSN = -1;
int activeCC_CS = -1, activeCC_GDO0 = -1;
int activeSD_CS = -1;

// ======== ПРОВЕРКА ПИНОВ ========
bool tryPin(int pin, bool outputMode) {
    if (pin < 0 || Pins::isBlocked(pin)) return false;
    pinMode(pin, outputMode ? OUTPUT : INPUT_PULLUP);
    delay(2);
    bool ok = outputMode ? (digitalRead(pin) == LOW) : (digitalRead(pin) == HIGH);
    pinMode(pin, INPUT);
    return ok;
}

// ======== SPI ПИНГ ========
bool pingSPI(int csPin) {
    if (csPin < 0 || Pins::isBlocked(csPin)) return false;
    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);
    delay(1);
    SPI.begin();
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);
    byte status = SPI.transfer(0x00);
    digitalWrite(csPin, HIGH);
    SPI.endTransaction();
    pinMode(csPin, INPUT);
    return (status != 0xFF && status != 0x00);
}

// ======== ДЕТЕКЦИЯ МОДУЛЕЙ (ГЛАВНАЯ ФУНКЦИЯ) ========
void detectModules() {  // <-- ОТКРЫВАЕМ ФУНКЦИЮ
    Serial.println("[HAL] Scanning hardware...");
    
    // ===== IR Module =====
    if (tryPin(Pins::IR_TX.primary) && tryPin(Pins::IR_RX.primary, false)) {
        mods.ir = true;
        activeIR_TX = Pins::IR_TX.primary;
        activeIR_RX = Pins::IR_RX.primary;        Serial.println("[HAL] IR: PRIMARY");
    } else if (tryPin(Pins::IR_TX.backup) && tryPin(Pins::IR_RX.backup, false)) {
        mods.ir = true;
        activeIR_TX = Pins::IR_TX.backup;
        activeIR_RX = Pins::IR_RX.backup;
        Serial.println("[HAL] IR: BACKUP");
    } else {
        Serial.println("[HAL] IR: NOT FOUND");
    }

    // ===== NRF24L01 =====
    if (pingSPI(Pins::NRF_CSN.primary)) {
        mods.nrf = true;
        activeNRF_CE = Pins::NRF_CE.primary;
        activeNRF_CSN = Pins::NRF_CSN.primary;
        Serial.println("[HAL] NRF24: PRIMARY");
    } else if (pingSPI(Pins::NRF_CSN.backup)) {
        mods.nrf = true;
        activeNRF_CE = Pins::NRF_CE.backup;
        activeNRF_CSN = Pins::NRF_CSN.backup;
        Serial.println("[HAL] NRF24: BACKUP");
    } else {
        Serial.println("[HAL] NRF24: NOT FOUND");
    }

    // ===== CC1101 =====
    if (pingSPI(Pins::CC_CS.primary)) {
        mods.cc = true;
        activeCC_CS = Pins::CC_CS.primary;
        activeCC_GDO0 = Pins::CC_GDO0.primary;
        Serial.println("[HAL] CC1101: PRIMARY");
    } else if (pingSPI(Pins::CC_CS.backup)) {
        mods.cc = true;
        activeCC_CS = Pins::CC_CS.backup;
        activeCC_GDO0 = Pins::CC_GDO0.backup;
        Serial.println("[HAL] CC1101: BACKUP");
    } else {
        Serial.println("[HAL] CC1101: NOT FOUND");
    }

    // ===== SD Card =====
    pinMode(Pins::SD_CS.primary, OUTPUT);
    digitalWrite(Pins::SD_CS.primary, HIGH);
    
    if (SD.begin(Pins::SD_CS.primary, SPI, 4000000)) {
        mods.sd = true;
        activeSD_CS = Pins::SD_CS.primary;
        Serial.println("[HAL] SD: PRIMARY");
    } else {
        pinMode(Pins::SD_CS.backup, OUTPUT);        digitalWrite(Pins::SD_CS.backup, HIGH);
        if (SD.begin(Pins::SD_CS.backup, SPI, 4000000)) {
            mods.sd = true;
            activeSD_CS = Pins::SD_CS.backup;
            Serial.println("[HAL] SD: BACKUP");
        } else {
            Serial.println("[HAL] SD: NOT FOUND");
        }
    }
    
    Serial.println("[HAL] Scan complete.");
}  // <-- ⚠️ ЗАКРЫВАЕМ ФУНКЦИЮ ЗДЕСЬ! Это критично!

// ======== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ========
String getActivePinsString() {
    String s = "IR:"; s += mods.ir ? "OK " : "X ";
    s += "NRF:"; s += mods.nrf ? "OK " : "X ";
    s += "CC:"; s += mods.cc ? "OK " : "X ";
    s += "SD:"; s += mods.sd ? "OK" : "X";
    return s;
}

bool fs_init() {
    if (SD.begin()) return true;
    return LittleFS.begin();
}

String fs_list_dir(const String& path) {
    String out = "";
    File root = SD.open(path);
    if (!root) root = LittleFS.open(path);
    if (!root) return "ERR: PATH";
    
    File file = root.openNextFile();
    while (file) {
        String name = file.name();
        if (path != "/") name.remove(0, path.length() + (path.endsWith("/") ? 0 : 1));
        if (name.length() > 0 && !name.startsWith(".")) {
            out += (file.isDirectory() ? "[D] " : "    ") + name + "\n";
        }
        file = root.openNextFile();
    }
    root.close();
    return out.length() ? out : "EMPTY";
}

bool fs_file_exists(const String& path) { return SD.exists(path) || LittleFS.exists(path); }
bool fs_delete_file(const String& path) { return SD.remove(path) || LittleFS.remove(path); }
bool fs_rename_file(const String& oldPath, const String& newPath) { 
    return SD.rename(oldPath, newPath) || LittleFS.rename(oldPath, newPath); }

void initSafeMode() {
    safeState.lastBoot = millis();
    if (safeState.crashCount >= SAFE_MODE_THRESHOLD) {
        safeState.active = true;
        Serial.println("[SAFE] Too many crashes. Entering Safe Mode.");
    }
}

void feedWatchdog() {
    safeState.loopStart = millis();
    unsigned long t = millis() - safeState.lastBoot;
    if (t > 5000 && !safeState.active) {
        safeState.crashCount = 0;
        safeState.lastBoot = millis();
    }
}

void checkLoopHang() {
    if (millis() - safeState.loopStart > WATCHDOG_TIMEOUT_MS) {
        Serial.println("[WATCHDOG] Loop hang detected. Rebooting...");
        safeState.crashCount++;
        ESP.restart();
    }
}
