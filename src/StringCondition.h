#ifndef STRINGCONDITION_H
#define STRINGCONDITION_H

#include "Condition.h"
#include <cctype>

enum class StripInput { DONT_STRIP, STRIP };

/**
 * @brief Warunek porównujący wejście z napisem który podajemy jako arg. konstruktora.
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class StringCondition : public Condition<EventT> {
public:
        using EventType = EventT;

        StringCondition (const char *condition, StripInput stripInput = StripInput::STRIP, bool ne = false,
                         InputRetention retainInput = InputRetention::IGNORE_INPUT)
            : Condition<EventType> (retainInput), condition (condition), stripInput (stripInput), negated (ne)
        {
        }

#ifndef UNIT_TEST
protected:
#endif
        virtual bool checkImpl (EventType const &event) const;

        const char *condition;
        StripInput stripInput;
        bool negated;
};

/*****************************************************************************/

template <typename EventT> bool StringCondition<EventT>::checkImpl (EventType const &event) const
{
        if (event.empty ()) {
                return !condition;
        }

        const char *c = condition;
        int ei = 0;
        int ci = 0;

        // Stripuj początek.
        if (stripInput == StripInput::STRIP) {
                while (ei < event.size () && std::isspace (event[ei])) {
                        ++ei;
                }
        }

        while (ei < event.size () /*&& ci < c.size () */ && (event[ei] == *c)) {
                ++c;
                ++ei;
        }

        // Stripuj koniec.
        if (stripInput == StripInput::STRIP) {
                while (ei < event.size () && std::isspace (event[ei])) {
                        ++ei;
                }
        }

        bool result = (event[ei] == *c);
        return (negated) ? (!result) : (result);
}

/*****************************************************************************/

template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE>
StringCondition<EventT> *eq (const char *condition, StripInput stripInput = StripInput::STRIP,
                             InputRetention retainInput = InputRetention::IGNORE_INPUT)
{
        return new StringCondition<EventT> (condition, stripInput, false, retainInput);
}

/*****************************************************************************/

template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE>
StringCondition<EventT> *ne (const char *condition, StripInput stripInput = StripInput::STRIP,
                             InputRetention retainInput = InputRetention::IGNORE_INPUT)
{
        return new StringCondition<EventT> (condition, stripInput, true);
}

#endif // STRINGCONDITION_H
