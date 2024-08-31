#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <QString>

class ShortcutManager {
public:
    ShortcutManager();
    ~ShortcutManager();

    bool isShortcutPresent();
    void manageShortcut(bool state);

private:
    void setPaths(QString &targetPath, QString &startupFolder);
    QString getShortcutPath();
    void createShortcut(const QString &targetPath);
    void removeShortcut();
    QString getStartupFolder();
};



#endif // SHORTCUTMANAGER_H
