#ifndef UTILS_H
#define UTILS_H
#include <QIcon>
#include <QString>

void runDisplayswitch(const QString &command);
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

#endif // UTILS_H
