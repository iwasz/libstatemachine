/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "Condition.h"
#include <cstring>

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

/*****************************************************************************/

bool Condition::checkAndRetain (EventType const &event, EventType &retainedEvent) const
{
        bool conditionMet = checkImpl (event);

        if (conditionMet && retainInput) {
                retainedEvent = event;
        }

        return conditionMet;
}
