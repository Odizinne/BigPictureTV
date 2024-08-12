#include "shortcutmanager.h"
#include <filesystem>
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>

const std::wstring SHORTCUT_NAME = L"BigPictureTV.lnk";

void setPaths(std::wstring& targetPath, std::wstring& startupFolder)
{
    wchar_t executablePath[MAX_PATH];
    GetModuleFileName(NULL, executablePath, MAX_PATH);
    targetPath = std::wstring(executablePath);

    startupFolder = getStartupFolder();
}

std::wstring getStartupFolder()
{
    wchar_t path[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, path) == S_OK)
    {
        return std::wstring(path);
    }
    return L"";
}

std::wstring getShortcutPath()
{
    return getStartupFolder() + L"\\" + SHORTCUT_NAME;
}

void createShortcut(const std::wstring& targetPath)
{
    std::wstring shortcutPath = getShortcutPath();
    std::wstring workingDirectory = targetPath.substr(0, targetPath.find_last_of(L"\\"));

    HRESULT hResult;
    IShellLink* pShellLink = NULL;
    IPersistFile* pPersistFile = NULL;

    CoInitialize(NULL);
    hResult = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pShellLink);
    if (SUCCEEDED(hResult))
    {
        pShellLink->SetPath(targetPath.c_str());
        pShellLink->SetWorkingDirectory(workingDirectory.c_str());
        pShellLink->SetDescription(L"Launch BigPictureTV");

        hResult = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);
        if (SUCCEEDED(hResult))
        {
            hResult = pPersistFile->Save(shortcutPath.c_str(), TRUE);
            pPersistFile->Release();
        }
        pShellLink->Release();
    }

    CoUninitialize();
}

void removeShortcut()
{
    std::wstring shortcutPath = getShortcutPath();
    if (isShortcutPresent())
    {
        _wremove(shortcutPath.c_str());
    }
}

bool isShortcutPresent()
{
    std::wstring shortcutPath = getShortcutPath();
    return std::filesystem::exists(shortcutPath);
}

void manageShortcut(bool state)
{
    std::wstring targetPath;
    std::wstring startupFolder;

    setPaths(targetPath, startupFolder);

    if (state)
    {
        if (!isShortcutPresent())
        {
            createShortcut(targetPath);
        }
    }
    else
    {
        if (isShortcutPresent())
        {
            removeShortcut();
        }
    }
}
