#include <Arduino.h>
#include "ui.h"
#include "filemanager.h"
#include "hardware.h"
#include <esp_system.h>

unsigned long btnTime[3] = {0};
const int BTN_PINS[] = {Pins::BTN_UP.primary, Pins::BTN_DOWN.primary, Pins::BTN_OK.primary};
enum Btn { UP, DOWN, OK };
unsigned long longPressStart = 0;
bool longPressed = false;

int readBtn() {
    for(int i=0; i<3; i++) {
        if(digitalRead(BTN_PINS[i]) == LOW) {
            if(millis() - btnTime[i] > 120) {
                btnTime[i] = millis();
                if(i == OK) {
                    if(longPressStart == 0) longPressStart = millis();
                    else if(millis() - longPressStart > 800) { longPressed = true; return OK; }
                }
                return i;
            }
        } else {
            if(i == OK) longPressStart = 0;
        }
    }
    return -1;
}

void runModuleLogic(const String& mod) {
    String log = "Init " + mod + "...\n";
    if(mod == "IR REMOTE") log += "TX:" + hw.irPins + "\nReady.";
    else if(mod == "NRF24") log += "CE/CSN:" + hw.nrfPins + "\nScanning...";
    else if(mod == "CC1101") log += "CS/GDO0:" + hw.ccPins + "\nFreq: 433MHz";
    else if(mod == "SD CARD") log += "CS:" + hw.sdPins + "\nType: " + String(SD.cardType());
    else if(mod == "WIFI") log += "MAC: " + WiFi.macAddress() + "\nScan...";
    else if(mod == "BLUETOOTH") log += "BT: ESP32-S3\nAdvertising...";
    drawModule(mod, log);
    delay(2000);
}

void setup() {
    Serial.begin(115200);
    for(int p : BTN_PINS) pinMode(p, INPUT_PULLUP);
    uiInit();
    drawBoot("Scanning HW...");
    detectHardware();
    fm.init();
    delay(500);
    uiState = MENU;
    menuIndex = 0;
    drawMenu();
}

void loop() {
    int btn = readBtn();
    if(btn == -1 && !longPressed) return;
    
    if(uiState == MENU) {
        int activeCount = 0;
        bool acts[] = {hw.wifi, hw.ble, hw.ir, hw.nrf24, hw.cc1101, hw.sd, true};
        for(bool a : acts) if(a) activeCount++;
        
        if(btn == UP) { menuIndex = (menuIndex > 0) ? menuIndex - 1 : activeCount - 1; drawMenu(); }
        else if(btn == DOWN) { menuIndex = (menuIndex < activeCount - 1) ? menuIndex + 1 : 0; drawMenu(); }
        else if(btn == OK) {
            String mods[] = {"WIFI","BLUETOOTH","IR REMOTE","NRF24","CC1101","SD CARD","SETTINGS"};
            int idx = 0;
            for(int i=0; i<7; i++) {
                if(!acts[i]) continue;
                if(idx == menuIndex) {
                    if(i == 6) { uiState = SETTINGS; drawSettings(); }
                    else if(i == 5) { uiState = FILEMAN; fm.listDir("/sd/"); drawFileManager(); }
                    else { uiState = MODULE; currentModule = mods[i]; runModuleLogic(mods[i]); uiState = MENU; drawMenu(); }
                    break;
                }
                idx++;
            }
        }
    }
    else if(uiState == FILEMAN) {
        if(btn == UP) { subIndex = max(0, subIndex - 1); drawFileManager(); }
        else if(btn == DOWN) { subIndex = min((int)fm.dirList.size()-1, subIndex + 1); drawFileManager(); }
        else if(btn == OK) { fm.select(); drawFileManager(); }
        else if(longPressed) { fm.goUp(); drawFileManager(); longPressed = false; }
    }
    else if(uiState == SETTINGS) {
        if(btn == OK) { drawMessage("SYSTEM", "Rebooting..."); delay(1000); ESP.restart(); }
        if(longPressed) { uiState = MENU; drawMenu(); longPressed = false; }
    }
    longPressed = false;
}
