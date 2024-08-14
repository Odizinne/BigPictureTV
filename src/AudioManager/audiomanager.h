#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <string>
#include <vector>

struct Device
{
    int index;
    std::string name;
    std::string type;
};

std::string executeCommand(const std::string &command);
std::vector<Device> parseDevices(const std::string &output);
bool checkDevice(const std::string &deviceName);
void setAudioDevice(const std::string &deviceName);

#endif // AUDIOMANAGER_H
