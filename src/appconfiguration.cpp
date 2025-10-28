#include "appconfiguration.h"
#include "shortcutmanager.h"

AppConfiguration* AppConfiguration::s_instance = nullptr;

AppConfiguration* AppConfiguration::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)

    if (!s_instance) {
        s_instance = new AppConfiguration();
    }
    return s_instance;
}

AppConfiguration* AppConfiguration::instance()
{
    return s_instance;
}

AppConfiguration::AppConfiguration(QObject *parent)
    : QObject(parent)
    , m_settings("Odizinne", "BigPictureTV")
{
    loadSettings();
}

AppConfiguration::~AppConfiguration()
{
}

void AppConfiguration::loadSettings()
{
    m_targetWindowMode = m_settings.value("target_window_mode", 0).toInt();
    m_customWindowTitle = m_settings.value("custom_window_title", "").toString();
    m_skipIntro = m_settings.value("skip_intro", false).toBool();
    m_launchAtStartup = ShortcutManager::isShortcutPresent("BigPictureTV");

    m_disableAudioSwitch = m_settings.value("disable_audio_switch", false).toBool();
    m_gamemodeAudioDevice = m_settings.value("gamemode_audio_device", "").toString();
    m_desktopAudioDevice = m_settings.value("desktop_audio_device", "").toString();
    m_gamemodeAudioDeviceId = m_settings.value("gamemode_audio_device_id", "").toString();
    m_desktopAudioDeviceId = m_settings.value("desktop_audio_device_id", "").toString();

    m_disableMonitorSwitch = m_settings.value("disable_monitor_switch", false).toBool();
    m_gamemodeDisplayDevice = m_settings.value("gamemode_display_device", "").toString();
    m_gamemodeDisplayWidth = m_settings.value("gamemode_display_width", 3840).toUInt();
    m_gamemodeDisplayHeight = m_settings.value("gamemode_display_height", 2160).toUInt();
    m_gamemodeDisplayRefreshRate = m_settings.value("gamemode_display_refresh_rate", 60).toUInt();

    m_closeDiscordAction = m_settings.value("close_discord_action", false).toBool();
    m_performancePowerplanAction = m_settings.value("performance_powerplan_action", false).toBool();
    m_pauseMediaAction = m_settings.value("pause_media_action", false).toBool();
    m_disableNightlightAction = m_settings.value("disable_nightlight_action", false).toBool();
    m_enableHdr = m_settings.value("enable_hdr", false).toBool();

    m_gamemode = m_settings.value("gamemode", false).toBool();
    m_firstRun = m_settings.value("first_run", true).toBool();
}

void AppConfiguration::saveSettings()
{
    m_settings.setValue("target_window_mode", m_targetWindowMode);
    m_settings.setValue("custom_window_title", m_customWindowTitle);
    m_settings.setValue("skip_intro", m_skipIntro);

    m_settings.setValue("disable_audio_switch", m_disableAudioSwitch);
    m_settings.setValue("gamemode_audio_device", m_gamemodeAudioDevice);
    m_settings.setValue("desktop_audio_device", m_desktopAudioDevice);
    m_settings.setValue("gamemode_audio_device_id", m_gamemodeAudioDeviceId);
    m_settings.setValue("desktop_audio_device_id", m_desktopAudioDeviceId);

    m_settings.setValue("disable_monitor_switch", m_disableMonitorSwitch);
    m_settings.setValue("gamemode_display_device", m_gamemodeDisplayDevice);
    m_settings.setValue("gamemode_display_width", m_gamemodeDisplayWidth);
    m_settings.setValue("gamemode_display_height", m_gamemodeDisplayHeight);
    m_settings.setValue("gamemode_display_refresh_rate", m_gamemodeDisplayRefreshRate);

    m_settings.setValue("close_discord_action", m_closeDiscordAction);
    m_settings.setValue("performance_powerplan_action", m_performancePowerplanAction);
    m_settings.setValue("pause_media_action", m_pauseMediaAction);
    m_settings.setValue("disable_nightlight_action", m_disableNightlightAction);
    m_settings.setValue("enable_hdr", m_enableHdr);

    m_settings.setValue("gamemode", m_gamemode);
    m_settings.setValue("first_run", m_firstRun);
}

void AppConfiguration::setTargetWindowMode(int value)
{
    if (m_targetWindowMode != value) {
        m_targetWindowMode = value;
        saveSettings();
        emit targetWindowModeChanged();
    }
}

void AppConfiguration::setCustomWindowTitle(const QString &value)
{
    if (m_customWindowTitle != value) {
        m_customWindowTitle = value;
        saveSettings();
        emit customWindowTitleChanged();
    }
}

void AppConfiguration::setSkipIntro(bool value)
{
    if (m_skipIntro != value) {
        m_skipIntro = value;
        saveSettings();
        emit skipIntroChanged();
    }
}

void AppConfiguration::setLaunchAtStartup(bool value)
{
    if (m_launchAtStartup != value) {
        m_launchAtStartup = value;
        ShortcutManager::manageShortcut(value, "BigPictureTV");
        emit launchAtStartupChanged();
    }
}

