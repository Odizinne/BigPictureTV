#include "audiodevicenotifier.h"
#include <QDebug>
#include <Functiondiscoverykeys_devpkey.h>

AudioDeviceNotifier::AudioDeviceNotifier(QObject *parent)
    : QObject(parent)
    , m_refCount(1)
    , m_isListening(false)
{
}

AudioDeviceNotifier::~AudioDeviceNotifier()
{
    stopListening();
}

bool AudioDeviceNotifier::startListening(const QStringList &currentDeviceIds)
{
    if (m_isListening) {
        qWarning() << "Already listening for audio device changes";
        return false;
    }

    // Store the snapshot of current device IDs
    m_deviceIdsBeforeListening = currentDeviceIds;

    // Initialize COM
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        qWarning() << "Failed to initialize COM for audio device notifier";
        return false;
    }

    // Create device enumerator
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&m_pEnumerator));
    if (FAILED(hr)) {
        qWarning() << "Failed to create device enumerator for notifications";
        CoUninitialize();
        return false;
    }

    // Register for notifications
    hr = m_pEnumerator->RegisterEndpointNotificationCallback(this);
    if (FAILED(hr)) {
        qWarning() << "Failed to register for audio device notifications";
        m_pEnumerator.Release();
        CoUninitialize();
        return false;
    }

    m_isListening = true;
    qDebug() << "Started listening for audio device changes. Current devices:" << m_deviceIdsBeforeListening.size();
    return true;
}

void AudioDeviceNotifier::stopListening()
{
    if (!m_isListening) {
        return;
    }

    if (m_pEnumerator) {
        m_pEnumerator->UnregisterEndpointNotificationCallback(this);
        m_pEnumerator.Release();
    }

    CoUninitialize();
    m_isListening = false;
    m_deviceIdsBeforeListening.clear();
    qDebug() << "Stopped listening for audio device changes";
}

// IUnknown methods
STDMETHODIMP AudioDeviceNotifier::QueryInterface(REFIID riid, void **ppvObject)
{
    if (ppvObject == nullptr) {
        return E_POINTER;
    }

    if (riid == __uuidof(IUnknown) || riid == __uuidof(IMMNotificationClient)) {
        *ppvObject = static_cast<IMMNotificationClient*>(this);
        AddRef();
        return S_OK;
    }

    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) AudioDeviceNotifier::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG) AudioDeviceNotifier::Release()
{
    LONG refCount = InterlockedDecrement(&m_refCount);
    if (refCount == 0) {
        // Don't delete this object - it's managed by Qt
    }
    return refCount;
}

// IMMNotificationClient methods
STDMETHODIMP AudioDeviceNotifier::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
    if (!m_isListening) {
        return S_OK;
    }

    QString deviceId = QString::fromWCharArray(pwstrDeviceId);

    // Check if this is a NEW device (not in our snapshot)
    if (m_deviceIdsBeforeListening.contains(deviceId)) {
        qDebug() << "Device added but was already in snapshot, ignoring:" << deviceId;
        return S_OK;
    }

    qDebug() << "NEW audio device detected:" << deviceId;

    // Get device friendly name
    CComPtr<IMMDevice> pDevice;
    HRESULT hr = m_pEnumerator->GetDevice(pwstrDeviceId, &pDevice);
    if (SUCCEEDED(hr)) {
        CComPtr<IPropertyStore> pProperties;
        hr = pDevice->OpenPropertyStore(STGM_READ, &pProperties);
        if (SUCCEEDED(hr)) {
            PROPVARIANT varName;
            PropVariantInit(&varName);
            hr = pProperties->GetValue(PKEY_Device_FriendlyName, &varName);
            if (SUCCEEDED(hr)) {
                QString deviceName = QString::fromWCharArray(varName.pwszVal);
                qDebug() << "New device name:" << deviceName;

                // Emit signal to notify that a new audio device was detected
                emit newAudioDeviceDetected(deviceId, deviceName);

                PropVariantClear(&varName);
            }
        }
    }

    return S_OK;
}

STDMETHODIMP AudioDeviceNotifier::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
    Q_UNUSED(pwstrDeviceId)
    return S_OK;
}

STDMETHODIMP AudioDeviceNotifier::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    if (!m_isListening) {
        return S_OK;
    }

    // Check if a device became ACTIVE that wasn't in our snapshot
    if (dwNewState == DEVICE_STATE_ACTIVE) {
        QString deviceId = QString::fromWCharArray(pwstrDeviceId);

        if (!m_deviceIdsBeforeListening.contains(deviceId)) {
            qDebug() << "Device became ACTIVE (was likely DISABLED):" << deviceId;

            // Get device friendly name
            CComPtr<IMMDevice> pDevice;
            HRESULT hr = m_pEnumerator->GetDevice(pwstrDeviceId, &pDevice);
            if (SUCCEEDED(hr)) {
                CComPtr<IPropertyStore> pProperties;
                hr = pDevice->OpenPropertyStore(STGM_READ, &pProperties);
                if (SUCCEEDED(hr)) {
                    PROPVARIANT varName;
                    PropVariantInit(&varName);
                    hr = pProperties->GetValue(PKEY_Device_FriendlyName, &varName);
                    if (SUCCEEDED(hr)) {
                        QString deviceName = QString::fromWCharArray(varName.pwszVal);
                        qDebug() << "Newly active device name:" << deviceName;

                        // Emit signal for newly active device
                        emit newAudioDeviceDetected(deviceId, deviceName);

                        PropVariantClear(&varName);
                    }
                }
            }
        }
    }

    return S_OK;
}

STDMETHODIMP AudioDeviceNotifier::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
    Q_UNUSED(flow)
    Q_UNUSED(role)
    Q_UNUSED(pwstrDefaultDeviceId)
    return S_OK;
}

STDMETHODIMP AudioDeviceNotifier::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
    Q_UNUSED(pwstrDeviceId)
    Q_UNUSED(key)
    return S_OK;
}
