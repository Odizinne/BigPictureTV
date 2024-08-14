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

void runDisplayswitch(const QString &command)
{
    QProcess process;
    process.start("displayswitch.exe", QStringList() << command);
    process.waitForFinished(); // Wait for the process to finish

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

QString getTheme()
{
    // Determine the theme based on registry value
    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat);
    int value = settings.value("AppsUseLightTheme", 1).toInt();

    // Return the opposite to match icon (dark icon on light theme)
    return (value == 0) ? "light" : "dark";
}

QIcon getIconForTheme()
{
    QString theme = getTheme();
    QString iconPath = QString(":/icons/icon_%1.png").arg(theme);
    return QIcon(iconPath);
}

void switchPowerPlan(const QString &planGuid)
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
    QString localAppData = qgetenv("LOCALAPPDATA");
    if (localAppData.isEmpty()) {
        qWarning() << "Failed to get LOCALAPPDATA environment variable";
        return QString();
    }

    return localAppData + "/Discord/" + DISCORD_EXECUTABLE_NAME;
}

bool isDiscordInstalled()
{
    QString discordPath = getDiscordPath();
    return QFileInfo::exists(discordPath);
}

void closeDiscord()
{
    QStringList arguments;
    arguments << "/IM" << DISCORD_PROCESS_NAME
              << "/F"; // /IM specifies the image name, /F forces termination

    QProcess process;
    process.start("taskkill.exe", arguments);

    if (!process.waitForFinished()) {
        qWarning() << "Failed to execute taskkill command to kill Discord";
    } else {
        qDebug() << "Taskkill command executed successfully";
    }
}

void startDiscord()
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

bool isAudioDeviceCmdletsInstalled()
{
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

bool isWindows10()
{
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                       QSettings::NativeFormat);
    QString buildNumberString = settings.value("CurrentBuild").toString();
    bool ok;
    int buildNumber = buildNumberString.toInt(&ok);

    if (!ok) {
        qWarning() << "Failed to parse build number";
        return false;
    }

    return (buildNumber >= 10240 && buildNumber < 22000);
}
