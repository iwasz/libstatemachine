#include "DelayAction.h"
#include "Timer.h"
#include <cstdio>

bool DelayAction::run (const EventType &event)
{
        if (!lastMs) {
                lastMs = Timer::getTick ();
        }

        uint32_t currentMs = Timer::getTick ();

        if (currentMs - lastMs >= delayMs) {
                lastMs = 0;

#if 0 && !defined(UNIT_TEST)
        printf ("delay %dms\n", delayMs);
#endif

                return true;
        }

        return false;
}

DelayAction *delayMs (uint16_t delay) { return new DelayAction (delay); }
