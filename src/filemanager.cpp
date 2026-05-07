#include "filemanager.h"
FileManager fm;

bool FileManager::init() {
    return SD.begin() || LittleFS.begin();
}

void FileManager::listDir(const String& path) {
    currentPath = path;
    dirList.clear(); isDir.clear(); listIndex = 0;
    if (currentPath != "/") { dirList.push_back(".."); isDir.push_back(true); }
    
    File root = SD.open(currentPath);
    if(!root) root = LittleFS.open(currentPath);
    if(!root) return;
    
    File file = root.openNextFile();
    while(file) {
        String name = file.name();
        name.remove(0, currentPath.length());
        if(currentPath != "/") name.remove(0, 1);
        if(name.length() > 0) {
            dirList.push_back(name);
            isDir.push_back(file.isDirectory());
        }
        file = root.openNextFile();
    }
    root.close();
}

void FileManager::select() {
    if(listIndex < 0 || listIndex >= dirList.size()) return;
    String target = dirList[listIndex];
    if(target == "..") { goUp(); return; }
    String nextPath = currentPath;
    if(nextPath != "/") nextPath += "/";
    nextPath += target;
    if(isDir[listIndex]) listDir(nextPath);
}

void FileManager::goUp() {
    if(currentPath == "/") return;
    int lastSlash = currentPath.lastIndexOf('/');
    if(lastSlash == 0) listDir("/");
    else listDir(currentPath.substring(0, lastSlash));
}

String FileManager::getFileContent(const String& path) {
    File f = SD.open(path);
    if(!f) f = LittleFS.open(path);
    if(!f) return "ERR: OPEN";
    String content = "";
    while(f.available()) content += (char)f.read();
    f.close();
    return content.substring(0, 200);
}
