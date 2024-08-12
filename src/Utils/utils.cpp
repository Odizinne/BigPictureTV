#include "utils.h"
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QDebug>
#include <windows.h>
#include <iostream>
#include <string>
#include <tlhelp32.h>
#include <windows.h>
#include <filesystem>

const std::wstring DISCORD_EXECUTABLE_NAME = L"Update.exe";
const std::wstring DISCORD_PROCESS_NAME = L"Discord.exe";

void runDisplayswitch(const QString &command)
{
    QProcess process;
    process.start("displayswitch.exe", QStringList() << command);
    process.waitForFinished();  // Wait for the process to finish

    QString appDataBasePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (appDataBasePath.isEmpty()) {
        return;
    }
    QString statusFilePath = appDataBasePath + "/displayswitch.txt";
    QString commandToStore = command.startsWith('/') ? command.mid(1) : command;
    QFile file(statusFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    out << commandToStore;
    file.close();
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

void switchPowerPlan(const std::wstring& planGuid) {
    // Convert std::wstring to QString
    QString planGuidQString = QString::fromStdWString(planGuid);

    // Construct the command and arguments
    QString command = "powercfg";
    QStringList arguments;
    arguments << "/s" << planGuidQString;

    QProcess process;
    process.start(command, arguments);
    if (!process.waitForFinished()) {
        // Log or handle error
        qWarning() << "Failed to execute powercfg command:" << process.errorString();
        return;
    }

    // Check the exit code
    if (process.exitCode() != 0) {
        // Log or handle error
        qWarning() << "powercfg command failed with exit code:" << process.exitCode();
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
    // Define the command to kill Discord.exe
    QStringList arguments;
    arguments << "/IM" << "Discord.exe" << "/F";  // /IM specifies the image name, /F forces termination

    QProcess process;
    process.start("taskkill.exe", arguments);

    if (!process.waitForFinished()) {
        std::wcerr << L"Failed to execute taskkill command to kill Discord" << std::endl;
    } else {
        std::wcout << L"Taskkill command executed successfully" << std::endl;
    }
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

    qint64 processId;
    bool success = QProcess::startDetached(discordPathQString, arguments, QString(), &processId);

    if (!success) {
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

// Function to check if the OS is Windows 10 by querying the registry
bool isWindows10() {
    HKEY hKey;
    RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                  L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                  0, KEY_READ, &hKey);

    WCHAR buildNumberString[256];
    DWORD bufferSize = sizeof(buildNumberString);
    RegQueryValueExW(hKey, L"CurrentBuild", NULL, NULL,
                     reinterpret_cast<LPBYTE>(buildNumberString),
                     &bufferSize);

    std::wstring buildNumberWStr(buildNumberString);
    DWORD buildNumber = std::stoi(buildNumberWStr);

    RegCloseKey(hKey);

    return (buildNumber >= 10240 && buildNumber < 22000);
}

