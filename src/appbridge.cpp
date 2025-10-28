#include "appbridge.h"
#include "appconfiguration.h"
#include "windoweventmonitor.h"
#include "audiomanager.h"
#include "nightlightswitcher.h"
#include "steamwindowmanager.h"
#include "displaymanager.h"
#include "utils.h"
#include <QApplication>
#include <QVariant>

AppBridge* AppBridge::s_instance = nullptr;

AppBridge* AppBridge::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)

    if (!s_instance) {
        s_instance = new AppBridge();
    }
    return s_instance;
}

AppBridge* AppBridge::instance()
{
    return s_instance;
}

AppBridge::AppBridge(QObject *parent)
    : QObject(parent)
    , m_hdrCapability(0)
    , m_discordInstalled(false)
    , activePowerPlan("")
    , nightLightState(false)
    , discordState(false)
    , windowMonitor(new WindowEventMonitor(this))
    , m_currentlyInGamemode(false)
{
    updateAudioDevices();
    checkHDRCapability();
    checkDiscordInstalled();

    // Connect to window events
    connect(windowMonitor, &WindowEventMonitor::windowActivated,
            this, &AppBridge::onWindowActivated);
    connect(windowMonitor, &WindowEventMonitor::windowDestroyed,
            this, &AppBridge::onWindowDestroyed);

    windowMonitor->start();

    startupReset();
}

AppBridge::~AppBridge()
{
    delete windowMonitor;
}

QString AppBridge::getDeviceIdFromName(const QString& name)
{
    for (const Device &device : devices) {
        if (device.name == name) {
            return device.ID;
        }
    }
    return "";
}

void AppBridge::onWindowActivated(QString windowTitle)
{
    AppConfiguration* config = AppConfiguration::instance();

    if (config->targetWindowMode() == 2 && config->customWindowTitle().isEmpty()) {
        return;
    }

    bool isTargetWindow = false;

    if (config->targetWindowMode() == 0) {
        // Check for Big Picture
        isTargetWindow = SteamWindowManager::isBigPictureWindowTitle(windowTitle);
    } else if (config->targetWindowMode() == 1) {
        // Check for Playnite
        isTargetWindow = SteamWindowManager::isCustomWindowTitle(windowTitle, "Playnite");
    } else {
        // Check for custom window
        isTargetWindow = SteamWindowManager::isCustomWindowTitle(windowTitle, config->customWindowTitle());
    }

    if (isTargetWindow && !m_currentlyInGamemode) {
        // Target window activated - enter game mode and track it
        m_currentlyInGamemode = true;
        config->setGamemode(true);
        windowMonitor->trackWindow(windowTitle);
        handleActions(false);
        handleMonitorChanges(false);
        checkAndSetHDR(false);
        if (config->skipIntro()) {
            Utils::skipBigPictureIntro();
        }
        handleAudioChanges(false);
    }
    // Don't exit game mode when switching to a different window
    // Game mode should persist as long as the target window exists
}

void AppBridge::onWindowDestroyed()
{
    // When a window is destroyed, check if we need to exit gamemode
    AppConfiguration* config = AppConfiguration::instance();

    if (m_currentlyInGamemode) {
        m_currentlyInGamemode = false;
        config->setGamemode(false);
        handleActions(true);
        checkAndSetHDR(true);
        handleMonitorChanges(true);
        handleAudioChanges(true);
    }
}

void AppBridge::handleMonitorChanges(bool isDesktopMode)
{
    AppConfiguration* config = AppConfiguration::instance();

    if (config->disableMonitorSwitch())
        return;

    DisplayManager* displayManager = DisplayManager::instance();
    if (!displayManager)
        return;

    if (isDesktopMode) {
        // Restore original configuration
        displayManager->restoreOriginalConfiguration();
    } else {
        // Save current configuration before switching
        displayManager->saveCurrentConfiguration();

        // Switch to gamemode display with custom resolution
        QString displayDevice = config->gamemodeDisplayDevice();
        if (!displayDevice.isEmpty()) {
            quint32 width = config->gamemodeDisplayWidth();
            quint32 height = config->gamemodeDisplayHeight();
            quint32 refreshRate = config->gamemodeDisplayRefreshRate();

            if (width > 0 && height > 0 && refreshRate > 0) {
                displayManager->switchToDisplayWithResolution(displayDevice, width, height, refreshRate);
            } else {
                // Fallback to default switching if resolution not set
                displayManager->switchToDisplay(displayDevice);
            }
        }
    }
}

void AppBridge::checkAndSetHDR(bool isDesktopMode)
{
    AppConfiguration* config = AppConfiguration::instance();

    if (!config->enableHdr()) {
        return;
    }

    bool enable = !isDesktopMode;
    if (Utils::getHDRStatus() != 2) {
        Utils::setHDR(enable);
    }
}

void AppBridge::handleAudioChanges(bool isDesktopMode)
{
    AppConfiguration* config = AppConfiguration::instance();

    if (config->disableAudioSwitch()) {
        return;
    }

    QString audioDevice = isDesktopMode ? config->desktopAudioDeviceId() : config->gamemodeAudioDeviceId();
    AudioManager::setAudioDevice(audioDevice);
}

void AppBridge::handleActions(bool isDesktopMode)
{
    AppConfiguration* config = AppConfiguration::instance();

    if (config->closeDiscordAction()) {
        handleDiscordAction(isDesktopMode);
    }
    if (config->disableNightlightAction()) {
        handleNightLightAction(isDesktopMode);
    }
    if (config->performancePowerplanAction()) {
        handlePowerPlanAction(isDesktopMode);
    }
    if (config->pauseMediaAction()) {
        handleMediaAction(isDesktopMode);
    }
}

void AppBridge::handleDiscordAction(bool isDesktopMode)
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

void AppBridge::handleNightLightAction(bool isDesktopMode)
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

void AppBridge::handleMediaAction(bool isDesktopMode)
{
    if (!isDesktopMode) {
        Utils::sendMediaKey(VK_MEDIA_STOP);
    }
}

void AppBridge::handlePowerPlanAction(bool isDesktopMode)
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

void AppBridge::startupReset()
{
    AppConfiguration* config = AppConfiguration::instance();

    if (config->gamemode()) {
        bool isRunning;
        if (config->targetWindowMode() == 0) {
            isRunning = SteamWindowManager::isBigPictureRunning();
        } else {
            isRunning = SteamWindowManager::isCustomWindowRunning(config->customWindowTitle());
        }
        if (!isRunning) {
            handleMonitorChanges(true);
            handleAudioChanges(true);
        }
    }
}

void AppBridge::updateAudioDevices()
{
    devices = AudioManager::ListAudioOutputDevices();
    m_audioDevices.clear();

    for (const Device &device : devices) {
        QVariantMap deviceMap;
        deviceMap["name"] = device.name;
        deviceMap["ID"] = device.ID;
        m_audioDevices.append(deviceMap);
    }

    emit audioDevicesChanged();
}

void AppBridge::checkHDRCapability()
{
    m_hdrCapability = Utils::getHDRStatus();
    emit hdrCapabilityChanged();
}

void AppBridge::checkDiscordInstalled()
{
    m_discordInstalled = Utils::isDiscordInstalled();
    emit discordInstalledChanged();
}
