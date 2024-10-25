#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QMainWindow>
#include <QString>
#include "shortcutmanager.h"
#include "utils.h"
#include "steamwindowmanager.h"
#include <QSettings>

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
    void toggleAllActions();
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
    QSettings settings;

signals:
    void closed();
};

#endif // CONFIGURATOR_H
