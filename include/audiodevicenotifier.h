#ifndef AUDIODEVICENOTIFIER_H
#define AUDIODEVICENOTIFIER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <mmdeviceapi.h>
#include <atlbase.h>

class AudioDeviceNotifier : public QObject, public IMMNotificationClient
{
    Q_OBJECT

public:
    explicit AudioDeviceNotifier(QObject *parent = nullptr);
    ~AudioDeviceNotifier();

    // Start listening for device changes with a snapshot of current device IDs
    bool startListening(const QStringList &currentDeviceIds);
    void stopListening();

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject) override;
    STDMETHOD_(ULONG, AddRef)() override;
    STDMETHOD_(ULONG, Release)() override;

    // IMMNotificationClient methods
    STDMETHOD(OnDeviceStateChanged)(LPCWSTR pwstrDeviceId, DWORD dwNewState) override;
    STDMETHOD(OnDeviceAdded)(LPCWSTR pwstrDeviceId) override;
    STDMETHOD(OnDeviceRemoved)(LPCWSTR pwstrDeviceId) override;
    STDMETHOD(OnDefaultDeviceChanged)(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override;
    STDMETHOD(OnPropertyValueChanged)(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override;

signals:
    void newAudioDeviceDetected(QString deviceId, QString deviceName);

private:
    LONG m_refCount;
    CComPtr<IMMDeviceEnumerator> m_pEnumerator;
    QStringList m_deviceIdsBeforeListening;
    bool m_isListening;
};

#endif // AUDIODEVICENOTIFIER_H
