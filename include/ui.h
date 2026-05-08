#pragma once
#include <TFT_eSPI.h>
#include "config.h"
#include "hal.h"

extern TFT_eSPI tft;

// ======== СОСТОЯНИЯ ИНТЕРФЕЙСА ========
enum UIState {
    UI_BOOT,
    UI_MAIN_MENU,
    UI_MODULE_SELECT,
    UI_MODULE_RUN,
    UI_SDK,
    UI_SETTINGS,
    UI_WEB,
    UI_SAFE_MODE,
    UI_DISCONNECT_MSG
};
extern UIState currentState;
extern UIState prevState;

// ======== НАВИГАЦИЯ ========
struct MenuNav {
    int index = 0;
    int depth = 0;
    String selectedModule = "";
};
extern MenuNav nav;

// ======== ФУНКЦИИ UI ========
void ui_init();
void ui_loop();
void ui_render();
void ui_handle_input();

void ui_show_boot();
void ui_show_main_menu();
void ui_show_module_select();
void ui_show_module_run();
void ui_show_sdk();
void ui_show_settings();
void ui_show_web();
void ui_show_disconnect_msg();
void ui_show_safe_mode();

void ui_clear();
void drawText(const String& txt, int x, int y, uint16_t color, int size);

// ======== ВАЖНО: ОБЪЯВЛЕНИЕ ФУНКЦИИ ЗАПУСКА ========
void checkAndRunModule(String modName);