void AppConfiguration::setDisableAudioSwitch(bool value)
{
    if (m_disableAudioSwitch != value) {
        m_disableAudioSwitch = value;
        saveSettings();
        emit disableAudioSwitchChanged();
    }
}

void AppConfiguration::setGamemodeAudioDevice(const QString &value)
{
    if (m_gamemodeAudioDevice != value) {
        m_gamemodeAudioDevice = value;
        saveSettings();
        emit gamemodeAudioDeviceChanged();
    }
}

void AppConfiguration::setDesktopAudioDevice(const QString &value)
{
    if (m_desktopAudioDevice != value) {
        m_desktopAudioDevice = value;
        saveSettings();
        emit desktopAudioDeviceChanged();
    }
}

void AppConfiguration::setGamemodeAudioDeviceId(const QString &value)
{
    if (m_gamemodeAudioDeviceId != value) {
        m_gamemodeAudioDeviceId = value;
        saveSettings();
        emit gamemodeAudioDeviceIdChanged();
    }
}

void AppConfiguration::setDesktopAudioDeviceId(const QString &value)
{
    if (m_desktopAudioDeviceId != value) {
        m_desktopAudioDeviceId = value;
        saveSettings();
        emit desktopAudioDeviceIdChanged();
    }
}

void AppConfiguration::setDisableMonitorSwitch(bool value)
{
    if (m_disableMonitorSwitch != value) {
        m_disableMonitorSwitch = value;
        saveSettings();
        emit disableMonitorSwitchChanged();
    }
}

void AppConfiguration::setGamemodeDisplayDevice(const QString &value)
{
    if (m_gamemodeDisplayDevice != value) {
        m_gamemodeDisplayDevice = value;
        saveSettings();
        emit gamemodeDisplayDeviceChanged();
    }
}

void AppConfiguration::setGamemodeDisplayWidth(quint32 value)
{
    if (m_gamemodeDisplayWidth != value) {
        m_gamemodeDisplayWidth = value;
        saveSettings();
        emit gamemodeDisplayWidthChanged();
    }
}

void AppConfiguration::setGamemodeDisplayHeight(quint32 value)
{
    if (m_gamemodeDisplayHeight != value) {
        m_gamemodeDisplayHeight = value;
        saveSettings();
        emit gamemodeDisplayHeightChanged();
    }
}

void AppConfiguration::setGamemodeDisplayRefreshRate(quint32 value)
{
    if (m_gamemodeDisplayRefreshRate != value) {
        m_gamemodeDisplayRefreshRate = value;
        saveSettings();
        emit gamemodeDisplayRefreshRateChanged();
    }
}

void AppConfiguration::setCloseDiscordAction(bool value)
{
    if (m_closeDiscordAction != value) {
        m_closeDiscordAction = value;
        saveSettings();
        emit closeDiscordActionChanged();
    }
}

void AppConfiguration::setPerformancePowerplanAction(bool value)
{
    if (m_performancePowerplanAction != value) {
        m_performancePowerplanAction = value;
        saveSettings();
        emit performancePowerplanActionChanged();
    }
}

void AppConfiguration::setPauseMediaAction(bool value)
{
    if (m_pauseMediaAction != value) {
        m_pauseMediaAction = value;
        saveSettings();
        emit pauseMediaActionChanged();
    }
}

void AppConfiguration::setDisableNightlightAction(bool value)
{
    if (m_disableNightlightAction != value) {
        m_disableNightlightAction = value;
        saveSettings();
        emit disableNightlightActionChanged();
    }
}

void AppConfiguration::setEnableHdr(bool value)
{
    if (m_enableHdr != value) {
        m_enableHdr = value;
        saveSettings();
        emit enableHdrChanged();
    }
}

void AppConfiguration::setGamemode(bool value)
{
    if (m_gamemode != value) {
        m_gamemode = value;
        saveSettings();
        emit gamemodeChanged();
    }
}

void AppConfiguration::setFirstRun(bool value)
{
    if (m_firstRun != value) {
        m_firstRun = value;
        saveSettings();
        emit firstRunChanged();
    }
}

void AppConfiguration::resetToDefaults()
{
    setTargetWindowMode(0);
    setCustomWindowTitle("");
    setSkipIntro(false);
    setLaunchAtStartup(false);

    setDisableAudioSwitch(false);
    setGamemodeAudioDevice("");
    setDesktopAudioDevice("");
    setGamemodeAudioDeviceId("");
    setDesktopAudioDeviceId("");

    setDisableMonitorSwitch(false);
    setGamemodeDisplayDevice("");
    setGamemodeDisplayWidth(3840);
    setGamemodeDisplayHeight(2160);
    setGamemodeDisplayRefreshRate(60);

    setCloseDiscordAction(false);
    setPerformancePowerplanAction(false);
    setPauseMediaAction(false);
    setDisableNightlightAction(false);
    setEnableHdr(false);
}
