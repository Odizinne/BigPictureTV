#ifndef STEAMWINDOWMANAGER_H
#define STEAMWINDOWMANAGER_H

#include <QString>

QString getBigPictureWindowTitle();
QString getSteamLanguage();
bool isBigPictureRunning();
bool isCustomWindowRunning(QString windowTitle);

#endif // STEAMWINDOWMANAGER_H
