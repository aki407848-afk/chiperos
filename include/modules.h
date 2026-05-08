#pragma once
#include <Arduino.h>
#include <SD.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <RF24.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>

// ======== NRF24L01 ========
void nrf_init();
void nrf_full_jammer_init();
void nrf_full_jammer_loop();
void nrf_full_jammer_stop();
void nrf_radio_listen();

// ======== CC1101 ========
void cc_init();
void cc_sniff_init();
void cc_sniff_loop();
void cc_sniff_stop();
void cc_fsk_tx_test();
void cc_freq_sweep();

// ======== IR ========
void ir_init();
void ir_record_to_sd(const char* filename);
void ir_play_from_sd(const char* filename);
void ir_match_mode();

// ======== SDK / FS ========
void sdk_list_files();
bool sdk_delete_file(const String& path);
bool sdk_rename_file(const String& oldP, const String& newP);
String sdk_read_file(const String& path);
