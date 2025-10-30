#include "displaystatemanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <cstring>

DisplayStateManager::DisplayStateManager()
{
}

QString DisplayStateManager::getStateFilePath()
{
    // Use AppData/Local like other app data
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir dir(appDataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return appDataPath + "/display_topology_state.json";
}

bool DisplayStateManager::saveDisplayState(const std::vector<DISPLAYCONFIG_PATH_INFO> &paths,
                                           const std::vector<DISPLAYCONFIG_MODE_INFO> &modes)
{
    if (paths.empty() || modes.empty()) {
        qWarning() << "Cannot save empty display state";
        return false;
    }

    QString filePath = getStateFilePath();
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open state file for writing:" << filePath;
        return false;
    }

    try {
        // Create root JSON object
        QJsonObject root;
        root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        root["pathCount"] = static_cast<int>(paths.size());
        root["modeCount"] = static_cast<int>(modes.size());

        // Serialize paths
        QJsonArray pathsArray;
        for (const auto &path : paths) {
            QJsonObject pathObj;
            pathObj["flags"] = static_cast<int>(path.flags);
            pathObj["sourceId"] = static_cast<int>(path.sourceInfo.id);
            pathObj["sourceModeIdx"] = static_cast<int>(path.sourceInfo.modeInfoIdx);
            pathObj["targetAdapterId"] = static_cast<int>(path.targetInfo.adapterId.LowPart);
            pathObj["targetId"] = static_cast<int>(path.targetInfo.id);
            pathObj["targetModeIdx"] = static_cast<int>(path.targetInfo.modeInfoIdx);

            pathsArray.append(pathObj);
        }
        root["paths"] = pathsArray;

        // Serialize modes
        QJsonArray modesArray;
        for (const auto &mode : modes) {
            QJsonObject modeObj;
            modeObj["infoType"] = static_cast<int>(mode.infoType);

            if (mode.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE) {
                modeObj["sourceWidth"] = static_cast<int>(mode.sourceMode.width);
                modeObj["sourceHeight"] = static_cast<int>(mode.sourceMode.height);
            } else if (mode.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_TARGET) {
                const auto &targetMode = mode.targetMode;
                modeObj["targetWidth"] = static_cast<int>(targetMode.targetVideoSignalInfo.activeSize.cx);
                modeObj["targetHeight"] = static_cast<int>(targetMode.targetVideoSignalInfo.activeSize.cy);
                modeObj["targetRefreshRateNum"] = static_cast<int>(targetMode.targetVideoSignalInfo.vSyncFreq.Numerator);
                modeObj["targetRefreshRateDen"] = static_cast<int>(targetMode.targetVideoSignalInfo.vSyncFreq.Denominator);
            }

            modesArray.append(modeObj);
        }
        root["modes"] = modesArray;

        QJsonDocument doc(root);
        file.write(doc.toJson(QJsonDocument::Compact));
        file.close();

        qDebug() << "Successfully saved display state to:" << filePath;
        return true;

    } catch (const std::exception &e) {
        qWarning() << "Exception while saving display state:" << e.what();
        file.close();
        return false;
    }
}

