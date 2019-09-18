/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef STRINGCONDITION_H
#define STRINGCONDITION_H

#include "Condition.h"
#include <cctype>
#include <cstring>
#include <etl/string_view.h>

enum class StripInput { DONT_STRIP, STRIP };

/**
 * @brief Warunek porównujący wejście z napisem który podajemy jako arg. konstruktora.
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class StringCondition : public Condition<EventT> {
public:
        using EventType = EventT;

        StringCondition (const char *condition, StripInput stripInput = StripInput::STRIP, bool ne = false,
                         InputRetention retainInput = InputRetention::IGNORE_INPUT)
            : Condition<EventType> (retainInput), condition (condition, strlen (condition)), stripInput (stripInput), negated (ne)
        {
        }

#ifndef UNIT_TEST
protected:
#endif
        virtual bool checkImpl (EventType const &event) const override;

        etl::string_view condition;
        StripInput stripInput;
        bool negated;
};

/*****************************************************************************/

template <typename EventT> bool StringCondition<EventT>::checkImpl (EventType const &event) const
{
        if (event.size () == 0) {
                return condition.empty ();
        }

        size_t ei = 0;
        size_t ci = 0;

        // Stripuj początek.
        if (stripInput == StripInput::STRIP) {
                while (ei < event.size () && std::isspace (event.at (ei))) {
                        ++ei;
                }
        }

        while (ei < event.size () && ci < condition.size () && (event.at (ei) == condition.at (ci))) {
                ++ei;
                ++ci;
        }

        // Stripuj koniec.
        if (stripInput == StripInput::STRIP) {
                while (ei < event.size () && std::isspace (event.at (ei))) {
                        ++ei;
                }
        }

        bool result = (ci == condition.size ()) && (ei == event.size ());
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

/**
 * Checks if event has non zero size.
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class NotEmptyCondition : public Condition<EventT> {
public:
        using EventType = EventT;

        NotEmptyCondition (InputRetention retainInput = InputRetention::IGNORE_INPUT) : Condition<EventType> (retainInput) {}

#ifndef UNIT_TEST
protected:
#endif
        virtual bool checkImpl (EventType const &event) const override { return event.size () > 0; }
};

/*****************************************************************************/

template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE>
NotEmptyCondition<EventT> *notEmpty (InputRetention retainInput = InputRetention::IGNORE_INPUT)
{
        return new NotEmptyCondition<EventT> (retainInput);
}

/*****************************************************************************/

template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class LengthCondition : public Condition<EventT> {
public:
        LengthCondition (size_t *len, InputRetention retainInput = InputRetention::IGNORE_INPUT) : Condition<EventT> (retainInput), len (len) {}

#ifndef UNIT_TEST
protected:
#endif
        bool checkImpl (EventT const &event) const override { return event.size () == *len; }

private:
        size_t *len;
};

/*****************************************************************************/

template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE>
LengthCondition<EventT> *len (size_t *l, InputRetention retainInput = InputRetention::IGNORE_INPUT)
{
        return new LengthCondition<EventT> (l, retainInput);
}

#endif // STRINGCONDITION_H
