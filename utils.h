#ifndef UTILS_H
#define UTILS_H
#include <QString>

void runDisplayswitch(const QString &command);
bool switchPowerPlan(const std::wstring& planGuid);
bool isDiscordInstalled();
void closeDiscord();
void startDiscord();
std::string getTheme();

#endif // UTILS_H
