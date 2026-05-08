# 🛡️ ChiperOS v1 BETA
**Educational Multi-Tool Firmware for ESP32-S3**

---
## 🇷🇺 О Проекте (RU)
**ChiperOS** — экспериментальная ОС для ESP32-S3. Предназначена для изучения протоколов связи, анализа спектра и отладки устройств.

### ⚡ Возможности:
- 🔹 Интерфейс на ST7789 1.14" (кибер-стиль)
- 🔹 Точка доступа + веб-сервер
- 🔹 NRF24L01: стресс-тест 2.4GHz, прослушивание
- 🔹 CC1101: сниффинг 433/868MHz, свип частот
- 🔹 IR: запись/воспроизведение кодов на SD
- 🔹 SDK: управление файлами
- 🔹 Safe Mode + Watchdog

---
## 🇺🇸 About (EN)
**ChiperOS** is experimental firmware for ESP32-S3. Designed for educational purposes: studying protocols, RF analysis, debugging smart devices.

### ⚡ Features:
- 🔹 ST7789 1.14" cyberpunk UI
- 🔹 AP + Web Server
- 🔹 NRF24L01: 2.4GHz stress-test, listening
- 🔹 CC1101: 433/868MHz sniffing, frequency sweep
- 🔹 IR: record/playback codes to SD
- 🔹 SDK: file management
- 🔹 Safe Mode + Watchdog protection

---
## 🔌 Распиновка (Pinout) — ESP32-S3 DevKit

| Компонент | Пин (Primary) | Пин (Backup) | Примечание |
|-----------|--------------|--------------|------------|
| **OLED SDA** | GPIO4 (D2) | - | I2C |
| **OLED SCL** | GPIO5 (D1) | - | I2C |
| **OLED CS** | GPIO10 | GPIO34 | SPI |
| **OLED DC** | GPIO13 | GPIO33 | SPI |
| **OLED RST** | GPIO14 | GPIO32 | SPI |
| **OLED BL** | GPIO15 | GPIO2 | Backlight |
| **BTN UP** | GPIO35 | GPIO36 | INPUT_PULLUP |
| **BTN DOWN** | GPIO37 | GPIO38 | INPUT_PULLUP |
| **BTN OK** | GPIO39 | GPIO40 | INPUT_PULLUP |
| **BTN BACK** | GPIO41 | GPIO42 | INPUT_PULLUP |
| **IR TX** | GPIO21 | GPIO43 | PWM output |
| **IR RX** | GPIO22 | GPIO44 | Interrupt input |
| **NRF CE** | GPIO16 | GPIO17 | SPI device |
| **NRF CSN** | GPIO5 | GPIO18 | SPI CS |
| **CC1101 CS** | GPIO4 | GPIO6 | SPI CS |
| **CC1101 GDO0** | GPIO7 | GPIO8 | Interrupt || **SD CS** | GPIO9 | GPIO46 | SPI CS |
| **SPI MOSI** | GPIO11 | - | Shared |
| **SPI MISO** | GPIO12 | - | Shared |
| **SPI SCK** | GPIO13 | - | Shared |

> ⚠️ **Blocked pins** (не использовать): 0, 19, 20, 45, 47, 48 (USB/strapping)

---
## 🚀 Быстрый старт

### 1. Сборка через PlatformIO
```bash
# Клонируй репо
git clone https://github.com/yourname/ChiperOS.git
cd ChiperOS

# Установи зависимости
pio lib install

# Собери прошивку
pio run -e esp32-s3-devkitc-1

# Залей на плату
pio run -e esp32-s3-devkitc-1 --target upload
```

### 2. Сборка через GitHub Actions
1. Запушь код в репозиторий
2. Перейди во вкладку **Actions**
3. Скачай артефакт `ChiperOS_v1_BETA.bin`
4. Залей через [ESP32 Flash Download Tool](https://www.espressif.com/en/support/download/other-tools)

### 3. Первое включение
1. Подключи питание (5V USB или стабилизированный 3.3В)
2. Экран покажет `ChiperOS v1 BETA`
3. Нажми **OK** для входа в меню
4. Навигация: ▲▼ выбор, ● вход, ●(удерж.) назад

### 4. Подключение к веб-интерфейсу
1. Найди сеть `ChiperOS_v1BETA`
2. Пароль: `1234567890`
3. Открой браузер: `http://192.168.4.1`

---
## ⚙️ Калибровка датчиков

### Почва (для теплицы)
1. Открой Serial Monitor (115200 baud)
2. Воткни датчик в сухую землю → запиши `Raw:` значение
3. Полей → воткни в мокрую → запиши новое `Raw:`4. В `config.h` замени:
```cpp
#define SOIL_DRY  900  // твоё "сухое" число
#define SOIL_WET  350  // твоё "мокрое" число
```

### ИК-коды
1. В меню выбери `RF MODULES` → `IR`
2. Нажми "Record" и нажми кнопку на пульте
3. Код сохранится в `/ir_codes/remote_001.txt`

---
## ⚠️ Disclaimer / Предупреждение
> Данный проект создан исключительно для образовательных целей. Использование для глушения каналов связи, взлома чужих устройств или нарушения приватности является незаконным. Авторы не несут ответственности за misuse кода.
>
> *If you refuse to use this project out of political bias — that's your personal limitation. Code has no borders.*

---
## 📜 License
MIT License — Copyright (c) May 2026, Moscow  
See [LICENSE](LICENSE) for details.
