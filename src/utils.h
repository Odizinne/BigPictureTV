#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include <QIcon>

void runDisplayswitch(const QString &command);
bool switchPowerPlan(const std::wstring& planGuid);
bool isDiscordInstalled();
bool isAudioDeviceCmdletsInstalled();
void closeDiscord();
void startDiscord();
std::string getTheme();
QIcon getIconForTheme();

#endif // UTILS_H
