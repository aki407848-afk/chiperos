#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "pins.h"
#include "hal.h"
#include "ui.h"
#include "modules.h"  // <-- Подключаем модули

// ======== WEB SERVER (Async - не требует handleClient) ========
AsyncWebServer server(80);

// ======== MODULE RUNTIME STATE ========
bool moduleActive = false;
String runningMod = "";
unsigned long moduleUptime = 0;
unsigned long moduleLastActivity = 0;

// ======== ИНИЦИАЛИЗАЦИЯ WiFi AP ========
void setupWiFiAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_SSID, AP_PASS);
    Serial.printf("[WiFi] AP Started | SSID: %s | IP: %s\n", AP_SSID, WiFi.softAPIP().toString().c_str());
}

// ======== ИНИЦИАЛИЗАЦИЯ ВЕБ-СЕРВЕРА ========
void setupWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>ChiperOS Web</title>";
        html += "<style>body{background:#111;color:#0f0;font-family:monospace;text-align:center;padding:50px}";
        html += "h1{color:#0ff;border-bottom:1px solid #333;padding-bottom:10px}</style></head><body>";
        html += "<h1>Website Active</h1>";
        html += "<p>wifi name: <b>" + String(AP_SSID) + "</b></p>";
        html += "<p>passcode: <b>" + String(AP_PASS) + "</b></p>";
        html += "<hr><p style='color:#666;font-size:0.8em'>ChiperOS " + String(OS_VERSION) + " | " + String(LICENSE_NOTICE) + "</p>";
        html += "</body></html>";
        request->send(200, "text/html", html);
    });

    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "{";
        json += "\"os\":\"" + String(OS_NAME) + "\",\"ver\":\"" + String(OS_VERSION) + "\",";
        json += "\"modules\":{\"ir\":" + String(mods.ir) + ",\"nrf\":" + String(mods.nrf) + ",\"cc\":" + String(mods.cc) + "},";
        json += "\"active_module\":\"" + runningMod + "\"";
        json += "}";
        request->send(200, "application/json", json);
    });
    server.begin();
    Serial.println("[Web] Server started on port 80");
}

// ======== ЗАПУСК МОДУЛЯ ========
void startModule(String modName) {
    if (modName == "CC1101" && !mods.cc) return;
    if (modName == "NRF24" && !mods.nrf) return;

    moduleActive = true;
    runningMod = modName;
    moduleUptime = millis();
    moduleLastActivity = millis();
    
    Serial.printf("[RUN] Module %s started.\n", modName.c_str());
    
    // Инициализация конкретного модуля
    if (modName == "NRF24") nrf_full_jammer_init();
    if (modName == "CC1101") cc_sniff_init();
}

void stopModule() {
    if (!moduleActive) return;
    Serial.printf("[RUN] Module %s stopped.\n", runningMod.c_str());
    
    // Остановка модуля
    if (runningMod == "NRF24") nrf_full_jammer_stop();
    if (runningMod == "CC1101") cc_sniff_stop();
    
    moduleActive = false;
    runningMod = "";
}

// ======== ОБРАБОТКА ЗАПУЩЕННОГО МОДУЛЯ (В LOOP) ========
void runActiveModule() {
    if (!moduleActive) return;

    // Вызываем ПРАВИЛЬНЫЕ функции из modules.h
    if (runningMod == "NRF24") {
        nrf_full_jammer_loop();  // ✅ Правильное имя
    }
    else if (runningMod == "CC1101") {
        cc_sniff_subghz();       // ✅ Правильное имя (было cc_sniff_loop - ошибка)
    }

    // Обновляем таймер активности
    moduleLastActivity = millis();

    // Авто-стоп через 10 минут (безопасность)
    if (millis() - moduleUptime > 600000) {        Serial.println("[RUN] Auto-stop: Timeout reached.");
        stopModule();
        if (currentState == UI_MODULE_RUN) {
            currentState = UI_MAIN_MENU;
            nav.index = 0;
        }
    }
}

// ======== SETUP ========
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n[BOOT] " + String(OS_NAME) + " " + String(OS_VERSION));
    Serial.println("[BOOT] " + String(LICENSE_NOTICE));

    // 1. Безопасность
    initSafeMode();
    
    // 2. Железо
    detectModules();
    fs_init();

    // 3. Интерфейс
    ui_init();
    
    // 4. Сеть
    setupWiFiAP();
    setupWebServer();  // AsyncWebServer - не нужен handleClient()

    // 5. Готовность
    delay(1000);
    currentState = UI_MAIN_MENU;
    nav.index = 0;
    Serial.println("[BOOT] System ready. Awaiting input.");
}

// ======== LOOP ========
void loop() {
    // 🔒 ЗАЩИТА ОТ ЗАВИСАНИЙ
    feedWatchdog();
    checkLoopHang();

    // 🌐 Сеть (AsyncWebServer обрабатывает запросы сам, handleClient() НЕ НУЖЕН!)
    // server.handleClient();  <-- УДАЛИТЬ ЭТУ СТРОКУ!

    // 🎨 Интерфейс
    ui_loop();

    // ⚙️ Активный модуль    runActiveModule();

    // Если пользователь вернулся в меню из модуля → стоп модуль
    if (currentState == UI_MAIN_MENU && moduleActive) {
        stopModule();
    }
}
