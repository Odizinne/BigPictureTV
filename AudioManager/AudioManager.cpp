#include "AudioManager.h"
#include <QProcess>
#include <chrono>
#include <thread>
#include <QDebug>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <comdef.h>
#include <atlbase.h>
#include "PolicyConfig.h"

bool setDefaultAudioOutputDevice(const QString &deviceId)
{
    HRESULT hr;
    IMMDeviceEnumerator *deviceEnumerator = nullptr;
    IMMDevice *defaultDevice = nullptr;
    IPolicyConfig *policyConfig = nullptr;
    IPolicyConfigVista *policyConfigVista = nullptr;

    // Initialize COM library
    hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        qDebug() << "Failed to initialize COM library";
        return false;
    }

    // Create device enumerator
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&deviceEnumerator));
    if (FAILED(hr)) {
        qDebug() << "Failed to create device enumerator";
        CoUninitialize();
        return false;
    }

    // Get the device by ID
    hr = deviceEnumerator->GetDevice(reinterpret_cast<LPCWSTR>(deviceId.utf16()), &defaultDevice);
    if (FAILED(hr)) {
        qDebug() << "Failed to get device by ID";
        deviceEnumerator->Release();
        CoUninitialize();
        return false;
    }

    // Attempt to get IPolicyConfig interface
    hr = CoCreateInstance(__uuidof(CPolicyConfigClient), nullptr, CLSCTX_ALL,
                          IID_PPV_ARGS(&policyConfig));
    if (FAILED(hr)) {
        // If IPolicyConfig is unavailable, try IPolicyConfigVista
        hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient), nullptr, CLSCTX_ALL,
                              IID_PPV_ARGS(&policyConfigVista));
        if (FAILED(hr)) {
            qDebug() << "Failed to create PolicyConfig interface";
            defaultDevice->Release();
            deviceEnumerator->Release();
            CoUninitialize();
            return false;
        }
    }

    // Set as default device for all audio roles (Console, Multimedia, Communications)
    if (policyConfig) {
        hr = policyConfig->SetDefaultEndpoint(reinterpret_cast<LPCWSTR>(deviceId.utf16()), eConsole);
        policyConfig->SetDefaultEndpoint(reinterpret_cast<LPCWSTR>(deviceId.utf16()), eMultimedia);
        policyConfig->SetDefaultEndpoint(reinterpret_cast<LPCWSTR>(deviceId.utf16()), eCommunications);
    } else if (policyConfigVista) {
        hr = policyConfigVista->SetDefaultEndpoint(reinterpret_cast<LPCWSTR>(deviceId.utf16()), eConsole);
        policyConfigVista->SetDefaultEndpoint(reinterpret_cast<LPCWSTR>(deviceId.utf16()), eMultimedia);
        policyConfigVista->SetDefaultEndpoint(reinterpret_cast<LPCWSTR>(deviceId.utf16()), eCommunications);
    }

    // Release resources
    if (policyConfig) policyConfig->Release();
    if (policyConfigVista) policyConfigVista->Release();
    defaultDevice->Release();
    deviceEnumerator->Release();
    CoUninitialize();

    if (FAILED(hr)) {
        qDebug() << "Failed to set default audio output device";
        return false;
    }

    qDebug() << "Default audio output device set to:" << deviceId;
    return true;
}


void AudioManager::setAudioDevice(QString ID)
{
    bool deviceFound = false;
    int maxRetries = 10;
    int retryCount = 0;
    qDebug() << "Will try to set output to:" << ID;

    while (retryCount < maxRetries) {

        if (setDefaultAudioOutputDevice(ID)) {
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

