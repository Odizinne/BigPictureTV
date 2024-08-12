#include "shortcutmanager.h"
#include <filesystem>
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h> // For IShellLink and IPersistFile
#include <fstream>    // For file operations

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

    // Initialize COM library
    CoInitialize(NULL);

    // Create a shell link object
    hResult = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pShellLink);
    if (SUCCEEDED(hResult))
    {
        // Set the path to the shortcut target
        pShellLink->SetPath(targetPath.c_str());
        pShellLink->SetWorkingDirectory(workingDirectory.c_str());
        pShellLink->SetDescription(L"Launch BigPictureTV");

        // Query IPersistFile interface
        hResult = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);
        if (SUCCEEDED(hResult))
        {
            // Save the shortcut to disk
            hResult = pPersistFile->Save(shortcutPath.c_str(), TRUE);
            pPersistFile->Release();
        }
        pShellLink->Release();
    }

    // Uninitialize COM library
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
