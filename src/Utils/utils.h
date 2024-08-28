#ifndef UTILS_H
#define UTILS_H
#include <QIcon>
#include <QString>
#include <windows.h>

void runEnhancedDisplayswitch(const QString &command);
QString getTheme();
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

#endif // UTILS_H
