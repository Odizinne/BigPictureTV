#ifndef NIGHTLIGHTSWITCHER_H
#define NIGHTLIGHTSWITCHER_H

#include <windows.h>
#include <string>
#include <vector>

class NightLightSwitcher {
private:
    static const std::wstring keyPath;
    HKEY hKey;

    // Convert hex string to byte array
    std::vector<BYTE> hexToBytes(const std::wstring& hex);
    
    // Convert byte array to hex string
    std::wstring bytesToHex(const std::vector<BYTE>& bytes);

public:
    NightLightSwitcher();
    ~NightLightSwitcher();

    bool supported() const;
    bool enabled();
    void enable();
    void disable();
    void toggle();
};

#endif // NIGHTLIGHTSWITCHER_H
