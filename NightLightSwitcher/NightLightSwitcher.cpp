#include "NightLightSwitcher.h"
#include <QDebug>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <Windows.h>

HKEY hKey = nullptr;

void init() {
    const std::wstring keyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\CloudStore\\Store\\DefaultAccount\\Current\\default$windows.data.bluelightreduction.bluelightreductionstate\\windows.data.bluelightreduction.bluelightreductionstate";

    if (RegOpenKeyEx(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        hKey = nullptr;
    }
}

bool NightLightSwitcher::supported() {
    init();
    return hKey != nullptr;
}

bool NightLightSwitcher::enabled() {
    if (!supported()) return false;

    BYTE data[1024];
    DWORD dataSize = sizeof(data);
    if (RegQueryValueEx(hKey, L"Data", NULL, NULL, data, &dataSize) != ERROR_SUCCESS) {
        qDebug() << "Failed to query registry value.";
        return false;
    }

    std::vector<BYTE> bytes(data, data + dataSize);
    if (bytes.size() < 19) return false; // Ensure enough data length
    return bytes[18] == 0x15; // 21 in decimal
}

void NightLightSwitcher::enable() {
    if (supported() && !enabled()) {
        toggle();
    }
}

void NightLightSwitcher::disable() {
    if (supported() && enabled()) {
        toggle();
    }
}

void NightLightSwitcher::toggle() {
    if (!supported()) return;

    BYTE data[1024];
    DWORD dataSize = sizeof(data);
    if (RegQueryValueEx(hKey, L"Data", NULL, NULL, data, &dataSize) != ERROR_SUCCESS) {
        qDebug() << "Failed to query registry value.";
        return;
    }

    std::vector<BYTE> newData;

    bool currentlyEnabled = enabled();

    if (currentlyEnabled) {
        // Allocate 41 bytes and modify the necessary fields
        newData.resize(41, 0);
        std::copy(data, data + 22, newData.begin());
        std::copy(data + 25, data + 43, newData.begin() + 23);
        newData[18] = 0x13; // Disable
    } else {
        // Allocate 43 bytes and modify the necessary fields
        newData.resize(43, 0);
        std::copy(data, data + 22, newData.begin());
        std::copy(data + 23, data + 41, newData.begin() + 25);
        newData[18] = 0x15; // Enable
        newData[23] = 0x10;
        newData[24] = 0x00;
    }

    // Increment bytes from index 10 to 14
    for (int i = 10; i < 15; ++i) {
        if (newData[i] != 0xff) {
            newData[i]++;
            break;
        }
    }

    // Set the modified data back to the registry
    DWORD newDataSize = static_cast<DWORD>(newData.size());
    if (RegSetValueEx(hKey, L"Data", 0, REG_BINARY, newData.data(), newDataSize) != ERROR_SUCCESS) {
        qDebug() << "Failed to update registry value.";
    }
}

std::vector<BYTE> hexToBytes(const std::wstring& hex) {
    std::vector<BYTE> bytes;
    size_t len = hex.size();
    for (size_t i = 0; i < len; i += 2) {
        BYTE byte = static_cast<BYTE>(std::stoi(hex.substr(i, 2), nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

std::wstring bytesToHex(const std::vector<BYTE>& bytes) {
    std::wstringstream ss;
    for (BYTE byte : bytes) {
        ss << std::hex << std::setw(2) << std::setfill(L'0') << static_cast<int>(byte);
    }
    return ss.str();
}

