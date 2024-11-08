#include "Utils.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>
#include <QCoreApplication>
#include <QFileInfo>

QColor adjustColor(const QColor &color, double factor) {
    int r = color.red();
    int g = color.green();
    int b = color.blue();
    int a = color.alpha();

    r = std::min(std::max(static_cast<int>(r * factor), 0), 255);
    g = std::min(std::max(static_cast<int>(g * factor), 0), 255);
    b = std::min(std::max(static_cast<int>(b * factor), 0), 255);

    return QColor(r, g, b, a);
}

bool isDarkMode(const QColor &color) {
    int r = color.red();
    int g = color.green();
    int b = color.blue();
    double brightness = (r + g + b) / 3.0;
    return brightness < 127;
}

void Utils::setFrameColorBasedOnWindow(QWidget *window, QFrame *frame) {
    QColor main_bg_color = window->palette().color(QPalette::Window);
    QColor frame_bg_color;

    if (isDarkMode(main_bg_color)) {
        frame_bg_color = adjustColor(main_bg_color, 1.75);  // Brighten color
    } else {
        frame_bg_color = adjustColor(main_bg_color, 0.95);  // Darken color
    }

    QPalette palette = frame->palette();
    palette.setBrush(QPalette::Window, QBrush(frame_bg_color));
    frame->setAutoFillBackground(true);
    frame->setPalette(palette);
}

void Utils::runEnhancedDisplayswitch(const QString &command) {
    QProcess process;
    QString executablePath = "dependencies/EnhancedDisplaySwitch.exe";
    process.start(executablePath, QStringList() << command);
    process.waitForFinished();
}

QString getTheme() {
    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat);
    int value = settings.value("AppsUseLightTheme", 1).toInt();

    return (value == 0) ? "light" : "dark";
}

QIcon Utils::getIconForTheme() {
    QString theme = getTheme();
    QString iconPath = QString(":/icons/icon_%1.png").arg(theme);
    return QIcon(iconPath);
}

QString Utils::getActivePowerPlan() {
    QString regPath = "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Power\\User\\PowerSchemes";
    QString regKey = "ActivePowerScheme";

    QSettings settings(regPath, QSettings::NativeFormat);
    QString activeSchemeGuid = settings.value(regKey).toString();

    return activeSchemeGuid;
}

void Utils::setPowerPlan(QString planGuid) {
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

QString getDiscordPath() {
    QString discordExecutableName = "Update.exe";
    QString localAppData = qgetenv("LOCALAPPDATA");
    if (localAppData.isEmpty()) {
        qWarning() << "Failed to get LOCALAPPDATA environment variable";
        return QString();
    }

    return localAppData + "/Discord/" + discordExecutableName;
}

bool Utils::isDiscordInstalled() {
    QString discordPath = getDiscordPath();
    return QFileInfo::exists(discordPath);
}

bool Utils::isDiscordRunning() {
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

void Utils::closeDiscord() {
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

void Utils::startDiscord() {
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

void Utils::sendMediaKey(WORD keyCode) {
    INPUT ip = {0};
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = keyCode;
    SendInput(1, &ip, sizeof(INPUT));
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));
}
