#ifndef NIGHTLIGHTSWITCHER_H
#define NIGHTLIGHTSWITCHER_H

#include <windows.h>

namespace NightLightSwitcher
{
    bool supported();
    bool enabled();
    void enable();
    void disable();
    void toggle();
}

#endif // NIGHTLIGHTSWITCHER_H
