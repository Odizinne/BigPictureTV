#include "steamwindowmanager.h"
#include <windows.h>
#include <winreg.h>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <vector>

// Define the map
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

// Function to get the Big Picture window title based on Steam language
std::string getBigPictureWindowTitle() {
    std::string language = getSteamLanguage();

    auto it = BIG_PICTURE_WINDOW_TITLES.find(language);
    if (it != BIG_PICTURE_WINDOW_TITLES.end()) {
        return it->second;
    }
    return BIG_PICTURE_WINDOW_TITLES.at("english");
}

// Function to check if Big Picture is running
bool isBigPictureRunning() {
    std::string bigPictureTitle = getBigPictureWindowTitle();
    std::vector<std::string> bigPictureWords = extractWords(bigPictureTitle);

    // Convert all Big Picture words to lowercase
    std::transform(bigPictureWords.begin(), bigPictureWords.end(), bigPictureWords.begin(),
                   [](const std::string& word) { return toLower(word); });

    std::vector<std::wstring> currentWindowTitles = getAllWindowTitles();
    for (const auto& windowTitle : currentWindowTitles) {
        std::string windowTitleStr = convertWStringToString(windowTitle);
        std::vector<std::string> windowWords = extractWords(windowTitleStr);

        // Convert all window words to lowercase
        std::transform(windowWords.begin(), windowWords.end(), windowWords.begin(),
                       [](const std::string& word) { return toLower(word); });

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

// Function to get the Steam language from the registry
std::string getSteamLanguage() {
    return getRegistryValue(L"Software\\Valve\\Steam\\steamglobal", L"Language");
}

// Function to get registry value
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

const std::string NON_BREAKING_SPACE = "\u00A0";

// Function to replace non-breaking spaces with regular spaces
std::string cleanString(const std::string& str) {
    std::string cleanedStr;
    size_t pos = 0;

    while (pos < str.size()) {
        if (str.compare(pos, NON_BREAKING_SPACE.size(), NON_BREAKING_SPACE) == 0) {
            cleanedStr += ' '; // Replace with regular space
            pos += NON_BREAKING_SPACE.size(); // Move past the character
        } else {
            cleanedStr += str[pos]; // Keep all other characters
            pos++;
        }
    }

    return cleanedStr;
}

std::string cleanString(const std::wstring& wstr) {
    return cleanString(convertWStringToString(wstr));
}

// Function to convert wide string to multi-byte string
std::string convertWStringToString(const std::wstring& wstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, nullptr, nullptr);
    return str;
}

// Function to convert multi-byte string to wide string
std::wstring convertStringToWString(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

// Function to convert string to lowercase
std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

std::string toLower(const std::wstring& wstr) {
    return toLower(convertWStringToString(wstr));
}

// Function to extract words from a string
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

// Global variable to store window titles
std::vector<std::wstring> g_windowTitles;

// Callback function to enumerate windows
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    std::vector<std::wstring>* titles = reinterpret_cast<std::vector<std::wstring>*>(lParam);

    // Check if the window is visible and not minimized
    if (IsWindowVisible(hwnd) && !(GetWindowLong(hwnd, GWL_STYLE) & WS_MINIMIZE)) {
        wchar_t windowTitle[256];
        if (GetWindowText(hwnd, windowTitle, sizeof(windowTitle) / sizeof(wchar_t)) > 0) {
            titles->push_back(windowTitle);
        }
    }
    return TRUE; // Continue enumeration
}

// Function to get all window titles
std::vector<std::wstring> getAllWindowTitles() {
    g_windowTitles.clear(); // Clear previous results

    // Enumerate all top-level windows
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&g_windowTitles));

    return g_windowTitles;
}
