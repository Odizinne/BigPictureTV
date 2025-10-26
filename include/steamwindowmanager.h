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

    // New event-based methods
    bool isBigPictureWindowTitle(const QString &windowTitle);
    bool isCustomWindowTitle(const QString &windowTitle, const QString &customTitle);
}

#endif // STEAMWINDOWMANAGER_H
