#include "displaymanager.h"
#include <QDebug>
#include <map>

DisplayManager* DisplayManager::s_instance = nullptr;

DisplayManager* DisplayManager::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)

    if (!s_instance) {
        s_instance = new DisplayManager();
    }
    return s_instance;
}

DisplayManager* DisplayManager::instance()
{
    return s_instance;
}

DisplayManager::DisplayManager(QObject *parent)
    : QObject(parent)
    , m_configSaved(false)
{
    updateDisplays();
}

DisplayManager::~DisplayManager()
{
}

std::vector<DisplayInfo> DisplayManager::enumerateDisplays()
{
    std::vector<DisplayInfo> displays;
    std::map<std::wstring, DisplayInfo> uniqueDisplays;

    UINT32 pathCount = 0, modeCount = 0;

    if (GetDisplayConfigBufferSizes(QDC_ALL_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS) {
        qWarning() << "Failed to get display config buffer sizes";
        return displays;
    }

    std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

    if (QueryDisplayConfig(QDC_ALL_PATHS, &pathCount, paths.data(), &modeCount, modes.data(), nullptr) != ERROR_SUCCESS) {
        qWarning() << "Failed to query display config";
        return displays;
    }

    for (UINT32 i = 0; i < pathCount; i++) {
        DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
        targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        targetName.header.size = sizeof(targetName);
        targetName.header.adapterId = paths[i].targetInfo.adapterId;
        targetName.header.id = paths[i].targetInfo.id;

        if (DisplayConfigGetDeviceInfo(&targetName.header) != ERROR_SUCCESS) {
            continue;
        }

        if (wcslen(targetName.monitorDevicePath) == 0) {
            continue;
        }

        std::wstring devicePath = targetName.monitorDevicePath;

        if (uniqueDisplays.find(devicePath) == uniqueDisplays.end()) {
            DisplayInfo info;
            info.device_path = devicePath;
            info.adapter_id = paths[i].targetInfo.adapterId;
            info.target_id = paths[i].targetInfo.id;
            info.source_id = paths[i].sourceInfo.id;
            info.friendly_name = targetName.monitorFriendlyDeviceName;
            info.is_active = (paths[i].flags & DISPLAYCONFIG_PATH_ACTIVE) != 0;

            if (info.friendly_name.empty()) {
                info.friendly_name = L"Generic Monitor";
            }

            uniqueDisplays[devicePath] = info;
        }
    }

    for (auto &pair : uniqueDisplays) {
        displays.push_back(pair.second);
    }

    return displays;
}

void DisplayManager::updateDisplays()
{
    QVariantList result;
    auto displays = enumerateDisplays();

    for (const auto &display : displays) {
        QVariantMap displayMap;
        displayMap["name"] = QString::fromStdWString(display.friendly_name);
        displayMap["devicePath"] = QString::fromStdWString(display.device_path);
        displayMap["isActive"] = display.is_active;
        result.append(displayMap);
    }

    if (m_displays != result) {
        m_displays = result;
        emit displaysChanged();
    }
}

void DisplayManager::refreshDisplays()
{
    updateDisplays();
}

SavedConfig DisplayManager::saveTopology()
{
    SavedConfig config;
    UINT32 pathCount = 0, modeCount = 0;

    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS) {
        qWarning() << "Failed to get display config buffer sizes for saving";
        return config;
    }

    config.paths.resize(pathCount);
    config.modes.resize(modeCount);

    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, config.paths.data(),
                          &modeCount, config.modes.data(), nullptr) != ERROR_SUCCESS) {
        qWarning() << "Failed to query display config for saving";
        config.paths.clear();
        config.modes.clear();
    }

    return config;
}

bool DisplayManager::saveCurrentConfiguration()
{
    m_savedConfig = saveTopology();
    m_configSaved = !m_savedConfig.paths.empty();

    if (!m_configSaved) {
        qWarning() << "Failed to save current display configuration";
    }

    return m_configSaved;
}

