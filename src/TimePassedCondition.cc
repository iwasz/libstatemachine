#include "TimePassedCondition.h"
#include "TimeCounter.h"
#include <cctype>
#include <cstdio>

/*****************************************************************************/

bool TimePassedCondition::checkImpl (const EventType &) const
{
        bool b = timeCounter->msSinceLastStateChange () > msPassed;

#ifndef UNIT_TEST
        if (b) {
                printf ("Timeout reached : [%d]\n", msPassed);
        }
#endif

        return b;
}

/*****************************************************************************/

TimePassedCondition *msPassed (uint16_t m, TimeCounter *s) { return new TimePassedCondition (m, s); }
