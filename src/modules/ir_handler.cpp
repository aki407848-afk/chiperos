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
    if (irrecv.decode(&results)) {
        File f = SD.open(filename, FILE_WRITE);
        if (f) {
            f.print(results.decode_type); f.print(",");
            f.print(results.value, HEX); f.print(",");
            f.println(results.bits);
            f.close();
        }
        irrecv.resume();
    }
}

void ir_play_from_sd(const char* filename) {
    File f = SD.open(filename, FILE_READ);
    if (!f) return;
    
    String line = f.readStringUntil('\n');
    f.close();
    
    int c1 = line.indexOf(',');
    int c2 = line.indexOf(',', c1 + 1);
    if (c1 < 0 || c2 < 0) return;
    
    int typeInt = line.substring(0, c1).toInt();
    decode_type_t type = (decode_type_t)typeInt;
    unsigned long val = strtoul(line.substring(c1 + 1, c2).c_str(), NULL, 16);
    int bits = line.substring(c2 + 1).toInt();
    
    // ✅ ИСПРАВЛЕНО: используем конкретные функции send
    switch (type) {
        case NEC: irsend.sendNEC(val, bits); break;
        case SONY: irsend.sendSony(val, bits); break;
        case RC5: irsend.sendRC5(val, bits); break;
        case SAMSUNG: irsend.sendSAMSUNG(val, bits); break;
        case WHIRLPOOL: irsend.sendWhirlpoolAC(val, bits); break; // ✅ Исправлено
        case PANASONIC: irsend.sendPanasonic64(val, bits); break;
        case JVC: irsend.sendJVC(val, bits); break;
        default: irsend.send(type, val, bits); break;
    }
}

void ir_match_mode() {
    if (irrecv.decode(&results)) {
        Serial.printf("[IR] Type:%d Val:%lX\n", results.decode_type, results.value);
        irrecv.resume();
    }
}
