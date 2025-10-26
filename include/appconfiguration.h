#ifndef APPCONFIGURATION_H
#define APPCONFIGURATION_H

#include <QObject>
#include <QQmlEngine>
#include <QSettings>

class AppConfiguration : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Window detection settings
    Q_PROPERTY(int targetWindowMode READ targetWindowMode WRITE setTargetWindowMode NOTIFY targetWindowModeChanged)
    Q_PROPERTY(QString customWindowTitle READ customWindowTitle WRITE setCustomWindowTitle NOTIFY customWindowTitleChanged)
    Q_PROPERTY(bool skipIntro READ skipIntro WRITE setSkipIntro NOTIFY skipIntroChanged)
    Q_PROPERTY(bool launchAtStartup READ launchAtStartup WRITE setLaunchAtStartup NOTIFY launchAtStartupChanged)

    // Audio settings
    Q_PROPERTY(bool disableAudioSwitch READ disableAudioSwitch WRITE setDisableAudioSwitch NOTIFY disableAudioSwitchChanged)
    Q_PROPERTY(QString gamemodeAudioDevice READ gamemodeAudioDevice WRITE setGamemodeAudioDevice NOTIFY gamemodeAudioDeviceChanged)
    Q_PROPERTY(QString desktopAudioDevice READ desktopAudioDevice WRITE setDesktopAudioDevice NOTIFY desktopAudioDeviceChanged)
    Q_PROPERTY(QString gamemodeAudioDeviceId READ gamemodeAudioDeviceId WRITE setGamemodeAudioDeviceId NOTIFY gamemodeAudioDeviceIdChanged)
    Q_PROPERTY(QString desktopAudioDeviceId READ desktopAudioDeviceId WRITE setDesktopAudioDeviceId NOTIFY desktopAudioDeviceIdChanged)

    // Monitor settings
    Q_PROPERTY(bool disableMonitorSwitch READ disableMonitorSwitch WRITE setDisableMonitorSwitch NOTIFY disableMonitorSwitchChanged)
    Q_PROPERTY(int gamemodeMonitorMode READ gamemodeMonitorMode WRITE setGamemodeMonitorMode NOTIFY gamemodeMonitorModeChanged)
    Q_PROPERTY(int desktopMonitorMode READ desktopMonitorMode WRITE setDesktopMonitorMode NOTIFY desktopMonitorModeChanged)

    // Action settings
    Q_PROPERTY(bool closeDiscordAction READ closeDiscordAction WRITE setCloseDiscordAction NOTIFY closeDiscordActionChanged)
    Q_PROPERTY(bool performancePowerplanAction READ performancePowerplanAction WRITE setPerformancePowerplanAction NOTIFY performancePowerplanActionChanged)
    Q_PROPERTY(bool pauseMediaAction READ pauseMediaAction WRITE setPauseMediaAction NOTIFY pauseMediaActionChanged)
    Q_PROPERTY(bool disableNightlightAction READ disableNightlightAction WRITE setDisableNightlightAction NOTIFY disableNightlightActionChanged)
    Q_PROPERTY(bool enableHdr READ enableHdr WRITE setEnableHdr NOTIFY enableHdrChanged)

    // Internal state
    Q_PROPERTY(bool gamemode READ gamemode WRITE setGamemode NOTIFY gamemodeChanged)

public:
    static AppConfiguration* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static AppConfiguration* instance();

    // Getters
    int targetWindowMode() const { return m_targetWindowMode; }
    QString customWindowTitle() const { return m_customWindowTitle; }
    bool skipIntro() const { return m_skipIntro; }
    bool launchAtStartup() const { return m_launchAtStartup; }

    bool disableAudioSwitch() const { return m_disableAudioSwitch; }
    QString gamemodeAudioDevice() const { return m_gamemodeAudioDevice; }
    QString desktopAudioDevice() const { return m_desktopAudioDevice; }
    QString gamemodeAudioDeviceId() const { return m_gamemodeAudioDeviceId; }
    QString desktopAudioDeviceId() const { return m_desktopAudioDeviceId; }

    bool disableMonitorSwitch() const { return m_disableMonitorSwitch; }
    int gamemodeMonitorMode() const { return m_gamemodeMonitorMode; }
    int desktopMonitorMode() const { return m_desktopMonitorMode; }

    bool closeDiscordAction() const { return m_closeDiscordAction; }
    bool performancePowerplanAction() const { return m_performancePowerplanAction; }
    bool pauseMediaAction() const { return m_pauseMediaAction; }
    bool disableNightlightAction() const { return m_disableNightlightAction; }
    bool enableHdr() const { return m_enableHdr; }

    bool gamemode() const { return m_gamemode; }

    // Setters
    void setTargetWindowMode(int value);
    void setCustomWindowTitle(const QString &value);
    void setSkipIntro(bool value);
    void setLaunchAtStartup(bool value);

    void setDisableAudioSwitch(bool value);
    void setGamemodeAudioDevice(const QString &value);
    void setDesktopAudioDevice(const QString &value);
    void setGamemodeAudioDeviceId(const QString &value);
    void setDesktopAudioDeviceId(const QString &value);

    void setDisableMonitorSwitch(bool value);
    void setGamemodeMonitorMode(int value);
    void setDesktopMonitorMode(int value);

    void setCloseDiscordAction(bool value);
    void setPerformancePowerplanAction(bool value);
    void setPauseMediaAction(bool value);
    void setDisableNightlightAction(bool value);
    void setEnableHdr(bool value);

    void setGamemode(bool value);

    Q_INVOKABLE void resetToDefaults();

signals:
    void targetWindowModeChanged();
    void customWindowTitleChanged();
    void skipIntroChanged();
    void launchAtStartupChanged();

    void disableAudioSwitchChanged();
    void gamemodeAudioDeviceChanged();
    void desktopAudioDeviceChanged();
    void gamemodeAudioDeviceIdChanged();
    void desktopAudioDeviceIdChanged();

    void disableMonitorSwitchChanged();
    void gamemodeMonitorModeChanged();
    void desktopMonitorModeChanged();

    void closeDiscordActionChanged();
    void performancePowerplanActionChanged();
    void pauseMediaActionChanged();
    void disableNightlightActionChanged();
    void enableHdrChanged();

    void gamemodeChanged();

private:
    explicit AppConfiguration(QObject *parent = nullptr);
    ~AppConfiguration();

    static AppConfiguration* s_instance;

    void loadSettings();
    void saveSettings();

    QSettings m_settings;

    // Member variables
    int m_targetWindowMode;
    QString m_customWindowTitle;
    bool m_skipIntro;
    bool m_launchAtStartup;

    bool m_disableAudioSwitch;
    QString m_gamemodeAudioDevice;
    QString m_desktopAudioDevice;
    QString m_gamemodeAudioDeviceId;
    QString m_desktopAudioDeviceId;

    bool m_disableMonitorSwitch;
    int m_gamemodeMonitorMode;
    int m_desktopMonitorMode;

    bool m_closeDiscordAction;
    bool m_performancePowerplanAction;
    bool m_pauseMediaAction;
    bool m_disableNightlightAction;
    bool m_enableHdr;

    bool m_gamemode;
};

#endif // APPCONFIGURATION_H
