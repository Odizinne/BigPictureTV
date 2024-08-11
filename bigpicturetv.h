#ifndef BIGPICTURETV_H
#define BIGPICTURETV_H

#include <QMainWindow>
#include <QTimer>
#include <QCheckBox>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <string>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QString>
#include <QWidget>

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
    void checkWindowTitle();
    void onCheckrateSliderReleased();
    void showSettings();

private:
    QSystemTrayIcon *trayIcon;

    void createTrayIcon();
    QMenu* createMenu();
    void toggleDetection();
    //void showSettings();
    bool gamemodeActive;
    void setFrames();
    void populateComboboxes();
    void runDisplaySwitchCommand(const QString &command);
    void toggleAudioSettings(bool state);
    void toggleMonitorSettings(bool state);
    void setupConnections();
    void handleMonitorChanges();
    void handleAudioChanges();
    Ui::BigPictureTV *ui;
    QTimer *windowCheckTimer;
    std::wstring shortcutName = L"BigPictureTV.lnk"; // Default values or set them appropriately
    QString settingsFilePath;
    QJsonObject settings;
    static const QString settingsFile; // Use QString for Qt compatibility

protected:
    void closeEvent(QCloseEvent *event) override;
};


#endif // BIGPICTURETV_H
