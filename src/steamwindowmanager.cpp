#include "steamwindowmanager.h"
#include <windows.h>
#include <winreg.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <sstream>
#include <algorithm>

// Define the map of window titles by language
const std::unordered_map<std::string, std::string> BIG_PICTURE_WINDOW_TITLES = {
    {"schinese", "Steam 大屏幕模式"},
    {"tchinese", "Steam Big Picture 模式"},
    {"japanese", "Steam Big Pictureモード"},
    {"koreana", "Steam Big Picture 모드"},
    {"thai", "โหมด Big Picture บน Steam"},
    {"bulgarian", "Steam режим „Голям екран“"},
    {"czech", "Steam režim Big Picture"},
    {"danish", "Steam Big Picture-tilstand"},
    {"german", "Big-Picture-Modus"},
    {"english", "Steam Big Picture mode"},
    {"spanish", "Modo Big Picture de Steam"},
    {"latam", "Modo Big Picture de Steam"},
    {"greek", "Steam Λειτουργία Big Picture"},
    {"french", "Steam mode Big Picture"},
    {"indonesian", "Mode Big Picture Steam"},
    {"italian", "Modalità Big Picture di Steam"},
    {"hungarian", "Steam Nagy Kép mód"},
    {"dutch", "Steam Big Picture-modus"},
    {"norwegian", "Steam Big Picture-modus"},
    {"polish", "Tryb Big Picture Steam"},
    {"portuguese", "Steam Big Picture"},
    {"brazilian", "Steam Modo Big Picture"},
    {"romanian", "Steam modul Big Picture"},
    {"russian", "Режим Big Picture"},
    {"finnish", "Steamin televisiotila"},
    {"swedish", "Steams Big Picture-läge"},
    {"turkish", "Steam Geniş Ekran Modu"},
    {"vietnamese", "Chế độ Big Picture trên Steam"},
    {"ukrainian", "Steam у режимі Big Picture"}
};

std::string getBigPictureWindowTitle() {
    std::string language = getSteamLanguage();
    auto it = BIG_PICTURE_WINDOW_TITLES.find(language);
    if (it != BIG_PICTURE_WINDOW_TITLES.end()) {
        return it->second;
    }
    return BIG_PICTURE_WINDOW_TITLES.at("english");
}

bool isBigPictureRunning() {
    std::string bigPictureTitle = getBigPictureWindowTitle();
    std::vector<std::string> bigPictureWords = extractWords(bigPictureTitle);

    std::vector<std::wstring> currentWindowTitles = getAllWindowTitles();
    for (const auto& windowTitle : currentWindowTitles) {
        std::string windowTitleStr = convertWStringToString(windowTitle);
        std::vector<std::string> windowWords = extractWords(windowTitleStr);

        bool allWordsFound = std::all_of(bigPictureWords.begin(), bigPictureWords.end(),
                                         [&windowWords](const std::string& word) {
                                             return std::find(windowWords.begin(), windowWords.end(), word) != windowWords.end();
                                         });

        if (allWordsFound) {
            return true;
        }
    }
    return false;
}

std::string getSteamLanguage() {
    return getRegistryValue(L"Software\\Valve\\Steam\\steamglobal", L"Language");
}

std::string getRegistryValue(const std::wstring& keyPath, const std::wstring& valueName) {
    HKEY hKey;
    WCHAR value[256];
    DWORD valueLength = sizeof(value);
    std::string result;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueEx(hKey, valueName.c_str(), NULL, NULL, (LPBYTE)value, &valueLength) == ERROR_SUCCESS) {
            result = toLower(convertWStringToString(value));
        }
        RegCloseKey(hKey);
    }

    return result;
}

std::vector<std::wstring> getAllWindowTitles() {
    std::vector<std::wstring> titles;
    HWND hwnd = GetTopWindow(NULL);

    while (hwnd) {
        wchar_t windowTitle[256];
        GetWindowText(hwnd, windowTitle, sizeof(windowTitle) / sizeof(wchar_t));
        if (wcslen(windowTitle) > 0) {
            titles.push_back(windowTitle);
        }
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }

    return titles;
}

std::vector<std::string> extractWords(const std::string& str) {
    std::string cleanedStr = cleanString(str);
    std::istringstream stream(cleanedStr);
    std::string word;
    std::vector<std::string> words;
    while (stream >> word) {
        words.push_back(word);
    }
    return words;
}

std::vector<std::string> extractWords(const std::wstring& wstr) {
    return extractWords(convertWStringToString(wstr));
}

std::string cleanString(const std::string& str) {
    std::string cleanedStr;
    for (char ch : str) {
        if (ch == (char)0xA0) {  // Replace non-breaking space with regular space
            cleanedStr += ' ';
        } else if (std::isalnum(static_cast<unsigned char>(ch)) || std::isspace(static_cast<unsigned char>(ch))) {
            cleanedStr += ch;
        }
    }
    return cleanedStr;
}

std::string cleanString(const std::wstring& wstr) {
    return cleanString(convertWStringToString(wstr));
}

std::string convertWStringToString(const std::wstring& wstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, nullptr, nullptr);
    return str;
}

std::wstring convertStringToWString(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

std::string toLower(const std::wstring& wstr) {
    return toLower(convertWStringToString(wstr));
}
