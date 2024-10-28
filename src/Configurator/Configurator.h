#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QMainWindow>
#include <QString>
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
    void onStartupCheckboxStateChanged(Qt::CheckState state);
    void onDisableAudioCheckboxStateChanged(Qt::CheckState state);
    void onDisableMonitorCheckboxStateChanged(Qt::CheckState state);
    void onTargetWindowComboBoxIndexChanged(int index);
    void onAudioButtonClicked();
    void setGeneralTab();
    void setAVTab();
    void setActionsTab();
    void setAdvancedTab();

private:
    void initDiscordAction();
    bool discordInstalled;
    void getAudioCapabilities();
    void populateComboboxes();
    void toggleAudioSettings(bool state);
    void toggleMonitorSettings(bool state);
    void toggleCustomWindowTitle(bool state);
    void setupConnections();
    void createDefaultSettings();
    void loadSettings();
    void saveSettings();
    void applySettings();
    void initUI();
    void fadeIn(QWidget *widget);
    void fadeOut(QWidget *widget, std::function<void()> onFinished);

    Ui::Configurator *ui;
    QSettings settings;
    int activeFrame;

signals:
    void closed();
};

#endif // CONFIGURATOR_H
