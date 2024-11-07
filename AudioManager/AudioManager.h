#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QString>

struct Device
{
    QString name;
    QString ID;
};

namespace AudioManager
{
    void setAudioDevice(QString ID);
    QList<Device> ListAudioOutputDevices();
    void PrintAudioOutputDevices();
};

#endif // AUDIOMANAGER_H
