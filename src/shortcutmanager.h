#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <string>
void setPaths(std::wstring& targetPath, std::wstring& startupFolder);
std::wstring getStartupFolder();
std::wstring getShortcutPath();
void createShortcut(const std::wstring& targetPath);
void removeShortcut();
bool isShortcutPresent();
void manageShortcut(bool state);

#endif // SHORTCUTMANAGER_H
