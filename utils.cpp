#include "utils.h"
#include <QProcess>
#include <QDebug>
#include <QIcon>
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
    // Define the registry key and value name
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

    // Determine the theme based on the registry value
    // Return the opposite to match icon (dark icon on light theme)
    std::string theme = (value == 0) ? "light" : "dark";
    return theme;
}

QIcon getIconForTheme() {
    // Get the system theme
    std::string theme = getTheme();

    // Convert theme to QString
    QString themeStr = QString::fromStdString(theme);

    // Construct the icon path based on the theme
    QString iconPath = QString(":/icons/icon_%1.png").arg(themeStr);

    // Return the QIcon object
    return QIcon(iconPath);
}

bool switchPowerPlan(const std::wstring& planGuid) {
    // Construct the command string
    std::wstring command = L"powercfg /s " + planGuid;

    // Convert the command string to a wide-character C-string
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    // Create the process
    BOOL success = CreateProcessW(
        NULL,                   // Application name
        &command[0],            // Command line
        NULL,                   // Process security attributes
        NULL,                   // Primary thread security attributes
        FALSE,                  // Handle inheritance
        0,                      // Creation flags
        NULL,                   // Environment
        NULL,                   // Current directory
        &si,                    // Startup info
        &pi                     // Process information
        );

    if (success) {
        // Wait until the process exits
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles
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
    // Create a QProcess instance
    QProcess process;

    // Set up the command to execute
    process.setProgram("powershell");
    process.setArguments({"-Command", "Get-Module -ListAvailable -Name AudioDeviceCmdlets"});

    // Start the process
    process.start();
    if (!process.waitForStarted()) {
        qWarning() << "Failed to start process.";
        return false;
    }

    // Wait for the process to finish
    if (!process.waitForFinished()) {
        qWarning() << "Process did not finish correctly.";
        return false;
    }

    // Get the output from the process
    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

    if (!error.isEmpty()) {
        qWarning() << "Error:" << error;
    }

    // Check if the output contains the module name
    return output.contains("AudioDeviceCmdlets", Qt::CaseInsensitive);
}
