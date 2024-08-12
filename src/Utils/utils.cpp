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
#include <filesystem>

const std::wstring DISCORD_EXECUTABLE_NAME = L"Update.exe";
const std::wstring DISCORD_PROCESS_NAME = L"Discord.exe";

void runDisplayswitch(const QString &command)
{
    QProcess process;
    process.start("displayswitch.exe", QStringList() << command);

    QString appDataBasePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (appDataBasePath.isEmpty()) {
        qWarning() << "Failed to get AppData path";
        return;
    }

    // Create the "displayswitch_history" directory under AppData
    QString historyDirPath = appDataBasePath + "/displayswitch_history";
    QDir dir(historyDirPath);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create directory:" << historyDirPath;
            return;
        }
    }

    // Write the command to the "displayswitch.txt" file
    QFile file(dir.filePath("displayswitch.txt"));
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << command.mid(1);  // Write the command without the leading '/'
        file.close();
    } else {
        qWarning() << "Could not open file for writing:" << file.errorString();
    }

    if (process.waitForFinished()) {
        return;
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
    // Convert std::wstring to QString
    QString planGuidQString = QString::fromStdWString(planGuid);

    // Construct the command string
    QString command = QString("powercfg /s %1").arg(planGuidQString);

    QProcess process;
    process.start(command);
    if (!process.waitForFinished()) {
        // Process execution failed
        return false;
    }

    // Check the exit code if needed
    int exitCode = process.exitCode();
    if (exitCode == 0) {
        return true;
    } else {
        // Command execution failed or returned an error
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
