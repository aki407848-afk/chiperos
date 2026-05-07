#include "ui.h"
#include <WiFi.h>
UIState uiState = BOOT;
int menuIndex = 0, subIndex = 0;
String currentModule = "";
TFT_eSPI tft = TFT_eSPI();

void uiInit() {
    tft.init(); tft.setRotation(1); tft.fillScreen(TFT_BLACK);
    tft.setTextFont(2);
    pinMode(Pins::TFT_BL.primary, OUTPUT);
    digitalWrite(Pins::TFT_BL.primary, HIGH);
}

void clearScreen() { tft.fillScreen(TFT_BLACK); }

void drawBoot(const String& log) {
    clearScreen();
    tft.setTextColor(TFT_CYAN, TFT_BLACK); tft.setCursor(10, 10); tft.println("CHIPER OS v1.0");
    tft.setTextColor(TFT_WHITE, TFT_BLACK); tft.setCursor(10, 30); tft.print(log);
}

void drawMenu() {
    clearScreen();
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK); tft.setCursor(10, 5); tft.println("MAIN MENU");
    tft.drawLine(0, 18, 135, 18, TFT_NAVY);
    
    String items[] = {"WIFI", "BLUETOOTH", "IR REMOTE", "NRF24", "CC1101", "SD CARD", "SETTINGS"};
    bool active[] = {hw.wifi, hw.ble, hw.ir, hw.nrf24, hw.cc1101, hw.sd, true};
    int count = 0; for(bool a : active) if(a) count++;
    
    int y = 25;
    for(int i=0, idx=0; i<7; i++) {
        if(!active[i]) continue;
        bool sel = (idx == menuIndex);
        if(sel) { tft.fillRect(2, y-2, 131, 18, TFT_RED); tft.setTextColor(TFT_WHITE, TFT_RED); }
        else { tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK); }
        tft.setCursor(8, y+2); tft.println(items[i]);
        y += 20; idx++;
    }
}

void drawModule(const String& title, const String& log) {
    clearScreen();
    tft.setTextColor(TFT_GREEN, TFT_BLACK); tft.setCursor(10, 10); tft.println(title);
    tft.setTextColor(TFT_WHITE, TFT_BLACK); tft.setCursor(10, 30); tft.print(log);
}

void drawFileManager() {
    clearScreen();
    tft.setTextColor(TFT_YELLOW, TFT_BLACK); tft.setCursor(10, 5); tft.println("FILE MANAGER");
    tft.setTextColor(TFT_CYAN, TFT_BLACK); tft.setCursor(10, 20); tft.println(fm.currentPath);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    
    auto& list = fm.dirList;
    int start = max(0, subIndex - 5);
    int end = min((int)list.size(), start + 6);
    int y = 35;
    for(int i=start; i<end; i++) {
        bool isDir = fm.isDir[i];
        bool sel = (i == subIndex);
        if(sel) { tft.fillRect(2, y-2, 131, 16, TFT_BLUE); tft.setTextColor(TFT_WHITE, TFT_BLUE); }
        else tft.setTextColor(isDir ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
        tft.setCursor(8, y);
        tft.println((isDir ? "[D] " : "    ") + list[i]);
        y += 16;
    }
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK); tft.setCursor(10, 220); tft.println("UP:▲ DN:▼ OK:● BK:LONG");
}

void drawSettings() {
    clearScreen();
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK); tft.setCursor(10, 10); tft.println("SETTINGS");
    tft.setTextColor(TFT_WHITE, TFT_BLACK); tft.setCursor(10, 30);
    tft.println("[1] Reboot System");
    tft.setCursor(10, 50); tft.println("[2] HW Info");
    tft.setCursor(10, 70);
    tft.println("HW: " + (hw.ir?"IR ":"") + (hw.nrf24?"NRF ":"") + (hw.cc1101?"CC ":"") + (hw.sd?"SD":""));
}

void drawMessage(const String& title, const String& msg) {
    clearScreen();
    tft.setTextColor(TFT_RED, TFT_BLACK); tft.setCursor(10, 10); tft.println(title);
    tft.setTextColor(TFT_WHITE, TFT_BLACK); tft.setCursor(10, 30); tft.println(msg);
}
