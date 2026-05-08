#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_random.h>
#include "modules.h"

// ======== НАСТРОЙКИ ========
#define USE_RANDOM_MAC        true
#define STATIC_MAC            "e4:5f:01:9a:c2:7b"
#define MAC_CHANGE_INTERVAL   5000

// ======== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ========
uint8_t current_mac[6];
unsigned long lastMacChange = 0;
bool deautherActive = false;

// ======== ФУНКЦИИ MAC ========
void generate_random_mac() {
    uint32_t random1 = esp_random();
    uint32_t random2 = esp_random();
    
    current_mac[0] = 0x02;
    current_mac[1] = random1 & 0xFF;
    current_mac[2] = (random1 >> 8) & 0xFF;
    current_mac[3] = (random1 >> 16) & 0xFF;
    current_mac[4] = (random1 >> 24) & 0xFF;
    current_mac[5] = random2 & 0xFF;
    
    current_mac[0] |= 0x02;
    
    Serial.print("[MAC] Random: ");
    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
                  current_mac[0], current_mac[1], current_mac[2],
                  current_mac[3], current_mac[4], current_mac[5]);
}

void set_static_mac(const char* mac_str) {
    sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &current_mac[0], &current_mac[1], &current_mac[2],
           &current_mac[3], &current_mac[4], &current_mac[5]);
    
    Serial.print("[MAC] Static: ");
    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
                  current_mac[0], current_mac[1], current_mac[2],
                  current_mac[3], current_mac[4], current_mac[5]);
}

void init_mac_randomizer() {
    if (USE_RANDOM_MAC) {
        generate_random_mac();
    } else {        set_static_mac(STATIC_MAC);
    }
    lastMacChange = millis();
}

void update_mac_if_needed() {
    if (USE_RANDOM_MAC && millis() - lastMacChange > MAC_CHANGE_INTERVAL) {
        Serial.println("[MAC] Rotating...");
        generate_random_mac();
        lastMacChange = millis();
        esp_wifi_set_mac(WIFI_IF_AP, current_mac);
    }
}

// ======== DEAUTHER ФУНКЦИИ ========
void wifi_deauther_init() {
    Serial.println("[DEAUTHER] Initializing...");
    
    init_mac_randomizer();
    esp_wifi_set_mac(WIFI_IF_AP, current_mac);
    
    wifi_promiscuous_filter_t filter = {
        .filter_mask = WIFI_PROMIS_FILTER_MASK_DATA | 
                       WIFI_PROMIS_FILTER_MASK_MGMT |
                       WIFI_PROMIS_FILTER_MASK_CTRL
    };
    esp_wifi_set_promiscuous_filter(&filter);
    
    deautherActive = true;
    Serial.println("[DEAUTHER] Started with random MAC rotation");
    Serial.println("[LEGAL] Educational use only!");
}

void wifi_deauther_loop() {
    if (!deautherActive) return;
    
    update_mac_if_needed();
    
    // Здесь твоя логика деаутентификации
    // Пример: отправка deauth пакетов
    // send_deauth_packets();
    
    yield();
}

void wifi_deauther_stop() {
    deautherActive = false;
    Serial.println("[DEAUTHER] Stopped");
}
void wifi_handshake_capture() {
    Serial.println("[DEAUTHER] Handshake capture mode");
    // Логика захвата handshake
}
