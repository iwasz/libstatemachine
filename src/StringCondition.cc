#include "StringCondition.h"
#include <cctype>

bool StringCondition::checkImpl (EventType const &event) const
{
        if (event.empty ()) {
                return !condition;
        }

        const char *c = condition;
        int ei = 0;
        int ci = 0;

        // Stripuj początek.
        if (stripInput) {
                while (ei < event.size () && std::isspace (event[ei])) {
                        ++ei;
                }
        }

        while (ei < event.size () /*&& ci < c.size () */ && (event[ei] == *c)) {
                ++c;
                ++ei;
        }

        // Stripuj koniec.
        if (stripInput) {
                while (ei < event.size () && std::isspace (event[ei])) {
                        ++ei;
                }
        }

        bool result = (event[ei] == *c);
        return (negated) ? (!result) : (result);
}

/*****************************************************************************/

StringCondition *eq (const char *condition, StringCondition::StripInput stripInput, Condition::InputRetention retainInput)
{
        return new StringCondition (condition, stripInput, false, retainInput);
}

/*****************************************************************************/

StringCondition *ne (const char *condition, StringCondition::StripInput stripInput, Condition::InputRetention retainInput)
{
        return new StringCondition (condition, stripInput, true);
}
