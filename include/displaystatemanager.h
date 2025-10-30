#ifndef DISPLAYSTATEMANAGER_H
#define DISPLAYSTATEMANAGER_H

#include <QString>
#include <vector>
#include <windows.h>

// Serializable topology data
struct SerializedDisplayState {
    std::vector<DISPLAYCONFIG_PATH_INFO> paths;
    std::vector<DISPLAYCONFIG_MODE_INFO> modes;
    QString timestamp;
    quint32 pathCount;
    quint32 modeCount;
};

class DisplayStateManager
{
public:
    DisplayStateManager();

    // Save display topology to file
    bool saveDisplayState(const std::vector<DISPLAYCONFIG_PATH_INFO> &paths,
                         const std::vector<DISPLAYCONFIG_MODE_INFO> &modes);

    // Load display topology from file
    bool loadDisplayState(std::vector<DISPLAYCONFIG_PATH_INFO> &paths,
                         std::vector<DISPLAYCONFIG_MODE_INFO> &modes);

    // Check if a saved state exists
    bool hasSavedState() const;

    // Clear saved state file
    bool clearSavedState();

    // Get the path to the state file
    static QString getStateFilePath();

private:
    // Validate that displays still connected match the saved state
    bool validateSavedState(const SerializedDisplayState &state);

    // Helper to convert DISPLAYCONFIG structures to JSON-serializable format
    static QString toJsonString(const std::vector<DISPLAYCONFIG_PATH_INFO> &paths,
                               const std::vector<DISPLAYCONFIG_MODE_INFO> &modes);

    // Helper to reconstruct DISPLAYCONFIG structures from JSON
    static bool fromJsonString(const QString &json,
                              std::vector<DISPLAYCONFIG_PATH_INFO> &paths,
                              std::vector<DISPLAYCONFIG_MODE_INFO> &modes);
};

#endif // DISPLAYSTATEMANAGER_H