bool DisplayManager::setOnlyDisplay(const DisplayInfo &target)
{
    UINT32 pathCount = 0, modeCount = 0;

    if (GetDisplayConfigBufferSizes(QDC_ALL_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS) {
        qWarning() << "Failed to get display config buffer sizes for setting display";
        return false;
    }

    std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

    if (QueryDisplayConfig(QDC_ALL_PATHS, &pathCount, paths.data(), &modeCount, modes.data(), nullptr) != ERROR_SUCCESS) {
        qWarning() << "Failed to query display config for setting display";
        return false;
    }

    std::vector<DISPLAYCONFIG_PATH_INFO> newPaths;
    std::vector<DISPLAYCONFIG_MODE_INFO> newModes;

    // Find the matching path by comparing device path
    for (UINT32 i = 0; i < pathCount; i++) {
        DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
        targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        targetName.header.size = sizeof(targetName);
        targetName.header.adapterId = paths[i].targetInfo.adapterId;
        targetName.header.id = paths[i].targetInfo.id;

        if (DisplayConfigGetDeviceInfo(&targetName.header) != ERROR_SUCCESS) {
            continue;
        }

        std::wstring devicePath = targetName.monitorDevicePath;

        if (devicePath == target.device_path) {
            // This is our target display - keep it active
            auto path = paths[i];
            path.flags = DISPLAYCONFIG_PATH_ACTIVE;

            // Copy modes
            UINT32 sourceModeIdx = DISPLAYCONFIG_PATH_MODE_IDX_INVALID;
            UINT32 targetModeIdx = DISPLAYCONFIG_PATH_MODE_IDX_INVALID;

            if (path.sourceInfo.modeInfoIdx != DISPLAYCONFIG_PATH_MODE_IDX_INVALID &&
                path.sourceInfo.modeInfoIdx < modeCount) {
                newModes.push_back(modes[path.sourceInfo.modeInfoIdx]);
                sourceModeIdx = (UINT32)newModes.size() - 1;
            }

            if (path.targetInfo.modeInfoIdx != DISPLAYCONFIG_PATH_MODE_IDX_INVALID &&
                path.targetInfo.modeInfoIdx < modeCount) {
                newModes.push_back(modes[path.targetInfo.modeInfoIdx]);
                targetModeIdx = (UINT32)newModes.size() - 1;
            }

            path.sourceInfo.modeInfoIdx = sourceModeIdx;
            path.targetInfo.modeInfoIdx = targetModeIdx;

            newPaths.push_back(path);
            break; // Found our display
        }
    }

    if (newPaths.empty()) {
        qWarning() << "Failed to find target display path";
        return false;
    }

    LONG result = SetDisplayConfig((UINT32)newPaths.size(), newPaths.data(),
                                   (UINT32)newModes.size(), newModes.data(),
                                   SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG |
                                       SDC_ALLOW_CHANGES | SDC_SAVE_TO_DATABASE);

    if (result != ERROR_SUCCESS) {
        qWarning() << "SetDisplayConfig failed with error:" << result;
        return false;
    }

    return true;
}

bool DisplayManager::switchToDisplay(const QString &devicePath)
{
    auto displays = enumerateDisplays();
    std::wstring targetPath = devicePath.toStdWString();

    for (const auto &display : displays) {
        if (display.device_path == targetPath) {
            return setOnlyDisplay(display);
        }
    }

    qWarning() << "Display not found:" << devicePath;
    return false;
}

bool DisplayManager::restoreTopology(const SavedConfig &config)
{
    if (config.paths.empty()) {
        qWarning() << "Cannot restore empty configuration";
        return false;
    }

    auto paths = config.paths;
    auto modes = config.modes;

    LONG result = SetDisplayConfig((UINT32)paths.size(), paths.data(),
                                   (UINT32)modes.size(), modes.data(),
                                   SDC_APPLY | SDC_USE_SUPPLIED_DISPLAY_CONFIG |
                                       SDC_ALLOW_CHANGES);

    if (result != ERROR_SUCCESS) {
        qWarning() << "RestoreTopology failed with error:" << result;
        return false;
    }

    return true;
}

bool DisplayManager::restoreOriginalConfiguration()
{
    if (!m_configSaved) {
        qWarning() << "No saved configuration to restore";
        return false;
    }

    return restoreTopology(m_savedConfig);
}
