#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMainWindow>
#include <QString>
#include <string>
#include "shortcutmanager.h"
#include "utils.h"
#include "steamwindowmanager.h"

namespace Ui {
class Configurator;
}

class Configurator : public QMainWindow
{
    Q_OBJECT

public:
    explicit Configurator(QWidget *parent = nullptr);
    ~Configurator();


private slots:
    void onStartupCheckboxStateChanged();
    void onDisableAudioCheckboxStateChanged(int state);
    void onDisableMonitorCheckboxStateChanged(int state);
    void onTargetWindowComboBoxIndexChanged(int index);
    void onAudioButtonClicked();

private:
    void initDiscordAction();
    Utils* utils;
    ShortcutManager* shortcutManager;
    SteamWindowManager* steamWindowManager;
    bool discordInstalled;
    void getAudioCapabilities();
    void populateComboboxes();
    void toggleAudioSettings(bool state);
    void toggleMonitorSettings(bool state);
    void toggleCustomWindowTitle(bool state);
    void setupConnections();
    void setupInfoTab();
    void createDefaultSettings();
    void loadSettings();
    void saveSettings();
    void applySettings();

    Ui::Configurator *ui;
    QString settingsFilePath;
    QJsonObject settings;
    static const QString settingsFile;

signals:
    void closed();
};

#endif // CONFIGURATOR_H
