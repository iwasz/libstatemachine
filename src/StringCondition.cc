#include <cctype>
#include "StringCondition.h"

bool StringCondition::checkImpl (const char *data) const
{
        if (!data) {
                return !condition;
        }

        const char *c = condition;

        // Stripuj początek.
        if (stripInput) {
                while (std::isspace (*data)) {
                        ++data;
                }
        }

        while (*data && (*data == *c)) {
                ++data, ++c;
        }

        // Stripuj koniec.
        if (stripInput) {
                while (std::isspace (*data)) {
                        ++data;
                }
        }

        bool result = (*(const unsigned char *)data - *(const unsigned char *)c) == 0;
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
