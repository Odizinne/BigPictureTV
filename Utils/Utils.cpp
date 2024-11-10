#include "Utils.h"
#include <QTranslator>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>
#include <QCoreApplication>
#include <QFileInfo>

void Utils::runDisplayswitch(const QString &command)
{
    QProcess process;
    process.start("displayswitch.exe", QStringList() << command);
    process.waitForFinished();
}

QString getTheme()
{
    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat);
    int value = settings.value("AppsUseLightTheme", 1).toInt();

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

QString getDiscordPath()
{
    QString discordExecutableName = "Update.exe";
    QString localAppData = qgetenv("LOCALAPPDATA");
    if (localAppData.isEmpty()) {
        qWarning() << "Failed to get LOCALAPPDATA environment variable";
        return QString();
    }

    return localAppData + "/Discord/" + discordExecutableName;
}

bool Utils::isDiscordInstalled()
{
    QString discordPath = getDiscordPath();
    return QFileInfo::exists(discordPath);
}

bool Utils::isDiscordRunning()
{
    QString discordProcessName = "Discord.exe";
    QProcess process;
    process.start("tasklist.exe", QStringList() << "/FI" << QString("IMAGENAME eq %1").arg(discordProcessName));

    if (!process.waitForFinished()) {
        qWarning() << "Failed to execute tasklist command";
        return false;
    }

    QString output = process.readAllStandardOutput();
    return output.contains(discordProcessName, Qt::CaseInsensitive);
}

void Utils::closeDiscord()
{
    QString discordProcessName = "Discord.exe";
    QStringList arguments;
    arguments << "/IM" << discordProcessName
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
    QString discordProcessName = "Discord.exe";
    QString discordPath = getDiscordPath();
    if (discordPath.isEmpty()) {
        qWarning() << "Failed to get Discord path";
        return;
    }

    QStringList arguments;
    arguments << "--processStart" << discordProcessName << "--process-start-args"
              << "--start-minimized";

    qint64 processId;
    bool success = QProcess::startDetached(discordPath, arguments, QString(), &processId);

    if (!success) {
        qWarning() << "Failed to start Discord with arguments";
    }
}

void Utils::sendMediaKey(WORD keyCode)
{
    INPUT ip = {0};
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = keyCode;
    SendInput(1, &ip, sizeof(INPUT));
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));
}

void Utils::skipBigPictureIntro()
{
    // Center the mouse on the primary screen
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen) {
        QRect screenGeometry = primaryScreen->geometry();
        int centerX = screenGeometry.width() / 2;
        int centerY = screenGeometry.height() / 2;
        QCursor::setPos(centerX, centerY);
    }

    // Define the input event for mouse button down and up
    INPUT input[2] = {};

    // Mouse button down (left click)
    input[0].type = INPUT_MOUSE;
    input[0].mi.dx = 0;  // X coordinate
    input[0].mi.dy = 0;  // Y coordinate
    input[0].mi.mouseData = 0;
    input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    input[0].mi.time = 0;
    input[0].mi.dwExtraInfo = 0;

    // Mouse button up (release left click)
    input[1].type = INPUT_MOUSE;
    input[1].mi.dx = 0;  // X coordinate
    input[1].mi.dy = 0;  // Y coordinate
    input[1].mi.mouseData = 0;
    input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    input[1].mi.time = 0;
    input[1].mi.dwExtraInfo = 0;

    // Send the input events (left down and then left up)
    SendInput(2, input, sizeof(INPUT));
}

int getBuildNumber()
{
    QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", QSettings::NativeFormat);
    QVariant buildVariant = registry.value("CurrentBuild");

    if (!buildVariant.isValid()) {
        buildVariant = registry.value("CurrentBuildNumber");
    }

    if (buildVariant.isValid() && buildVariant.canConvert<QString>()) {
        bool ok;
        int buildNumber = buildVariant.toString().toInt(&ok);
        if (ok) {
            return buildNumber;
        }
    }

    qDebug() << "Failed to retrieve build number from the registry.";
    return -1;
}


bool Utils::isWindows10()
{
    int buildNumber = getBuildNumber();
    return (buildNumber >= 10240 && buildNumber < 22000);
}
