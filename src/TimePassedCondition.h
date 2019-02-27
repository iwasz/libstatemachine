#ifndef TimePassedCondition_H
#define TimePassedCondition_H

#include "Condition.h"
#include <cstdint>

class TimeCounter;

/**
 * @brief Minęło x milisekund od ostatniego przejścia stanowego.
 */
class TimePassedCondition : public Condition {
public:
        TimePassedCondition (uint16_t m, TimeCounter *s) : msPassed (m), timeCounter (s) {}
        virtual ~TimePassedCondition () = default;

private:
        bool checkImpl (EventType const &event) const;

private:
        uint16_t msPassed;
        TimeCounter *timeCounter;
};

/**
 *
 */
extern TimePassedCondition *msPassed (uint16_t m, TimeCounter *s);

#endif // TimePassedCondition_H
