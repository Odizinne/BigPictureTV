#ifndef STEAMWINDOWMANAGER_H
#define STEAMWINDOWMANAGER_H

#include <vector>
#include <string>

std::string getBigPictureWindowTitle();
bool isBigPictureRunning();
std::string getSteamLanguage();
std::string getRegistryValue(const std::wstring& keyPath, const std::wstring& valueName);
std::vector<std::wstring> getAllWindowTitles();
std::vector<std::string> extractWords(const std::string& str);
std::vector<std::string> extractWords(const std::wstring& wstr);
std::string cleanString(const std::string& str);
std::string cleanString(const std::wstring& wstr);
std::string convertWStringToString(const std::wstring& wstr);
std::wstring convertStringToWString(const std::string& str);
std::string toLower(const std::string& str);
std::string toLower(const std::wstring& wstr);

#endif // STEAMWINDOWMANAGER_H
