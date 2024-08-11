#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <string>
#include <vector>

// Structure to store device information
struct Device {
    int index;
    std::string name;
    std::string type;
};

// Function to execute a command and return its output as a string
std::string executeCommand(const std::string& command);

// Function to parse the command output and extract playback devices
std::vector<Device> parseDevices(const std::string& output);

// Function to check if a device name matches any of the stored names
bool checkDevice(const std::string& deviceName);

void setAudioDevice(const std::string& deviceName);

#endif // AUDIOMANAGER_H
