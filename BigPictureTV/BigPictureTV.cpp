#include "BigPictureTV.h"
#include "AudioManager.h"
#include "NightLightSwitcher.h"
#include "SteamwindowManager.h"
#include "Utils.h"
#include <QApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <QJsonParseError>
#include <QTimer>
#include <QDir>
#include <QFileInfo>

BigPictureTV::BigPictureTV(QObject *parent)
    : QObject(parent)
    , configurator(nullptr)
    , activePowerPlan("")
    , nightLightState(false)
    , discordState(false)
    , windowCheckTimer(new QTimer(this))
    , settings("Odizinne", "BigPictureTV")
    , gamemodeActive(settings.value("gamemode", false).toBool())
{
    loadSettings();
    startupReset();
    windowCheckTimer->setInterval(window_checkrate);
    connect(windowCheckTimer, &QTimer::timeout, this, &BigPictureTV::checkWindowTitle);
    windowCheckTimer->start();
    createTrayIcon();
}

BigPictureTV::~BigPictureTV()
{
    delete windowCheckTimer;
    delete trayIcon;
    delete trayIconMenu;
    delete configAction;
    delete quitAction;
    delete configurator;
}

void BigPictureTV::createTrayIcon()
{
    trayIcon = new QSystemTrayIcon(Utils::getIconForTheme(), this);
    trayIconMenu = new QMenu();

    pauseAction = new QAction(tr("Pause detection"), this);
    pauseAction->setCheckable(true);
    connect(pauseAction, &QAction::triggered, this, &BigPictureTV::changeDetectionState);
    trayIconMenu->addAction(pauseAction);

    trayIconMenu->addSeparator();

    configAction = new QAction(tr("Settings"), this);
    connect(configAction, &QAction::triggered, this, &BigPictureTV::showSettings);
    trayIconMenu->addAction(configAction);

    quitAction = new QAction(tr("Quit"), this);
    connect(quitAction, &QAction::triggered, this, &QApplication::quit);
    trayIconMenu->addAction(quitAction);

    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setToolTip("BigPictureTV");
    trayIcon->show();
}

void BigPictureTV::checkWindowTitle()
{
    if (target_window_mode == 2 && custom_window_title == "") {
        return;
    }

    bool isRunning;
    if (target_window_mode == 0) {
        isRunning = SteamWindowManager::isBigPictureRunning();
    } else if (target_window_mode == 1){
        isRunning = SteamWindowManager::isCustomWindowRunning("Playnite");
    } else {
        isRunning = SteamWindowManager::isCustomWindowRunning(custom_window_title);
    }

    if (isRunning && !gamemodeActive) {
        gamemodeActive = true;
        handleActions(false);
        handleMonitorChanges(false, disable_monitor_switch);
        if (skip_intro) {
            Utils::skipBigPictureIntro();
        }
        handleAudioChanges(false, disable_audio_switch);
        settings.setValue("gamemode", gamemodeActive);
    } else if (!isRunning && gamemodeActive) {
        gamemodeActive = false;
        handleActions(true);
        handleMonitorChanges(true, disable_monitor_switch);
        handleAudioChanges(true, disable_audio_switch);
        settings.setValue("gamemode", gamemodeActive);
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
        Utils::runDisplayswitch(command);
    }
}

void BigPictureTV::handleAudioChanges(bool isDesktopMode, bool disableAudio)
{
    if (disableAudio) {
        return;
    }

    QString audioDevice = isDesktopMode ? desktop_audio_device
                                            : gamemode_audio_device;

    AudioManager::setAudioDevice(audioDevice);
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
            Utils::startDiscord();
        }
    } else {
        discordState = Utils::isDiscordRunning();
        Utils::closeDiscord();
    }
}

void BigPictureTV::handleNightLightAction(bool isDesktopMode)
{
    if (isDesktopMode) {
        if (nightLightState) {
            NightLightSwitcher::enable();
        }
    } else {
        nightLightState = NightLightSwitcher::enabled();
        NightLightSwitcher::disable();
    }
}

void BigPictureTV::handleMediaAction(bool isDesktopMode)
{
    if (!isDesktopMode) {
        Utils::sendMediaKey(VK_MEDIA_STOP);
    }
}

void BigPictureTV::handlePowerPlanAction(bool isDesktopMode)
{
    if (isDesktopMode) {
        if (!activePowerPlan.isEmpty()) {
            Utils::setPowerPlan(activePowerPlan);
        } else {
            Utils::setPowerPlan("381b4222-f694-41f0-9685-ff5bb260df2e");
        }
    } else {
        activePowerPlan = Utils::getActivePowerPlan();
        Utils::setPowerPlan("8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c");
    }
}

void BigPictureTV::loadSettings()
{
    gamemode_audio_device = settings.value("gamemode_audio_device_id").toString();
    desktop_audio_device = settings.value("desktop_audio_device_id").toString();
    disable_audio_switch = settings.value("disable_audio_switch").toBool();
    window_checkrate = settings.value("window_checkrate").toInt();
    close_discord_action = settings.value("close_discord_action").toBool();
    performance_powerplan_action = settings.value("performance_powerplan_action").toBool();
    pause_media_action = settings.value("pause_media_action").toBool();
    gamemode_monitor_mode = settings.value("gamemode_monitor_mode").toInt();
    desktop_monitor_mode = settings.value("desktop_monitor_mode").toInt();
    disable_monitor_switch = settings.value("disable_monitor_switch").toBool();
    disable_nightlight_action = settings.value("disable_nightlight_action").toBool();
    target_window_mode = settings.value("target_window_mode").toInt();
    custom_window_title = settings.value("custom_window_title").toString();
    skip_intro = settings.value("skip_intro").toBool();
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

    if (!pauseAction->isChecked()) {
        windowCheckTimer->start();
    }
}

void BigPictureTV::startupReset()
{
    if (gamemodeActive) {
        bool isRunning;
        if (target_window_mode == 0) {
            isRunning = SteamWindowManager::isBigPictureRunning();
        } else {
            isRunning = SteamWindowManager::isCustomWindowRunning(custom_window_title);
        }
        if (!isRunning) {
            handleMonitorChanges(true, disable_monitor_switch);
            handleAudioChanges(true, disable_audio_switch);
        }
    }
}

void BigPictureTV::changeDetectionState()
{
    if (!configurator) {
        pauseAction->isChecked() ? windowCheckTimer->stop() : windowCheckTimer->start();
    }
}
