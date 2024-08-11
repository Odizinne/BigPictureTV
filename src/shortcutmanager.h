#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <string>

void setPaths(const std::wstring& executableName, const std::wstring& shortcutName,
              std::wstring& targetPath, std::wstring& startupFolder, std::wstring& shortcutPath);
std::wstring getStartupFolder();
void createShortcut(const std::wstring& targetPath, const std::wstring& shortcutPath);
void removeShortcut(const std::wstring& shortcutPath);
bool isShortcutPresent(const std::wstring& shortcutPath);
void manageShortcut(const std::wstring& shortcutName, bool state);

#endif // SHORTCUTMANAGER_H
