#ifndef STRINGCONDITION_H
#define STRINGCONDITION_H

#include "Condition.h"

/**
 * @brief Warunek porównujący wejście z napisem który podajemy jako arg. konstruktora.
 */
class StringCondition : public Condition {
public:
        enum StripInput { DONT_STRIP, STRIP };

        StringCondition (const char *condition, StripInput stripInput = STRIP, bool ne = false, InputRetention retainInput = IGNORE_INPUT)
            : Condition (retainInput), condition (condition), stripInput (stripInput), negated (ne)
        {
        }

#ifndef UNIT_TEST
protected:
#endif
        virtual bool checkImpl (const char *data) const;

        const char *condition;
        bool stripInput;
        bool negated;
};

/**
 * @brief eq
 * @param condition
 * @param stripInput
 * @return
 */
extern StringCondition *eq (const char *condition, StringCondition::StripInput stripInput = StringCondition::STRIP,
                            Condition::InputRetention retainInput = Condition::IGNORE_INPUT);

extern StringCondition *ne (const char *condition, StringCondition::StripInput stripInput = StringCondition::STRIP,
                            Condition::InputRetention retainInput = Condition::IGNORE_INPUT);

#endif // STRINGCONDITION_H
