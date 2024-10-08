#include "shortcutmanager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QShortcut>
#include <QStandardPaths>
#include <QSysInfo>
#include <QtWidgets/QWidget>
#include <shlobj.h>
#include <shobjidl.h>
#include <windows.h>

ShortcutManager::ShortcutManager() {}

ShortcutManager::~ShortcutManager() {}

QString ShortcutManager::getStartupFolder()
{
    QString path;
    WCHAR szPath[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, szPath) == S_OK) {
        path = QString::fromWCharArray(szPath);
    }
    return path;
}

void ShortcutManager::setPaths(QString &targetPath, QString &startupFolder)
{
    TCHAR executablePath[MAX_PATH];
    GetModuleFileName(NULL, executablePath, MAX_PATH);
    targetPath = QString::fromWCharArray(executablePath);

    startupFolder = getStartupFolder();
}

QString ShortcutManager::getShortcutPath()
{
    static QString shortcutName = "BigPictureTV.lnk";
    return getStartupFolder() + "\\" + shortcutName;
}

void ShortcutManager::createShortcut(const QString &targetPath)
{
    QString shortcutPath = getShortcutPath();
    QString workingDirectory = QFileInfo(targetPath).path();

    IShellLink *pShellLink = nullptr;
    IPersistFile *pPersistFile = nullptr;

    if (FAILED(CoInitialize(nullptr))) {
        qDebug() << "Failed to initialize COM library.";
        return;
    }

    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink,
                                   nullptr,
                                   CLSCTX_INPROC_SERVER,
                                   IID_IShellLink,
                                   (void **) &pShellLink))) {
        pShellLink->SetPath(targetPath.toStdWString().c_str());
        pShellLink->SetWorkingDirectory(workingDirectory.toStdWString().c_str());
        pShellLink->SetDescription(L"Launch BigPictureTV");

        if (SUCCEEDED(pShellLink->QueryInterface(IID_IPersistFile, (void **) &pPersistFile))) {
            pPersistFile->Save(shortcutPath.toStdWString().c_str(), TRUE);
            pPersistFile->Release();
        }

        pShellLink->Release();
    } else {
        qDebug() << "Failed to create ShellLink instance.";
    }

    CoUninitialize();
}

bool ShortcutManager::isShortcutPresent()
{
    QString shortcutPath = getShortcutPath();
    return QFile::exists(shortcutPath);
}

void ShortcutManager::removeShortcut()
{
    QString shortcutPath = getShortcutPath();
    if (isShortcutPresent()) {
        QFile::remove(shortcutPath);
    }
}

void ShortcutManager::manageShortcut(bool state)
{
    QString targetPath;
    QString startupFolder;

    setPaths(targetPath, startupFolder);

    if (state) {
        if (!isShortcutPresent()) {
            createShortcut(targetPath);
        }
    } else {
        if (isShortcutPresent()) {
            removeShortcut();
        }
    }
}
