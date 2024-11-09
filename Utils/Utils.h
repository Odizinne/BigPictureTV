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
    void sendMediaKey(WORD keyCode);
    void skipBigPictureIntro();
    bool isWindows10();
}

#endif // UTILS_H
