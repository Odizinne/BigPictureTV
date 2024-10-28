#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <string>

struct Device
{
    int index;
    std::string name;
    std::string type;
};

namespace AudioManager
{
    void setAudioDevice(const std::string &deviceName);
};

#endif // AUDIOMANAGER_H
