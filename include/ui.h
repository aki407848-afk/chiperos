#pragma once
#include <TFT_eSPI.h>
#include "hardware.h"
#include "filemanager.h"
extern TFT_eSPI tft;
extern FileManager fm;

enum UIState { BOOT, MENU, MODULE, FILEMAN, SETTINGS, MSG };
extern UIState uiState;
extern int menuIndex, subIndex;
extern String currentModule;

void uiInit();
void drawBoot(const String& log);
void drawMenu();
void drawModule(const String& title, const String& log);
void drawFileManager();
void drawSettings();
void drawMessage(const String& title, const String& msg);
void clearScreen();
