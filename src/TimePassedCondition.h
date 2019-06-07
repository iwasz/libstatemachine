/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef TimePassedCondition_H
#define TimePassedCondition_H

#include "Condition.h"
#include "TimeCounter.h"
#include <cctype>
#include <cstdint>

/**
 * @brief Minęło x milisekund od ostatniego przejścia stanowego.
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class TimePassedCondition : public Condition<EventT> {
public:
        using EventType = EventT;

        TimePassedCondition (uint16_t m, TimeCounter *s) : msPassed (m), timeCounter (s) {}
        virtual ~TimePassedCondition () = default;

protected:
        bool checkImpl (EventType const &event) const;

private:
        uint16_t msPassed;
        TimeCounter *timeCounter;
};

/*****************************************************************************/

template <typename EventT> bool TimePassedCondition<EventT>::checkImpl (const EventType &) const
{
        bool b = timeCounter->msSinceLastStateChange () > msPassed;

#ifndef UNIT_TEST
//        if (b) {
//                printf ("Timeout reached : [%d]\n", msPassed);
//        }
#endif

        return b;
}

/*****************************************************************************/

/**
 *
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> TimePassedCondition<EventT> *msPassed (uint16_t m, TimeCounter *s)
{
        return new TimePassedCondition<EventT> (m, s);
}

#endif // TimePassedCondition_H
