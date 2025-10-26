#include "windoweventmonitor.h"
#include <QDebug>

WindowEventMonitor* WindowEventMonitor::s_instance = nullptr;

WindowEventMonitor::WindowEventMonitor(QObject *parent)
    : QObject(parent)
    , m_eventHook(nullptr)
{
    s_instance = this;
}

WindowEventMonitor::~WindowEventMonitor()
{
    stop();
    s_instance = nullptr;
}

bool WindowEventMonitor::start()
{
    if (m_eventHook) {
        return true; // Already started
    }

    // Hook into foreground window changes
    m_eventHook = SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND,        // eventMin
        EVENT_SYSTEM_FOREGROUND,        // eventMax
        nullptr,                        // hmodWinEventProc
        WinEventProc,                   // pfnWinEventProc
        0,                              // idProcess (0 = all processes)
        0,                              // idThread (0 = all threads)
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS
    );

    if (!m_eventHook) {
        qWarning() << "Failed to set Windows event hook";
        return false;
    }

    qDebug() << "Windows event monitor started";
    return true;
}

void WindowEventMonitor::stop()
{
    if (m_eventHook) {
        UnhookWinEvent(m_eventHook);
        m_eventHook = nullptr;
        qDebug() << "Windows event monitor stopped";
    }
}

void CALLBACK WindowEventMonitor::WinEventProc(
    HWINEVENTHOOK hWinEventHook,
    DWORD event,
    HWND hwnd,
    LONG idObject,
    LONG idChild,
    DWORD dwEventThread,
    DWORD dwmsEventTime)
{
    Q_UNUSED(hWinEventHook)
    Q_UNUSED(event)
    Q_UNUSED(idObject)
    Q_UNUSED(idChild)
    Q_UNUSED(dwEventThread)
    Q_UNUSED(dwmsEventTime)

    if (!s_instance || !hwnd) {
        return;
    }

    // Get window title
    int length = GetWindowTextLengthW(hwnd);
    if (length > 0) {
        wchar_t* buffer = new wchar_t[length + 1];
        GetWindowTextW(hwnd, buffer, length + 1);
        QString windowTitle = QString::fromWCharArray(buffer);
        delete[] buffer;

        if (!windowTitle.isEmpty()) {
            emit s_instance->windowActivated(windowTitle);
        }
    } else {
        // Window was closed or has no title
        emit s_instance->windowDestroyed();
    }
}
