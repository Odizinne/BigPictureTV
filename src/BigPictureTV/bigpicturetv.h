#ifndef BIGPICTURETV_H
#define BIGPICTURETV_H

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QJsonObject>
#include "utils.h"
#include "steamwindowmanager.h"
#include "audiomanager.h"
#include "NightLightSwitcher.h"
#include "configurator.h"

class BigPictureTV : public QObject
{
    Q_OBJECT

public:
    explicit BigPictureTV(QObject *parent = nullptr);
    ~BigPictureTV();

private slots:
    void onConfiguratorClosed();

private:
    Utils* utils;
    SteamWindowManager* steamWindowManager;
    AudioManager* audioManager;
    NightLightSwitcher* nightLightSwitcher;
    Configurator* configurator;

    QString activePowerPlan;
    bool nightLightState;
    bool discordState;

    QSystemTrayIcon *trayIcon;
    QTimer *windowCheckTimer;
    QMenu *trayIconMenu;
    QAction *quitAction;
    QAction *configAction;
    void loadSettings();
    void createTrayIcon();
    void handleMediaAction(bool isDesktopMode);
    void handlePowerPlanAction(bool isDesktopMode);
    void handleNightLightAction(bool isDesktopMode);
    void handleDiscordAction(bool isDesktopMode);
    void handleActions(bool isDesktopMode);
    void handleAudioChanges(bool isDesktopMode, bool disableAudio);
    void handleMonitorChanges(bool isDesktopMode, bool disableVideo);
    void checkWindowTitle();
    void showSettings();

    bool gamemodeActive;
    QJsonObject settings;
    static const QString settingsFile;

};

#endif // BIGPICTURETV_H
