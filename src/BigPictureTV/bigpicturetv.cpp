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
    , configurator(nullptr)
    , activePowerPlan("")
    , nightLightState(false)
    , discordState(false)
    , windowCheckTimer(new QTimer(this))
{
    loadSettings();
    windowCheckTimer->setInterval(window_checkrate);
    connect(windowCheckTimer, &QTimer::timeout, this, &BigPictureTV::checkWindowTitle);
    windowCheckTimer->start();
    createTrayIcon();
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
    if (target_window_mode == 1 && custom_window_title == "") {
        return;
    }

    if (utils->isSunshineStreaming()) {
        return;
    }

    bool disableVideo = disable_monitor_switch;
    bool disableAudio = disable_audio_switch;
    bool isRunning;
    if (target_window_mode == 0) {
        isRunning = steamWindowManager->isBigPictureRunning();
    } else if (target_window_mode == 1) {
        isRunning = steamWindowManager->isCustomWindowRunning(custom_window_title);
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

    int index = isDesktopMode ? desktop_monitor_mode
                              : gamemode_monitor_mode;

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

    QString audioDevice = isDesktopMode ? desktop_audio_device
                                            : gamemode_audio_device;

    audioManager->setAudioDevice(audioDevice.toStdString());

    try {
        audioManager->setAudioDevice(audioDevice.toStdString());
    } catch (const std::runtime_error &e) {
        qDebug() << "Error: " << e.what();
    }
}

void BigPictureTV::handleActions(bool isDesktopMode)
{
    if (close_discord_action) {
        handleDiscordAction(isDesktopMode);
    }
    if (disable_nightlight_action) {
        handleNightLightAction(isDesktopMode);
    }
    if (performance_powerplan_action) {
        handlePowerPlanAction(isDesktopMode);
    }
    if (pause_media_action) {
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
        showSettings();

    } else {
        if (file.open(QIODevice::ReadOnly)) {
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                settings = doc.object();

                gamemode_audio_device = settings.value("gamemode_audio_device").toString();
                desktop_audio_device = settings.value("desktop_audio_device").toString();
                disable_audio_switch = settings.value("disable_audio_switch").toBool();
                window_checkrate = settings.value("window_checkrate").toInt(1000);
                close_discord_action = settings.value("close_discord_action").toBool(false);
                performance_powerplan_action = settings.value("performance_powerplan_action").toBool(false);
                pause_media_action = settings.value("pause_media_action").toBool(false);
                gamemode_monitor_mode = settings.value("gamemode_monitor_mode").toInt(0);
                desktop_monitor_mode = settings.value("desktop_monitor_mode").toInt(0);
                disable_monitor_switch = settings.value("disable_monitor_switch").toBool();
                disable_nightlight_action = settings.value("disable_nightlight_action").toBool();
                target_window_mode = settings.value("target_window_mode").toInt(0);
                custom_window_title = settings.value("custom_window_title").toString();
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
    configurator = nullptr;
    loadSettings();
    windowCheckTimer->setInterval(window_checkrate);
    windowCheckTimer->start();
}
