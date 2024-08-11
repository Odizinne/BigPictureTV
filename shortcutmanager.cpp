#include "shortcutmanager.h"
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h> // For IShellLink and IPersistFile
#include <fstream>    // For file operations

void setPaths(const std::wstring& shortcutName,
              std::wstring& targetPath, std::wstring& startupFolder, std::wstring& shortcutPath)
{
    wchar_t executablePath[MAX_PATH];
    GetModuleFileName(NULL, executablePath, MAX_PATH);
    targetPath = std::wstring(executablePath);

    startupFolder = getStartupFolder();
    shortcutPath = startupFolder + L"\\" + shortcutName;
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

void createShortcut(const std::wstring& targetPath, const std::wstring& shortcutPath)
{
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

void removeShortcut(const std::wstring& shortcutPath)
{
    if (isShortcutPresent(shortcutPath))
    {
        _wremove(shortcutPath.c_str());
    }
}

bool isShortcutPresent(const std::wstring& shortcutPath)
{
    std::ifstream file(shortcutPath.c_str());
    return file.good();
}

void manageShortcut(const std::wstring& shortcutName, bool state)
{
    std::wstring targetPath;
    std::wstring startupFolder;
    std::wstring shortcutPath;

    setPaths(shortcutName, targetPath, startupFolder, shortcutPath);

    if (state)
    {
        if (!isShortcutPresent(shortcutPath))
        {
            createShortcut(targetPath, shortcutPath);
        }
    }
    else
    {
        if (isShortcutPresent(shortcutPath))
        {
            removeShortcut(shortcutPath);
        }
    }
}
