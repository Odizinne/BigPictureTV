#include "AudioManager.h"
#include <QProcess>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <QDebug>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <comdef.h>
#include <atlbase.h>

std::string executeCommand(QString command)
{
    QProcess process;
    process.setProgram("powershell.exe");

    QStringList arguments;
    arguments << "-NoProfile" << "-Command" << command;
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
        return error.toStdString();
    }

    return output.toStdString();
}


void AudioManager::setAudioDevice(QString ID)
{
    bool deviceFound = false;
    int maxRetries = 10;
    int retryCount = 0;
    std::string result;
    qDebug() << "Will try to set output to:" << ID;

    while (retryCount < maxRetries) {
        // Correctly add double quotes around the ID, without adding escape characters
        QString setCommand = QString("Set-AudioDevice -ID \"") + ID + "\"";
        result = executeCommand(setCommand);

        if (result.find("Error") == std::string::npos) {
            deviceFound = true;
            qDebug() << "Switched audio output to:" << ID;
            break;
        }

        if (deviceFound) {
            break;
        }

        qDebug() << "Device missing, retrying. Retry count:" << retryCount;
        ++retryCount;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (!deviceFound) {
        qDebug() << "Unable to set audio device.";
    }
}

QList<Device> AudioManager::ListAudioOutputDevices()
{
    QList<Device> devices;

    // Initialize COM
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        qDebug() << "Failed to initialize COM library.";
        return devices; // Return an empty list if initialization fails
    }

    // Create an instance of the IMMDeviceEnumerator interface
    CComPtr<IMMDeviceEnumerator> pEnumerator;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&pEnumerator));
    if (FAILED(hr)) {
        qDebug() << "Failed to create device enumerator.";
        CoUninitialize();
        return devices;
    }

    // Enumerate all audio output devices (eRender), excluding disabled ones
    CComPtr<IMMDeviceCollection> pCollection;
    hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE | DEVICE_STATE_UNPLUGGED, &pCollection);
    if (FAILED(hr)) {
        qDebug() << "Failed to enumerate audio output devices.";
        CoUninitialize();
        return devices;
    }

    // Get the number of audio devices
    UINT count;
    hr = pCollection->GetCount(&count);
    if (FAILED(hr)) {
        qDebug() << "Failed to get audio device count.";
        CoUninitialize();
        return devices;
    }

    // Loop through each device and collect its details
    for (UINT i = 0; i < count; i++) {
        CComPtr<IMMDevice> pDevice;
        hr = pCollection->Item(i, &pDevice);
        if (FAILED(hr)) {
            qDebug() << "Failed to get audio device at index " << i << ".";
            continue;
        }

        // Get the device state
        DWORD dwState;
        hr = pDevice->GetState(&dwState);
        if (FAILED(hr)) {
            qDebug() << "Failed to get device state.";
            continue;
        }

        // If the device is disabled, skip it
        if (dwState == DEVICE_STATE_DISABLED) {
            continue;
        }

        // Get the device properties
        CComPtr<IPropertyStore> pProperties;
        hr = pDevice->OpenPropertyStore(STGM_READ, &pProperties);
        if (FAILED(hr)) {
            qDebug() << "Failed to open property store.";
            continue;
        }

        // Get the device name
        PROPVARIANT varName;
        PropVariantInit(&varName);
        hr = pProperties->GetValue(PKEY_Device_FriendlyName, &varName);
        if (SUCCEEDED(hr)) {
            // Get the device ID
            LPWSTR deviceId = nullptr;
            hr = pDevice->GetId(&deviceId);
            if (SUCCEEDED(hr)) {
                // Add device info to the list
                devices.append(Device{
                    QString::fromWCharArray(varName.pwszVal),  // Device name
                    QString::fromWCharArray(deviceId)          // Device ID
                });

                CoTaskMemFree(deviceId); // Free the device ID memory
            }

            PropVariantClear(&varName);
        }
    }

    // Clean up COM
    CoUninitialize();

    return devices;
}

void AudioManager::PrintAudioOutputDevices()
{
    QList<Device> devices = ListAudioOutputDevices();

    for (const Device& device : devices) {
        qDebug() << "Device Name: " << device.name;
        qDebug() << "Device ID: " << device.ID;
    }
}
