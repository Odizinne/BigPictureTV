#ifndef STEAMWINDOWMANAGER_H
#define STEAMWINDOWMANAGER_H

#include <string>
#include <vector>
#include <windows.h>
#include <unordered_map>

// Function to get the Big Picture window title based on the current Steam language
std::string getBigPictureWindowTitle();

// Function to check if the Big Picture mode is currently running
bool isBigPictureRunning();

// Function to get the current Steam language from the registry
std::string getSteamLanguage();

// Function to retrieve a value from the Windows registry
std::string getRegistryValue(const std::wstring& keyPath, const std::wstring& valueName);

// Callback function for enumerating all top-level windows
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

// Function to get all window titles currently open
std::vector<std::wstring> getAllWindowTitles();

// Function to extract words from a given string
std::vector<std::string> extractWords(const std::string& str);
std::vector<std::string> extractWords(const std::wstring& wstr);

// Function to clean a string by removing non-alphanumeric characters
std::string cleanStringFromASCII(const std::string& str);
std::wstring cleanStringFromWide(const std::wstring& wstr);

// Function to convert between wide strings and standard strings
std::string convertWStringToString(const std::wstring& wstr);
std::wstring convertStringToWString(const std::string& str);

// Function to convert a string to lowercase
std::string toLower(const std::string& str);
std::string toLower(const std::wstring& wstr);

// Global or static variable to store window titles
extern std::vector<std::wstring> g_windowTitles;

#endif // STEAMWINDOWMANAGER_H
