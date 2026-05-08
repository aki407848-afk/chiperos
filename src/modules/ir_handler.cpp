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
        irrecv.resume();
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
    
    // ✅ ИСПРАВЛЕНО: конвертируем int в decode_type_t
    int typeInt = line.substring(0, c1).toInt();
    decode_type_t type = (decode_type_t)typeInt;  // ✅ Явное приведение типа
    
    unsigned long val = strtoul(line.substring(c1 + 1, c2).c_str(), NULL, 16);
    int bits = line.substring(c2 + 1).toInt();
    
    Serial.printf("[IR] Playing -> Type:%d Val:%lX Bits:%d\n", type, val, bits);    
    // ✅ ИСПРАВЛЕНО: используем правильный send()
    switch (type) {
        case NEC:
            irsend.sendNEC(val, bits);
            break;
        case SONY:
            irsend.sendSony(val, bits);
            break;
        case RC5:
            irsend.sendRC5(val, bits);
            break;
        case RC6:
            irsend.sendRC6(val, bits);
            break;
        case PANASONIC:
            irsend.sendPanasonic64(val, bits);
            break;
        case JVC:
            irsend.sendJVC(val, bits);
            break;
        case SAMSUNG:
            irsend.sendSAMSUNG(val, bits);
            break;
        case SANYO:
            irsend.sendSanyoLC7461(val, bits);
            break;
        case MITSUBISHI:
            irsend.sendMitsubishi(val, bits);
            break;
        case DISH:
            irsend.sendDISH(val, bits);
            break;
        case SHARP:
            irsend.sendSharpRaw(val, bits);
            break;
        case DENON:
            irsend.sendDenon(val, bits);
            break;
        case COOLIX:
            irsend.sendCOOLIX(val, bits);
            break;
        case WHIRLPOOL:
            irsend.sendWhirlpool(val, bits);
            break;
        case SHERWOOD:
            irsend.sendSherwood(val, bits);
            break;
        default:
            Serial.println("[IR] Unknown protocol, using generic send");            irsend.send(type, val, bits);  // Для неизвестных типов
            break;
    }
}

void ir_match_mode() {
    Serial.println("[IR] Match Mode. Waiting for signal...");
    if (irrecv.decode(&results)) {
        Serial.printf("[IR] Match Check -> Type:%d Val:%lX\n", results.decode_type, results.value);
        irrecv.resume();
    }
}
