#include "AudioManager.h"
#include <QProcess>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <thread>

std::string executeCommand(const std::string &command)
{
    QProcess process;
    process.setProgram("powershell.exe");

    QStringList arguments;
    arguments << "-Command" << QString::fromStdString(command);
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
            currentDevice = Device{-1, "", ""};
            deviceStarted = true;
            currentDevice.index = std::stoi(line.substr(line.find(":") + 2));
        } else if (line.find("Name") != std::string::npos) {
            std::string_view lineView(line);
            currentDevice.name = std::string(lineView.substr(lineView.find(":") + 2));
        } else if (line.find("Type") != std::string::npos) {
            std::string_view lineView(line);
            currentDevice.type = std::string(lineView.substr(lineView.find(":") + 2));
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

void setAudioDevice(const std::string &deviceName)
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
                    break;
                }
            }
        }

        if (deviceFound) {
            break;
        }

        ++retryCount;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (!deviceFound) {
        throw std::runtime_error("Failed to set the audio device after "
                                 + std::to_string(maxRetries) + " attempts.");
    }
}
