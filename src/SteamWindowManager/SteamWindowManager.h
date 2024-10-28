#ifndef STEAMWINDOWMANAGER_H
#define STEAMWINDOWMANAGER_H

#include <QMap>
#include <QStringList>
#include <QVector>

namespace SteamWindowManager
{
    QString getSteamLanguage();
    QString getBigPictureWindowTitle();
    bool isBigPictureRunning();
    bool isCustomWindowRunning(const QString &windowTitle);
}

#endif // STEAMWINDOWMANAGER_H
