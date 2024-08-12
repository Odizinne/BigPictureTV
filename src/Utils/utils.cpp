#include "utils.h"
#include <QProcess>
#include <QDebug>
#include <windows.h>
#include <iostream>
#include <string>
#include <tlhelp32.h>
#include <filesystem>

const std::wstring DISCORD_EXECUTABLE_NAME = L"Update.exe";
const std::wstring DISCORD_PROCESS_NAME = L"Discord.exe";

void runDisplayswitch(const QString &command)
{
    QProcess process;
    process.start("displayswitch.exe", QStringList() << command);

    if (!process.waitForStarted()) {
        qWarning() << "Failed to start process:" << process.errorString();
        return;
    }

    if (!process.waitForFinished()) {
        qWarning() << "Process did not finish:" << process.errorString();
        return;
    }

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

    if (!output.isEmpty()) {
        qDebug() << "Process Output:" << output;
    }

    if (!error.isEmpty()) {
        qWarning() << "Process Error:" << error;
    }
}

std::string getTheme() {
    // Determine the theme based on registry value
    const std::wstring registryKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
    const std::wstring valueName = L"AppsUseLightTheme";

    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, registryKey.c_str(), 0, KEY_QUERY_VALUE, &hKey);

    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to open registry key." << std::endl;
        return "unknown";
    }

    DWORD value = 1; // Default to 1 (dark theme) if not found
    DWORD valueSize = sizeof(value);
    result = RegQueryValueExW(hKey, valueName.c_str(), NULL, NULL, reinterpret_cast<LPBYTE>(&value), &valueSize);

    RegCloseKey(hKey);

    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to query registry value." << std::endl;
        return "unknown";
    }

    // Return the opposite to match icon (dark icon on light theme)
    std::string theme = (value == 0) ? "light" : "dark";
    return theme;
}

QIcon getIconForTheme() {
    std::string theme = getTheme();
    QString themeStr = QString::fromStdString(theme);
    QString iconPath = QString(":/icons/icon_%1.png").arg(themeStr);
    return QIcon(iconPath);
}

bool switchPowerPlan(const std::wstring& planGuid) {
    std::wstring command = L"powercfg /s " + planGuid;
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    BOOL success = CreateProcessW(
        NULL,
        &command[0],
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
        );

    if (success) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return true;
    } else {
        // Process creation failed
        return false;
    }
}

std::wstring getDiscordPath() {
    wchar_t localAppData[MAX_PATH];
    if (GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH) == 0) {
        std::wcerr << L"Failed to get LOCALAPPDATA environment variable" << std::endl;
        return L"";
    }

    return std::wstring(localAppData) + L"\\Discord\\" + DISCORD_EXECUTABLE_NAME;
}

bool isDiscordInstalled() {
    std::wstring discordPath = getDiscordPath();
    return std::filesystem::exists(discordPath);
}

void closeDiscord() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    PROCESSENTRY32 pe = { sizeof(pe) };
    if (Process32First(hSnapshot, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, DISCORD_PROCESS_NAME.c_str()) == 0) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProcess) {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                }
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
}

void startDiscord() {
    std::wstring discordPath = getDiscordPath();
    if (discordPath.empty()) {
        std::wcerr << L"Failed to get Discord path" << std::endl;
        return;
    }

    QString discordPathQString = QString::fromStdWString(discordPath);
    QStringList arguments;
    arguments << "--processStart" << "Discord.exe" << "--process-start-args" << "--start-minimized";

    QProcess process;
    process.setProgram(discordPathQString);
    process.setArguments(arguments);

    if (!process.startDetached()) {
        std::wcerr << L"Failed to start Discord with arguments" << std::endl;
    }
}


bool isAudioDeviceCmdletsInstalled() {
    QProcess process;
    process.setProgram("powershell");
    process.setArguments({"-Command", "Get-Module -ListAvailable -Name AudioDeviceCmdlets"});
    process.start();
    if (!process.waitForStarted()) {
        qWarning() << "Failed to start process.";
        return false;
    }

    if (!process.waitForFinished()) {
        qWarning() << "Process did not finish correctly.";
        return false;
    }

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

    if (!error.isEmpty()) {
        qWarning() << "Error:" << error;
    }

    return output.contains("AudioDeviceCmdlets", Qt::CaseInsensitive);
}
