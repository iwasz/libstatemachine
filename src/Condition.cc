/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "Condition.h"
#include <cstring>

#if 0
bool Condition::check (EventQueue &eventQueue, uint8_t inputNum, EventType &retainedInput) const
{
//        bool conditionMet = false;
//        EventType const &el;

        if (eventQueue.size ()) {
//                uint8_t i = 0;
//                while (i < inputNum && (el = eventQueue.front (i++)) && !(conditionMet = checkAndRetain (el, retainedInput)))
//                        ;

                for (int i = 0; i < inputNum; ++i) {
                    if (checkAndRetain (eventQueue.front (i), retainedInput)) {
                        return true;
                    }
                }
        }
        else {
                /*conditionMet = */return checkImpl (static_cast<const char *> (nullptr));
        }

//        return conditionMet;
        return false;
}
#endif

/*****************************************************************************/

bool Condition::check (EventType const &event, EventType &retainedEvent) const
{
        result = checkImpl (event);

        if (result && retainInput) {
                retainedEvent = event;
        }

        return result;
}
