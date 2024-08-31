#ifndef STEAMWINDOWMANAGER_H
#define STEAMWINDOWMANAGER_H

#include <QMap>
#include <QStringList>
#include <QVector>
#include <windows.h>

class SteamWindowManager {
public:
    SteamWindowManager();
    ~SteamWindowManager();
    bool isBigPictureRunning() const;
    bool isCustomWindowRunning(const QString &windowTitle) const;
    QString getSteamLanguage() const;
    QString getBigPictureWindowTitle() const;

private:
    QString getRegistryValue(const std::wstring &keyPath, const std::wstring &valueName) const;
    QString cleanString(const QString &str) const;
    QVector<QString> getAllWindowTitles() const;
    static const QMap<QString, QString> BIG_PICTURE_WINDOW_TITLES;
    static const QChar NON_BREAKING_SPACE;
};

#endif // STEAMWINDOWMANAGER_H
