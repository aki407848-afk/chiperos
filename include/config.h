#pragma once

#define OS_NAME       "ChiperOS"
#define OS_VERSION    "v1 BETA"
#define BUILD_DATE    "May 2026"
#define BUILD_CITY    "Moscow"
#define AP_SSID       "ChiperOS_v1BETA"
#define AP_PASS       "1234567890"

// ======== SYSTEM LIMITS & SAFEGUARDS ========
#define MAX_MENU_DEPTH      4
#define MAX_FILE_PATH_LEN   64
#define MAX_IR_CODES        50
#define WATCHDOG_TIMEOUT_MS 8000
#define SAFE_MODE_THRESHOLD 5   // Перезагрузок подряд → вход в SafeMode

// ======== SAFE MODE & WATCHDOG ========
struct SafeState {
    bool active = false;
    uint8_t crashCount = 0;
    unsigned long lastBoot = 0;
    unsigned long loopStart = 0;
    unsigned long maxLoopTime = 0;
};
extern SafeState safeState;

// ======== MODULE FLAGS ========
struct ModuleStatus {
    bool wifi = true;
    bool ble  = true;
    bool ir   = false;
    bool nrf  = false;
    bool cc   = false;
    bool sd   = false;
    bool web  = false;
};
extern ModuleStatus mods;

// ======== LEGAL / EDUCATIONAL FLAG ========
constexpr const char* LICENSE_NOTICE = 
    "ChiperOS v1 BETA | Created May 2026, Moscow\n"
    "Educational & Research Purposes Only.\n"
    "Open to all. Use responsibly & legally.";

void initSafeMode();
void feedWatchdog();
void checkLoopHang();
