#ifndef UTILS_H
#define UTILS_H

#include <QIcon>
#include <QString>
#include <windows.h>
#include <QFrame>

namespace Utils {
    void runEnhancedDisplayswitch(const QString &command);
    QIcon getIconForTheme();
    QString getActivePowerPlan();
    void setPowerPlan(QString planGuid);
    bool isDiscordInstalled();
    bool isDiscordRunning();
    void closeDiscord();
    void startDiscord();
    bool isAudioDeviceCmdletsInstalled();
    void sendMediaKey(WORD keyCode);
    void setFrameColorBasedOnWindow(QWidget *window, QFrame *frame);
}

#endif // UTILS_H
