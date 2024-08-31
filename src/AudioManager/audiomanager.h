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

class AudioManager
{
public:
    AudioManager();
    ~AudioManager();

    void setAudioDevice(const std::string &deviceName);

private:
    bool checkDevice(const std::string &deviceName);
    std::string executeCommand(const std::string &command);
    std::vector<Device> parseDevices(const std::string &output);
    bool containsIgnoreCase(const std::string &str, const std::string &substr);
};

#endif // AUDIOMANAGER_H
