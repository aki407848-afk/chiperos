#pragma once
#include "config.h"
#include "pins.h"

// ======== HARDWARE DETECTION ========
bool tryPin(int pin, bool outputMode = true);
bool pingSPI(int csPin);
void detectModules();
void applyBackupPins();
String getActivePinsString();

// ======== MODULE INTERFACES (STUBS) ========
void wifi_init();
void wifi_deauther_start();
void wifi_handshake_capture();
void ble_spam_start();
void ble_intercept_start();
void ir_record(const char* filename);
void ir_play(const char* filename);
void ir_match_mode();
void nrf_full_jammer();
void nrf_radio_listen();
void cc_sniff_subghz();
void cc_fsk_tx_test();

// ======== FILESYSTEM ========
bool fs_init();
String fs_list_dir(const String& path);
bool fs_file_exists(const String& path);
bool fs_delete_file(const String& path);
bool fs_rename_file(const String& oldPath, const String& newPath);
