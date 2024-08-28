#ifndef BIGPICTURETV_H
#define BIGPICTURETV_H

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMainWindow>
#include <QMenu>
#include <QString>
#include <QSystemTrayIcon>
#include <QTimer>
#include <string>
#include "NightLightSwitcher.h"

namespace Ui {
class BigPictureTV;
}

class BigPictureTV : public QMainWindow
{
    Q_OBJECT

public:
    explicit BigPictureTV(QWidget *parent = nullptr);
    ~BigPictureTV();
    void createDefaultSettings();
    void loadSettings();
    void saveSettings();
    void applySettings();

private slots:
    void onStartupCheckboxStateChanged();
    void onDisableAudioCheckboxStateChanged(int state);
    void onDisableMonitorCheckboxStateChanged(int state);
    void onTargetWindowComboBoxIndexChanged(int index);
    void onAudioButtonClicked();
    void checkWindowTitle();
    void onCheckrateSpinBoxValueChanged();
    void showSettings();

private:
    QSystemTrayIcon *trayIcon;
    void createTrayIcon();
    void initDiscordAction();
    QMenu *createMenu();
    bool gamemodeActive;
    NightLightSwitcher* nightLightSwitcher;
    bool discordInstalled;
    bool firstRun;
    QString activePowerPlan;
    bool nightLightState;
    bool discordState;
    void getAudioCapabilities();
    void setCustomFusion();
    void populateComboboxes();
    void toggleAudioSettings(bool state);
    void toggleMonitorSettings(bool state);
    void toggleCustomWindowTitle(bool state);
    void setupConnections();
    void handleMonitorChanges(bool isDesktopMode, bool disableVideo);
    void handleAudioChanges(bool isDesktopMode, bool disableAudio);
    void handleActions(bool isDesktopMode);
    void handleDiscordAction(bool isDesktopMode);
    void handleNightLightAction(bool isDesktopMode);
    void handlePowerPlanAction(bool isDesktopMode);
    void setupInfoTab();

    Ui::BigPictureTV *ui;
    QTimer *windowCheckTimer;
    std::wstring shortcutName = L"BigPictureTV.lnk";
    QString settingsFilePath;
    QJsonObject settings;
    static const QString settingsFile;
    QString status;
    QString message;
};

#endif // BIGPICTURETV_H
