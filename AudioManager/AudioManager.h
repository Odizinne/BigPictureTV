#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <string>

struct Device
{
    int index;
    std::string name;
    std::string type;
    bool isDefault = false;
};

namespace AudioManager
{
    void setAudioDevice(const std::string &deviceName);
    void detectNewOutputs();
    std::string getDefaultOutputDevice();
};

#endif // AUDIOMANAGER_H
