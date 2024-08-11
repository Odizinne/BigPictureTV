#include "utils.h"

Utils::Utils() {}

#include <windows.h>

bool Utils::isWindows10() {
    OSVERSIONINFOEX info;
    ZeroMemory(&info, sizeof(OSVERSIONINFOEX));
    info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if (GetVersionEx((OSVERSIONINFO*)&info)) {
        // Check if the major version is 10 and the minor version is 0 (which corresponds to Windows 10)
        if (info.dwMajorVersion == 10 && info.dwMinorVersion == 0) {
            return true;
        }
    }

    return false;
}
