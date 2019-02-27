/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef GPPATIMECOUNTER_H
#define GPPATIMECOUNTER_H

#include "Timer.h"
#include <cstdint>

class TimeCounter {
public:
        TimeCounter () : lastStateChangeTs (0) {}

        uint32_t msSinceLastStateChange () const { return Timer::getTick () - lastStateChangeTs; }
        void reset () { lastStateChangeTs = 0; }
        void set () { lastStateChangeTs = Timer::getTick (); }

private:
        uint32_t lastStateChangeTs;
};

#endif // TIMECOUNTER_H
