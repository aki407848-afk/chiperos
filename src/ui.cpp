#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "pins.h"
#include "hal.h"
#include "ui.h"
#include "modules.h"

TFT_eSPI tft = TFT_eSPI();
UIState currentState = UI_BOOT;
UIState prevState = UI_BOOT;
MenuNav nav;
unsigned long msgTimer = 0;
unsigned long lastKeyTime = 0;

// ======== КНОПКИ ========
int readKey() {
    if (millis() - lastKeyTime < 150) return -1;
    if (digitalRead(Pins::BTN_UP.primary) == LOW) { lastKeyTime = millis(); return 0; }
    if (digitalRead(Pins::BTN_DOWN.primary) == LOW) { lastKeyTime = millis(); return 1; }
    if (digitalRead(Pins::BTN_OK.primary) == LOW) {
        unsigned long pressStart = millis();
        while (digitalRead(Pins::BTN_OK.primary) == LOW) {
            if (millis() - pressStart > 1000) return 3;
            delay(10);
        }
        lastKeyTime = millis(); return 2;
    }
    if (digitalRead(Pins::BTN_BACK.primary) == LOW) { lastKeyTime = millis(); return 3; }
    return -1;
}

// ======== УТИЛИТЫ ========
void drawBox(int x, int y, int w, int h) { tft.drawRect(x, y, w, h, TFT_CYAN); }

void drawText(const String& txt, int x, int y, uint16_t color = TFT_WHITE, int size = 1) {
    tft.setTextSize(size);
    tft.setTextColor(color, TFT_BLACK);
    tft.setCursor(x, y);
    tft.print(txt);
}

// ======== ИНИЦИАЛИЗАЦИЯ ========
void ui_init() {
    pinMode(Pins::BTN_UP.primary, INPUT_PULLUP);
    pinMode(Pins::BTN_DOWN.primary, INPUT_PULLUP);
    pinMode(Pins::BTN_OK.primary, INPUT_PULLUP);
    pinMode(Pins::BTN_BACK.primary, INPUT_PULLUP);
    pinMode(Pins::PIN_TFT_BL.primary, OUTPUT);
    digitalWrite(Pins::PIN_TFT_BL.primary, HIGH);    tft.init(); tft.setRotation(1); tft.fillScreen(TFT_BLACK); tft.setTextFont(2);
}

// ======== ЦИКЛ ========
void ui_loop() {
    int key = readKey();
    if (currentState == UI_DISCONNECT_MSG && millis() > msgTimer) currentState = prevState;
    
    if (key == 0) nav.index--;
    if (key == 1) nav.index++;
    if (key == 2) handleOKPress();
    if (key == 3) handleBackPress();
    
    if (currentState != prevState || key != -1) { ui_render(); prevState = currentState; }
}

// ======== ОБРАБОТКА КНОПОК ========
void handleOKPress();  // Forward declaration
void handleBackPress();

void handleOKPress() {
    switch (currentState) {
        case UI_BOOT: currentState = UI_MAIN_MENU; nav.index = 0; break;
        case UI_MAIN_MENU:
            if (nav.index == 0) { currentState = UI_MODULE_SELECT; nav.index = 0; }
            if (nav.index == 1) { currentState = UI_SDK; nav.index = 0; }
            if (nav.index == 2) { currentState = UI_SETTINGS; nav.index = 0; }
            if (nav.index == 3) { currentState = UI_WEB; nav.index = 0; }
            break;
        case UI_MODULE_SELECT:
            if (nav.index == 0) checkAndRunModule("CC1101");
            if (nav.index == 1) checkAndRunModule("NRF24");
            break;
        case UI_SDK: sdk_list_files(); break;
        case UI_SETTINGS:
            if (nav.index == 3) {
                safeState.crashCount = 0;
                drawText("Reset OK", 20, 100, TFT_GREEN);
                delay(800);
            }
            break;
    }
}

void checkAndRunModule(String modName) {
    bool connected = (modName == "CC1101") ? mods.cc : (modName == "NRF24") ? mods.nrf : false;
    if (connected) {
        nav.selectedModule = modName;
        currentState = UI_MODULE_RUN;
        if (modName == "CC1101") cc_sniff_init();        if (modName == "NRF24") nrf_full_jammer_init();
    } else {
        prevState = currentState;
        currentState = UI_DISCONNECT_MSG;
        msgTimer = millis() + 3000;
    }
}

void handleBackPress() {
    if (currentState == UI_MODULE_RUN) {
        if (nav.selectedModule == "CC1101") cc_sniff_stop();
        if (nav.selectedModule == "NRF24") nrf_full_jammer_stop();
        currentState = UI_MAIN_MENU; nav.index = 0;
    } else if (currentState != UI_MAIN_MENU && currentState != UI_BOOT) {
        currentState = UI_MAIN_MENU; nav.index = 0;
    }
}

// ======== ОТРИСОВКА ========
void ui_render() {
    switch (currentState) {
        case UI_BOOT: draw_boot(); break;
        case UI_MAIN_MENU: draw_main_menu(); break;
        case UI_MODULE_SELECT: draw_module_select(); break;
        case UI_MODULE_RUN: draw_module_run(); break;
        case UI_SDK: draw_sdk(); break;
        case UI_SETTINGS: draw_settings(); break;
        case UI_WEB: draw_web(); break;
        case UI_DISCONNECT_MSG: draw_disconnect(); break;
        case UI_SAFE_MODE: draw_safe_mode(); break;
    }
}

void draw_boot() {
    tft.fillScreen(TFT_BLACK);
    drawBox(10, 10, 115, 40);
    drawText(String(OS_NAME), 25, 18, TFT_GREEN, 2);
    drawText(String(OS_VERSION), 45, 34, TFT_WHITE, 1);
    drawText("INIT...", 45, 220, TFT_YELLOW);
}

