#include "utils.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>
#include <QCoreApplication>
#include <QFileInfo>

const QString DISCORD_EXECUTABLE_NAME = "Update.exe";
const QString DISCORD_PROCESS_NAME = "Discord.exe";
const QString SUNSHINE_STATUS_FILE = QStandardPaths::writableLocation(
                                               QStandardPaths::AppDataLocation)
                                           + "/sunshine-status/status.txt";

Utils::Utils() {}

Utils::~Utils() {}

void Utils::runEnhancedDisplayswitch(const QString &command)
{
    QProcess process;
    QString executablePath = "dependencies/EnhancedDisplaySwitch.exe";
    process.start(executablePath, QStringList() << command);
    process.waitForFinished();
}

QString Utils::getTheme()
{
    // Determine the theme based on registry value
    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat);
    int value = settings.value("AppsUseLightTheme", 1).toInt();

    // Return the opposite to match icon (dark icon on light theme)
    return (value == 0) ? "light" : "dark";
}

QIcon Utils::getIconForTheme()
{
    QString theme = getTheme();
    QString iconPath = QString(":/icons/icon_%1.png").arg(theme);
    return QIcon(iconPath);
}

QString Utils::getActivePowerPlan()
{
    QString regPath = "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Power\\User\\PowerSchemes";
    QString regKey = "ActivePowerScheme";

    QSettings settings(regPath, QSettings::NativeFormat);
    QString activeSchemeGuid = settings.value(regKey).toString();

    return activeSchemeGuid;
}

void Utils::setPowerPlan(QString planGuid)
{
    QString command = "powercfg";
    QStringList arguments;
    arguments << "/s" << planGuid;

    QProcess process;
    process.start(command, arguments);
    if (!process.waitForFinished()) {
        qWarning() << "Failed to execute powercfg command:" << process.errorString();
        return;
    }

    if (process.exitCode() != 0) {
        qWarning() << "powercfg command failed with exit code:" << process.exitCode();
    }
}

QString Utils::getDiscordPath()
{
    QString localAppData = qgetenv("LOCALAPPDATA");
    if (localAppData.isEmpty()) {
        qWarning() << "Failed to get LOCALAPPDATA environment variable";
        return QString();
    }

    return localAppData + "/Discord/" + DISCORD_EXECUTABLE_NAME;
}

bool Utils::isDiscordInstalled()
{
    QString discordPath = getDiscordPath();
    return QFileInfo::exists(discordPath);
}

bool Utils::isDiscordRunning()
{
    QProcess process;
    process.start("tasklist.exe", QStringList() << "/FI" << QString("IMAGENAME eq %1").arg(DISCORD_PROCESS_NAME));

    if (!process.waitForFinished()) {
        qWarning() << "Failed to execute tasklist command";
        return false;
    }

    QString output = process.readAllStandardOutput();
    return output.contains(DISCORD_PROCESS_NAME, Qt::CaseInsensitive);
}

void Utils::closeDiscord()
{
    QStringList arguments;
    arguments << "/IM" << DISCORD_PROCESS_NAME
              << "/F";

    QProcess process;
    process.start("taskkill.exe", arguments);

    if (!process.waitForFinished()) {
        qWarning() << "Failed to execute taskkill command to kill Discord";
    } else {
        qDebug() << "Taskkill command executed successfully";
    }
}

void Utils::startDiscord()
{
    QString discordPath = getDiscordPath();
    if (discordPath.isEmpty()) {
        qWarning() << "Failed to get Discord path";
        return;
    }

    QStringList arguments;
    arguments << "--processStart" << DISCORD_PROCESS_NAME << "--process-start-args"
              << "--start-minimized";

    qint64 processId;
    bool success = QProcess::startDetached(discordPath, arguments, QString(), &processId);

    if (!success) {
        qWarning() << "Failed to start Discord with arguments";
    }
}

bool Utils::isAudioDeviceCmdletsInstalled()
{
    QProcess process;
    process.setProgram("powershell");
    process.setArguments({"-NoProfile", "-Command", "Get-Module -ListAvailable -Name AudioDeviceCmdlets"});
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

bool Utils::isSunshineStreaming()
{
    if (QFileInfo::exists(SUNSHINE_STATUS_FILE)) {
        return true;
    }
    return false;
}

void Utils::sendMediaKey(WORD keyCode) {
    INPUT ip = {0};
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = keyCode;

    // Press the key
    SendInput(1, &ip, sizeof(INPUT));

    // Release the key
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));
}
