#include "AudioManager.h"
#include <cstdio>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <iostream>

std::string executeCommand(const std::string& command) {
    std::string result;
    char buffer[128];
    // Prepare the PowerShell command
    std::string fullCommand = "powershell.exe -Command \"" + command + "\"";

    FILE* pipe = _popen(fullCommand.c_str(), "r"); // Use _popen for compatibility with Windows
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    try {
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
    } catch (...) {
        _pclose(pipe);
        throw;
    }
    _pclose(pipe);
    return result;
}

std::vector<Device> parseDevices(const std::string& output) {
    std::vector<Device> devices;
    std::istringstream stream(output);
    std::string line;
    Device currentDevice;

    while (std::getline(stream, line)) {
        if (line.find("Index") != std::string::npos) {
            // Push the previous device to the list if it's valid
            if (currentDevice.index >= 1 && currentDevice.type == "Playback") {
                devices.push_back(currentDevice);
            }
            // Initialize a new device
            currentDevice = Device{-1, "", ""};
            std::istringstream indexStream(line.substr(line.find(":") + 2));
            indexStream >> currentDevice.index;
        } else if (line.find("Name") != std::string::npos) {
            currentDevice.name = line.substr(line.find(":") + 2);
        } else if (line.find("Type") != std::string::npos) {
            currentDevice.type = line.substr(line.find(":") + 2);
        }
    }
    // Add the last device if it's valid
    if (currentDevice.index >= 1 && currentDevice.type == "Playback") {
        devices.push_back(currentDevice);
    }

    return devices;
}

bool containsIgnoreCase(const std::string& str, const std::string& substr) {
    std::string strLower = str;
    std::string substrLower = substr;
    std::transform(strLower.begin(), strLower.end(), strLower.begin(), ::tolower);
    std::transform(substrLower.begin(), substrLower.end(), substrLower.begin(), ::tolower);
    return strLower.find(substrLower) != std::string::npos;
}

bool checkDevice(const std::string& deviceName) {
    // Use the actual PowerShell cmdlet command you want to run
    std::string command = "Get-AudioDevice -l";
    std::string output = executeCommand(command);
    std::vector<Device> devices = parseDevices(output);
    for (const auto& device : devices) {
        if (containsIgnoreCase(device.name, deviceName)) {
            return true;
        }
    }
    return false;
}

// Function to set the default audio device
void setAudioDevice(const std::string& deviceName) {
    // Retrieve the list of audio devices
    std::string command = "Get-AudioDevice -l";
    std::string output = executeCommand(command);
    std::vector<Device> devices = parseDevices(output);

    // Find the device with the given name
    bool deviceFound = false;
    int maxRetries = 5;
    int retryCount = 0;
    std::string result;

    for (const auto& device : devices) {
        if (containsIgnoreCase(device.name, deviceName)) {
            // Ensure the index is valid (greater than or equal to 1)
            if (device.index < 1) {
                throw std::runtime_error("Invalid device index: " + std::to_string(device.index));
            }

            // Retry logic
            while (retryCount < maxRetries) {
                // Construct the PowerShell command to set the default audio device
                std::string setCommand = "Set-AudioDevice -Index " + std::to_string(device.index);
                result = executeCommand(setCommand);

                // Check if the command executed successfully
                if (result.find("Error") == std::string::npos) {
                    deviceFound = true;
                    break;
                }

                // Log or print the error message (for debugging purposes)
                std::cerr << "Failed to set the audio device. Retrying... (" << retryCount + 1 << "/" << maxRetries << ")" << std::endl;

                // Increment retry count and wait before the next attempt
                ++retryCount;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            if (deviceFound) {
                break;
            } else {
                throw std::runtime_error("Failed to set the audio device after " + std::to_string(maxRetries) + " attempts.");
            }
        }
    }

    if (!deviceFound) {
        throw std::runtime_error("Device with the specified name not found.");
    }
}
