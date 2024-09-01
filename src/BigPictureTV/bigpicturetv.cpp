#include "bigpicturetv.h"
#include <QApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <QJsonParseError>
#include <QTimer>
#include <QDir>
#include <QFileInfo>

const QString BigPictureTV::settingsFile = QStandardPaths::writableLocation(
                                               QStandardPaths::AppDataLocation)
                                           + "/BigPictureTV/settings.json";

BigPictureTV::BigPictureTV(QObject *parent)
    : QObject(parent)
    , utils(new Utils())
    , steamWindowManager(new SteamWindowManager())
    , audioManager(new AudioManager())
    , nightLightSwitcher(new NightLightSwitcher())
    , activePowerPlan("")
    , nightLightState(false)
    , discordState(false)
    , firstRun(false)
    , paused(false)
    , windowCheckTimer(new QTimer(this))
{
    loadSettings();
    windowCheckTimer->setInterval(settings.value("checkrate").toInt());
    connect(windowCheckTimer, &QTimer::timeout, this, &BigPictureTV::checkWindowTitle);
    windowCheckTimer->start();
    createTrayIcon();
    if (firstRun) {
        showSettings();
    }
}

BigPictureTV::~BigPictureTV()
{
    delete utils;
    delete steamWindowManager;
    delete audioManager;
    delete nightLightSwitcher;
    delete windowCheckTimer;
    delete trayIcon;
    delete trayIconMenu;
    delete configAction;
    delete quitAction;
    delete configurator;
}

void BigPictureTV::createTrayIcon()
{
    trayIcon = new QSystemTrayIcon(utils->getIconForTheme(), this);
    trayIconMenu = new QMenu();
    configAction = new QAction(tr("Settings"), this);
    quitAction = new QAction(tr("Quit"), this);

    connect(configAction, &QAction::triggered, this, &BigPictureTV::showSettings);
    connect(quitAction, &QAction::triggered, this, &QApplication::quit);

    trayIconMenu->addAction(configAction);
    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);

    trayIcon->setToolTip("BigPictureTV");
    trayIcon->show();
}

void BigPictureTV::checkWindowTitle()
{
    if (settings.value("target_window").toInt() == 1) {
        if (settings.value("custom_window").toString() != "") {
            return;
        }
    }

    if (utils->isSunshineStreaming()) {
        return;
    }

    bool disableVideo = settings.value("disable_monitor").toBool();
    bool disableAudio = settings.value("disable_audio_switch").toBool();
    bool isRunning;
    if (settings.value("target_window").toInt() == 0) {
        isRunning = steamWindowManager->isBigPictureRunning();
    } else if (settings.value("target_window").toInt() == 1) {
        isRunning = steamWindowManager->isCustomWindowRunning(settings.value("custom_window").toString());
    }

    if (isRunning && !gamemodeActive) {
        gamemodeActive = true;
        handleActions(false);
        handleMonitorChanges(false, disableVideo);
        handleAudioChanges(false, disableAudio);
    } else if (!isRunning && gamemodeActive) {
        gamemodeActive = false;
        handleActions(true);
        handleMonitorChanges(true, disableVideo);
        handleAudioChanges(true, disableAudio);
    }
}

void BigPictureTV::handleMonitorChanges(bool isDesktopMode, bool disableVideo)
{
    if (disableVideo)
        return;

    int index = isDesktopMode ? settings.value("desktop_monitor").toInt()
                              : settings.value("gamemode_monitor").toInt();

    const char *command = nullptr;

    if (index == 0) {
        command = isDesktopMode ? "/internal" : "/external";
    } else if (index == 1) {
        command = isDesktopMode ? "/extend" : "/clone";
    }

    if (command) {
        utils->runEnhancedDisplayswitch(command);
    }
}

void BigPictureTV::handleAudioChanges(bool isDesktopMode, bool disableAudio)
{
    if (disableAudio)
        return;

    QString audioDevice = isDesktopMode ? settings.value("desktop_audio").toString()
                                            : settings.value("gamemode_audio").toString();

    audioManager->setAudioDevice(audioDevice.toStdString());

    try {
        audioManager->setAudioDevice(audioDevice.toStdString());
    } catch (const std::runtime_error &e) {
        qDebug() << "Error: " << e.what();
    }
}

void BigPictureTV::handleActions(bool isDesktopMode)
{
    if (settings.value("close_discord_action").toBool()) {
        handleDiscordAction(isDesktopMode);
    }
    if (settings.value("disable_nightlight").toBool()) {
        handleNightLightAction(isDesktopMode);
    }
    if (settings.value("gamemode_powerplan").toBool()) {
        handlePowerPlanAction(isDesktopMode);
    }
    if (settings.value("gamemode_pause_media").toBool()) {
        handleMediaAction(isDesktopMode);
    }
}

void BigPictureTV::handleDiscordAction(bool isDesktopMode)
{
    if (isDesktopMode) {
        if (discordState) {
            utils->startDiscord();
        }
    } else {
        discordState = utils->isDiscordRunning();
        utils->closeDiscord();
    }
}

void BigPictureTV::handleNightLightAction(bool isDesktopMode)
{
    if (isDesktopMode) {
        if (nightLightState) {
            nightLightSwitcher->enable();
        }
    } else {
        nightLightState = nightLightSwitcher->enabled();
        nightLightSwitcher->disable();
    }
}

void BigPictureTV::handleMediaAction(bool isDesktopMode)
{
    if (!isDesktopMode) {
        utils->sendMediaKey(VK_MEDIA_STOP);
    }
}

void BigPictureTV::handlePowerPlanAction(bool isDesktopMode)
{
    if (isDesktopMode) {
        if (!activePowerPlan.isEmpty()) {
            utils->setPowerPlan(activePowerPlan);
        } else {
            utils->setPowerPlan("381b4222-f694-41f0-9685-ff5bb260df2e");
        }
    } else {
        activePowerPlan = utils->getActivePowerPlan();
        utils->setPowerPlan("8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c");
    }
}

void BigPictureTV::loadSettings()
{
    QDir settingsDir(QFileInfo(settingsFile).absolutePath());
    if (!settingsDir.exists()) {
        settingsDir.mkpath(settingsDir.absolutePath());
    }

    QFile file(settingsFile);
    if (!file.exists()) {
        firstRun = true;

    } else {
        if (file.open(QIODevice::ReadOnly)) {
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                settings = doc.object();
            }
            file.close();
        }
    }
}

void BigPictureTV::showSettings()
{
    if (configurator) {
        configurator->showNormal();
        configurator->raise();
        configurator->activateWindow();
        return;
    }

    windowCheckTimer->stop();
    configurator = new Configurator;
    configurator->setAttribute(Qt::WA_DeleteOnClose);
    connect(configurator, &Configurator::closed, this, &BigPictureTV::onConfiguratorClosed);
    configurator->show();
}

void BigPictureTV::onConfiguratorClosed()
{
    loadSettings();
    windowCheckTimer->setInterval(settings.value("checkrate").toInt());
    windowCheckTimer->start();
}
