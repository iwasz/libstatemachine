/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include <cstring>
#include "Condition.h"

bool Condition::check (StringQueue *queue, uint8_t inputNum, char *retainedInput) const
{
        bool conditionMet = false;
        char *el;

        if (queue->size ()) {
                uint8_t i = 0;
                while (i < inputNum && (el = queue->front (i++)) && !(conditionMet = checkAndRetain (el, retainedInput)))
                        ;
        }
        else {
                conditionMet = checkImpl (static_cast <const char *> (nullptr));
        }

        return conditionMet;
}

/*****************************************************************************/

bool Condition::checkAndRetain (const char *data, char *retainedInput) const
{
        bool conditionMet = checkImpl (data);

        if (conditionMet && retainInput) {
                strncpy (retainedInput, data, STRING_QUEUE_BUFFER_SIZE);
        }

        return conditionMet;
}
