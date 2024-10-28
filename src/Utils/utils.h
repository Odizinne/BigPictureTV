#ifndef UTILS_H
#define UTILS_H

#include <QIcon>
#include <QString>
#include <windows.h>
#include <QFrame>

class Utils {
public:
    Utils();
    ~Utils();

    void runEnhancedDisplayswitch(const QString &command);
    QIcon getIconForTheme();
    QString getActivePowerPlan();
    void setPowerPlan(QString planGuid);
    bool isDiscordInstalled();
    bool isDiscordRunning();
    void closeDiscord();
    void startDiscord();
    bool isAudioDeviceCmdletsInstalled();
    bool isSunshineStreaming();
    void sendMediaKey(WORD keyCode);
    void setFrameColorBasedOnWindow(QWidget *window, QFrame *frame);

private:
    QString getTheme();
    QString getDiscordPath();
    QColor adjustColor(const QColor &color, double factor);
    bool isDarkMode(const QColor &color);
};

#endif // UTILS_H
