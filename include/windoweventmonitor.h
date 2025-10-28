#ifndef WINDOWEVENTMONITOR_H
#define WINDOWEVENTMONITOR_H

#include <QObject>
#include <windows.h>

class WindowEventMonitor : public QObject
{
    Q_OBJECT

public:
    explicit WindowEventMonitor(QObject *parent = nullptr);
    ~WindowEventMonitor();

    bool start();
    void stop();

signals:
    void windowActivated(QString windowTitle);
    void windowDestroyed();

public slots:
    void trackWindow(QString windowTitle);

private:
    static void CALLBACK WinEventProc(
        HWINEVENTHOOK hWinEventHook,
        DWORD event,
        HWND hwnd,
        LONG idObject,
        LONG idChild,
        DWORD dwEventThread,
        DWORD dwmsEventTime
    );

    static WindowEventMonitor* s_instance;

    HWINEVENTHOOK m_eventHook;
    HWND m_trackedWindow;
};

#endif // WINDOWEVENTMONITOR_H
