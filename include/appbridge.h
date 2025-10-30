#ifndef APPBRIDGE_H
#define APPBRIDGE_H

#include <QObject>
#include <QQmlEngine>
#include "audiomanager.h"
#include "displaymanager.h"
#include "displaystatemanager.h"

class WindowEventMonitor;
class AudioDeviceNotifier;

class AppBridge : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Properties for QML binding
    Q_PROPERTY(QVariantList audioDevices READ audioDevices NOTIFY audioDevicesChanged)
    Q_PROPERTY(int hdrCapability READ hdrCapability NOTIFY hdrCapabilityChanged)
    Q_PROPERTY(bool discordInstalled READ discordInstalled NOTIFY discordInstalledChanged)

public:
    static AppBridge* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static AppBridge* instance();

    // Property getters
    QVariantList audioDevices() const { return m_audioDevices; }
    int hdrCapability() const { return m_hdrCapability; }
    bool discordInstalled() const { return m_discordInstalled; }

    // Invokable methods for QML
    Q_INVOKABLE QString getDeviceIdFromName(const QString& name);

signals:
    void audioDevicesChanged();
    void hdrCapabilityChanged();
    void discordInstalledChanged();

private slots:
    void onWindowActivated(QString windowTitle);
    void onWindowDestroyed();
    void onNewAudioDeviceDetected(QString deviceId, QString deviceName);

private:
    explicit AppBridge(QObject *parent = nullptr);
    ~AppBridge();

    static AppBridge* s_instance;

    void startupReset();
    void handleMediaAction(bool isDesktopMode);
    void handlePowerPlanAction(bool isDesktopMode);
    void handleNightLightAction(bool isDesktopMode);
    void handleDiscordAction(bool isDesktopMode);
    void handleActions(bool isDesktopMode);
    void handleAudioChanges(bool isDesktopMode);
    void handleMonitorChanges(bool isDesktopMode);
    void checkAndSetHDR(bool isDesktopMode);
    void updateAudioDevices();
    void checkHDRCapability();
    void checkDiscordInstalled();

    QVariantList m_audioDevices;
    int m_hdrCapability;
    bool m_discordInstalled;

    QString activePowerPlan;
    bool nightLightState;
    bool discordState;

    WindowEventMonitor *windowMonitor;
    AudioDeviceNotifier *audioDeviceNotifier;

    QList<Device> devices;
    bool m_currentlyInGamemode;
    QStringList m_audioDeviceIdsBeforeMonitorSwitch; // Track audio device IDs before display switch
    DisplayStateManager m_displayStateManager;
};

#endif // APPBRIDGE_H
