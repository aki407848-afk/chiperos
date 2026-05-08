#include "modules.h"
#include "hal.h"

void sdk_list_files() {
    String list = fs_list_dir("/");
    Serial.println("=== SDK FILE LIST ===");
    Serial.println(list);
}

bool sdk_delete_file(const String& path) {
    Serial.printf("[SDK] Deleting %s...\n", path.c_str());
    return fs_delete_file(path);
}

bool sdk_rename_file(const String& oldP, const String& newP) {
    Serial.printf("[SDK] Renaming %s -> %s...\n", oldP.c_str(), newP.c_str());
    return fs_rename_file(oldP, newP);
}

String sdk_read_file(const String& path) {
    File f = SD.open(path, FILE_READ);
    if (!f) return "ERR: OPEN";
    String content = "";
    while (f.available()) {
        content += (char)f.read();
        if (content.length() > 500) break; // Лимит для RAM
    }
    f.close();
    return content;
}
