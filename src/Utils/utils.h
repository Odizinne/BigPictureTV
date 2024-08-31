#ifndef UTILS_H
#define UTILS_H

#include <QIcon>
#include <QString>
#include <windows.h>

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

private:
    QString getTheme();
    QString getDiscordPath();

};

#endif // UTILS_H
