#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include <QIcon>

void runDisplayswitch(const QString &command);
std::string getTheme();
QIcon getIconForTheme();
void switchPowerPlan(const std::wstring& planGuid);
bool isDiscordInstalled();
void closeDiscord();
void startDiscord();
bool isAudioDeviceCmdletsInstalled();
bool isWindows10();

#endif // UTILS_H
