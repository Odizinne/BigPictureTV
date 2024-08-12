#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <string>

// Function to set the target executable path and the startup folder path
void setPaths(std::wstring& targetPath, std::wstring& startupFolder);

// Function to get the Startup folder path
std::wstring getStartupFolder();

// Function to get the full path of the shortcut
std::wstring getShortcutPath();

// Function to create a shortcut in the Startup folder
void createShortcut(const std::wstring& targetPath);

// Function to remove the shortcut from the Startup folder
void removeShortcut();

// Function to check if the shortcut exists in the Startup folder
bool isShortcutPresent();

// Function to manage the shortcut (create or remove based on the state)
void manageShortcut(bool state);

#endif // SHORTCUTMANAGER_H
