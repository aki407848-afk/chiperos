#pragma once
#include <FS.h>
#include <SD.h>
#include <LittleFS.h>
#include <vector>
#include <Arduino.h>

struct FileManager {
    String currentPath = "/";
    std::vector<String> dirList;
    std::vector<bool> isDir;
    int listIndex = 0;
    
    bool init();
    void listDir(const String& path);
    void select();
    void goUp();
    String getFileContent(const String& path);
};
extern FileManager fm;
