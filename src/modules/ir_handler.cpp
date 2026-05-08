#include "modules.h"
#include "pins.h"

IRrecv irrecv(Pins::IR_RX.primary);
IRsend irsend(Pins::IR_TX.primary);
decode_results results;

void ir_init() {
    irrecv.enableIRIn();
    Serial.println("[IR] Receiver Initialized");
}

void ir_record_to_sd(const char* filename) {
    Serial.printf("[IR] Recording to /%s... Point remote and press button.\n", filename);
    
    // ✅ ИСПРАВЛЕНО: проверка перед декодированием
    if (irrecv.decode(&results)) {
        File f = SD.open(filename, FILE_WRITE);
        if (f) {
            f.print(results.decode_type); f.print(",");
            f.print(results.value, HEX); f.print(",");
            f.println(results.bits);
            f.close();
            Serial.printf("[IR] Saved: Type=%d Val=%lX Bits=%d\n", 
                          results.decode_type, results.value, results.bits);
        } else {
            Serial.println("[IR] Error: Cannot open SD file");
        }
        irrecv.resume(); // ✅ Продолжить приём
    }
}

void ir_play_from_sd(const char* filename) {
    File f = SD.open(filename, FILE_READ);
    if (!f) { Serial.println("[IR] File not found"); return; }
    
    String line = f.readStringUntil('\n');
    f.close();
    
    int c1 = line.indexOf(',');
    int c2 = line.indexOf(',', c1 + 1);
    if (c1 < 0 || c2 < 0) { Serial.println("[IR] Corrupt format"); return; }
    
    int type = line.substring(0, c1).toInt();
    unsigned long val = strtoul(line.substring(c1 + 1, c2).c_str(), NULL, 16);
    int bits = line.substring(c2 + 1).toInt();
    
    Serial.printf("[IR] Playing -> Type:%d Val:%lX Bits:%d\n", type, val, bits);
    irsend.send(type, val, bits);
}

void ir_match_mode() {
    Serial.println("[IR] Match Mode. Waiting for signal...");
    if (irrecv.decode(&results)) {
        Serial.printf("[IR] Match Check -> Type:%d Val:%lX\n", results.decode_type, results.value);
        irrecv.resume();
    }
}
