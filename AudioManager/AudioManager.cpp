#include "AudioManager.h"
#include <QProcess>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <QDebug>

std::string executeCommand(const std::string &command)
{
    QProcess process;
    process.setProgram("powershell.exe");

    QStringList arguments;
    arguments << "-NoProfile" << "-Command" << QString::fromStdString(command);
    process.setArguments(arguments);

    process.start();
    if (!process.waitForStarted()) {
        throw std::runtime_error("Failed to start PowerShell process!");
    }
    if (!process.waitForFinished()) {
        throw std::runtime_error("PowerShell process did not finish!");
    }

    QByteArray output = process.readAllStandardOutput();
    QByteArray error = process.readAllStandardError();

    if (!error.isEmpty()) {
        throw std::runtime_error("PowerShell Error: " + error.toStdString());
    }

    return output.toStdString();
}

std::vector<Device> parseDevices(const std::string &output)
{
    std::vector<Device> devices;
    devices.reserve(20);
    std::istringstream stream(output);
    std::string line;
    Device currentDevice;
    bool deviceStarted = false;

    while (std::getline(stream, line)) {
        if (line.find("Index") != std::string::npos) {
            if (deviceStarted) {
                devices.push_back(currentDevice);
            }
            currentDevice = Device{-1, "", "", false};
            deviceStarted = true;
            currentDevice.index = std::stoi(line.substr(line.find(":") + 2));
        } else if (line.find("Name") != std::string::npos) {
            currentDevice.name = line.substr(line.find(":") + 2);
        } else if (line.find("Type") != std::string::npos) {
            currentDevice.type = line.substr(line.find(":") + 2);
        } else if (line.find("Default") != std::string::npos) {
            std::string defaultValue = line.substr(line.find(":") + 2);
            currentDevice.isDefault = (defaultValue == "True");
        }
    }

    if (deviceStarted) {
        devices.push_back(currentDevice);
    }

    return devices;
}

bool containsIgnoreCase(const std::string &str, const std::string &substr)
{
    std::string strLower = str;
    std::string substrLower = substr;
    std::transform(strLower.begin(), strLower.end(), strLower.begin(), ::tolower);
    std::transform(substrLower.begin(), substrLower.end(), substrLower.begin(), ::tolower);
    return strLower.find(substrLower) != std::string::npos;
}

bool checkDevice(const std::string &deviceName)
{
    std::string output = executeCommand("Get-AudioDevice -l");
    std::vector<Device> devices = parseDevices(output);

    for (const auto &device : devices) {
        if (containsIgnoreCase(device.name, deviceName)) {
            return true;
        }
    }
    return false;
}

std::string AudioManager::getDefaultOutputDevice()
{
    std::string output = executeCommand("Get-AudioDevice -l");
    std::vector<Device> devices = parseDevices(output);

    for (const auto &device : devices) {
        if (device.type == "Playback" && device.isDefault) {
            return device.name;
        }
    }

    return "";
}

void AudioManager::setAudioDevice(const std::string &deviceName)
{
    bool deviceFound = false;
    int maxRetries = 10;
    int retryCount = 0;
    std::string result;

    while (retryCount < maxRetries) {
        std::string output = executeCommand("Get-AudioDevice -l");
        std::vector<Device> devices = parseDevices(output);

        for (const auto &device : devices) {
            if (containsIgnoreCase(device.name, deviceName)) {
                if (device.index < 1) {
                    throw std::runtime_error("Invalid device index: "
                                             + std::to_string(device.index));
                }

                std::string setCommand = "Set-AudioDevice -Index " + std::to_string(device.index);
                result = executeCommand(setCommand);

                if (result.find("Error") == std::string::npos) {
                    deviceFound = true;
                    qDebug() << "Switchted audio output to:" << device.name;
                    break;
                }
            }
        }

        if (deviceFound) {
            break;
        }

        qDebug() << "Device missing, retrying. Retry count:" << retryCount;
        ++retryCount;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (!deviceFound) {
        throw std::runtime_error("Failed to set the audio device after "
                                 + std::to_string(maxRetries) + " attempts.");
    }
}

void AudioManager::detectNewOutputs()
{
    std::vector<Device> previousDevices = parseDevices(executeCommand("Get-AudioDevice -l"));
    int maxRetries = 10;
    int retryCount = 0;
    bool newDeviceFound = false;

    while (retryCount < maxRetries && !newDeviceFound) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::vector<Device> currentDevices = parseDevices(executeCommand("Get-AudioDevice -l"));

        for (const auto &currentDevice : currentDevices) {
            auto it = std::find_if(previousDevices.begin(), previousDevices.end(),
                                   [&](const Device &device) { return device.name == currentDevice.name; });

            if (it == previousDevices.end()) {
                qDebug() << "New audio output detected:" << QString::fromStdString(currentDevice.name);

                if (currentDevice.index < 1) {
                    throw std::runtime_error("Invalid device index: " + std::to_string(currentDevice.index));
                }

                std::string setCommand = "Set-AudioDevice -Index " + std::to_string(currentDevice.index);
                std::string result = executeCommand(setCommand);

                if (result.find("Error") == std::string::npos) {
                    qDebug() << "Switched audio output to new device:" << QString::fromStdString(currentDevice.name);
                    newDeviceFound = true;
                    break;
                } else {
                    qDebug() << "Failed to set new device due to error in command execution.";
                }
            }
        }

        if (newDeviceFound) {
            break;
        }

        qDebug() << "No new output found, retrying. Retry count:" << retryCount;
        previousDevices = currentDevices;
        ++retryCount;
    }

    if (!newDeviceFound) {
        throw std::runtime_error("Failed to detect and set a new audio device after "
                                 + std::to_string(maxRetries) + " attempts.");
    }
}