bool DisplayStateManager::loadDisplayState(std::vector<DISPLAYCONFIG_PATH_INFO> &paths,
                                           std::vector<DISPLAYCONFIG_MODE_INFO> &modes)
{
    QString filePath = getStateFilePath();
    QFile file(filePath);

    if (!file.exists()) {
        qWarning() << "Display state file does not exist:" << filePath;
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open state file for reading:" << filePath;
        return false;
    }

    try {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if (!doc.isObject()) {
            qWarning() << "Invalid JSON in display state file";
            return false;
        }

        QJsonObject root = doc.object();

        quint32 pathCount = root["pathCount"].toInt();
        quint32 modeCount = root["modeCount"].toInt();

        if (pathCount == 0 || modeCount == 0) {
            qWarning() << "Invalid path or mode count in saved state";
            return false;
        }

        paths.clear();
        modes.clear();

        // Deserialize paths
        QJsonArray pathsArray = root["paths"].toArray();
        if (static_cast<size_t>(pathsArray.size()) != pathCount) {
            qWarning() << "Path array size mismatch";
            return false;
        }

        for (int i = 0; i < pathsArray.size(); ++i) {
            QJsonObject pathObj = pathsArray[i].toObject();

            DISPLAYCONFIG_PATH_INFO path = {};
            path.flags = static_cast<UINT32>(pathObj["flags"].toInt());
            path.sourceInfo.id = static_cast<UINT32>(pathObj["sourceId"].toInt());
            path.sourceInfo.modeInfoIdx = static_cast<UINT32>(pathObj["sourceModeIdx"].toInt());
            path.targetInfo.adapterId.LowPart = static_cast<UINT32>(pathObj["targetAdapterId"].toInt());
            path.targetInfo.adapterId.HighPart = 0;
            path.targetInfo.id = static_cast<UINT32>(pathObj["targetId"].toInt());
            path.targetInfo.modeInfoIdx = static_cast<UINT32>(pathObj["targetModeIdx"].toInt());

            paths.push_back(path);
        }

        // Deserialize modes
        QJsonArray modesArray = root["modes"].toArray();
        if (static_cast<size_t>(modesArray.size()) != modeCount) {
            qWarning() << "Mode array size mismatch";
            return false;
        }

        for (int i = 0; i < modesArray.size(); ++i) {
            QJsonObject modeObj = modesArray[i].toObject();

            DISPLAYCONFIG_MODE_INFO mode = {};
            mode.infoType = static_cast<DISPLAYCONFIG_MODE_INFO_TYPE>(modeObj["infoType"].toInt());

            if (mode.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE) {
                mode.sourceMode.width = static_cast<UINT32>(modeObj["sourceWidth"].toInt());
                mode.sourceMode.height = static_cast<UINT32>(modeObj["sourceHeight"].toInt());
            } else if (mode.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_TARGET) {
                auto &targetMode = mode.targetMode;
                targetMode.targetVideoSignalInfo.activeSize.cx = static_cast<UINT32>(modeObj["targetWidth"].toInt());
                targetMode.targetVideoSignalInfo.activeSize.cy = static_cast<UINT32>(modeObj["targetHeight"].toInt());
                targetMode.targetVideoSignalInfo.totalSize.cx = static_cast<UINT32>(modeObj["targetWidth"].toInt());
                targetMode.targetVideoSignalInfo.totalSize.cy = static_cast<UINT32>(modeObj["targetHeight"].toInt());
                targetMode.targetVideoSignalInfo.vSyncFreq.Numerator = static_cast<UINT32>(modeObj["targetRefreshRateNum"].toInt());
                targetMode.targetVideoSignalInfo.vSyncFreq.Denominator = static_cast<UINT32>(modeObj["targetRefreshRateDen"].toInt());
            }

            modes.push_back(mode);
        }

        qDebug() << "Successfully loaded display state from:" << filePath;
        qDebug() << "Loaded paths:" << paths.size() << "modes:" << modes.size();
        return true;

    } catch (const std::exception &e) {
        qWarning() << "Exception while loading display state:" << e.what();
        file.close();
        return false;
    }
}

bool DisplayStateManager::hasSavedState() const
{
    QString filePath = getStateFilePath();
    return QFile::exists(filePath);
}

bool DisplayStateManager::clearSavedState()
{
    QString filePath = getStateFilePath();
    if (!QFile::exists(filePath)) {
        return true; // Already cleared
    }

    if (!QFile::remove(filePath)) {
        qWarning() << "Failed to remove state file:" << filePath;
        return false;
    }

    qDebug() << "Cleared display state file:" << filePath;
    return true;
}

bool DisplayStateManager::validateSavedState(const SerializedDisplayState &state)
{
    // Basic validation: check if paths and modes are not empty
    return !state.paths.empty() && !state.modes.empty();
}