void draw_main_menu() {
    tft.fillScreen(TFT_BLACK);
    drawText("MAIN MENU", 10, 5, TFT_RED);
    tft.drawLine(0, 18, 135, 18, TFT_RED);
    String items[] = {"RF MODULES", "SDK MANAGER", "SETTINGS", "WEBSITE INFO"};
    for (int i = 0; i < 4; i++) {
        uint16_t color = (i == nav.index) ? TFT_GREEN : TFT_WHITE;
        if (i == nav.index) tft.fillRect(2, 25 + (i * 20), 131, 18, TFT_NAVY);
        drawText("> " + items[i], 10, 27 + (i * 20), color);    }
}

void draw_module_select() {
    tft.fillScreen(TFT_BLACK);
    drawText("SELECT MODULE", 10, 5, TFT_MAGENTA);
    tft.drawLine(0, 18, 135, 18, TFT_MAGENTA);
    String items[] = {"CC1101 SUB-GHZ", "NRF24L01 2.4GHZ"};
    for (int i = 0; i < 2; i++) {
        uint16_t color = (i == nav.index) ? TFT_YELLOW : TFT_WHITE;
        String status = ((i==0&&mods.cc)||(i==1&&mods.nrf)) ? " [OK]" : " [OFF]";
        if (i == nav.index) tft.fillRect(2, 30 + (i * 25), 131, 20, 0x5A0D);
        drawText(items[i] + status, 10, 32 + (i * 25), color);
    }
}

void draw_module_run() {
    tft.fillScreen(TFT_BLACK);
    drawText("RUNNING: " + nav.selectedModule, 5, 5, TFT_GREEN);
    drawBox(5, 20, 125, 180);
    drawText("STATUS: ACTIVE", 15, 30, TFT_WHITE);
    if (nav.selectedModule == "NRF24") {
        drawText("JAMMER", 60, 50, TFT_RED);
        drawText("SIG: [||||||||||]", 15, 70, TFT_GREEN);
    } else {
        drawText("SUB-GHZ", 60, 50, TFT_CYAN);
        drawText("FREQ: 433.92", 15, 70, TFT_YELLOW);
    }
    drawText("Hold OK to Stop", 20, 220, TFT_DARKGREY);
}

void draw_sdk() {
    tft.fillScreen(TFT_BLACK);
    drawText("SDK / FILE SYS", 10, 5, TFT_ORANGE);
    tft.drawLine(0, 18, 135, 18, TFT_ORANGE);
    String files[] = {"[D] CONFIG", "[F] IR_LOG1", "[F] CAP_01", "[F] KEY.DB"};
    for (int i = 0; i < 4; i++) {
        if (i == nav.index) tft.fillRect(2, 25 + (i * 20), 131, 18, 0x001F);
        drawText(files[i], 10, 27 + (i * 20), (i == nav.index) ? TFT_CYAN : TFT_WHITE);
    }
}

void draw_settings() {
    tft.fillScreen(TFT_BLACK);
    drawText("SETTINGS", 10, 5, TFT_YELLOW);
    tft.drawLine(0, 18, 135, 18, TFT_YELLOW);
    String items[] = {"INFO PINS", "MODULE INFO", "OS VERSION", "RESET SAFE MODE"};
    int count = 4;
    if (nav.index >= count) nav.index = count - 1;
    for (int i = 0; i < count; i++) {        if (i == nav.index) tft.fillRect(2, 25 + (i * 25), 131, 22, 0x7800);
        drawText(items[i], 10, 28 + (i * 25), (i == nav.index) ? TFT_WHITE : TFT_DARKGREY);
    }
    if (nav.index == 0) drawText(getActivePinsString(), 10, 130, TFT_CYAN);
    if (nav.index == 1) {
        String s = "W:" + String(mods.wifi) + " B:" + String(mods.ble) + " IR:" + String(mods.ir) + " CC:" + String(mods.cc);
        drawText(s, 10, 130, TFT_GREEN);
    }
    if (nav.index == 2) {
        drawText(String(OS_NAME) + " " + String(OS_VERSION), 10, 130, TFT_WHITE);  // ✅ FIX: String() wrapper
        drawText(String(BUILD_DATE) + " " + String(BUILD_CITY), 10, 145, TFT_WHITE);  // ✅ FIX
    }
}

void draw_web() {
    tft.fillScreen(TFT_BLACK);
    drawText("WEBSITE INFO", 10, 5, TFT_BLUE);
    tft.drawLine(0, 18, 135, 18, TFT_BLUE);
    drawText("STATUS: ACTIVE", 20, 40, TFT_GREEN);
    drawText("SSID:", 20, 70, TFT_WHITE);
    drawText(AP_SSID, 60, 70, TFT_YELLOW);
    drawText("PASS:", 20, 90, TFT_WHITE);
    drawText(AP_PASS, 60, 90, TFT_YELLOW);
}

void draw_disconnect() {
    tft.fillRect(10, 50, 115, 60, TFT_RED);
    tft.fillRect(12, 52, 111, 56, TFT_BLACK);
    drawText("DISCONNECT", 25, 60, TFT_RED);
    drawText("MODULE!", 35, 75, TFT_RED);
    drawText("Check Wiring", 20, 95, TFT_WHITE);
}

void draw_safe_mode() {
    tft.fillScreen(TFT_RED);
    drawText("SAFE MODE", 20, 50, TFT_BLACK, 2);
    drawText("Too many crashes.", 10, 80, TFT_BLACK);
    drawText("Rebooting...", 30, 100, TFT_BLACK);
}
