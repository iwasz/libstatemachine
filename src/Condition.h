#ifndef ICONDITION_H
#define ICONDITION_H

#include "StringQueue.h"

struct Condition {

        enum InputRetention { IGNORE_INPUT, RETAIN_INPUT };

        Condition (InputRetention r = IGNORE_INPUT) : retainInput (r) {}
        virtual ~Condition () = default;

        /**
         * @brief check
         * @param queue
         * @param retainedInput J
         * @param inputConsumed
         * @return
         */
        virtual bool check (StringQueue *queue, uint8_t inputNum, char *retainedInput) const;

protected:

        /**
         * @brief Wywołuje check i kopiuje input jeśli retainInput == true
         * @param data
         * @param retainedInput
         * @param inputConsumed
         * @return
         */
        virtual bool checkAndRetain (const char *data, char *retainedInput) const;

        /**
         * @brief Sprawdza jakiś warunek.
         * @param data Dane wejściowe z kolejki.
         * @return Czy warunek jest spełniony
         */
        virtual bool checkImpl (const char *data) const = 0;

        friend class NotCondition;
        friend class AndCondition;
        friend class OrCondition;
        friend class SequenceCondition;

        InputRetention retainInput;
};

/**
 * Można podać np. lambdę.
 */
template <typename Func>
class FuncCondition : public Condition {
public:

        FuncCondition (Func func) : func (func) {}
        virtual ~FuncCondition () {}
        virtual bool checkImpl (const char *data) const { return func (data); }

private:

        Func func;
};

#endif // ICONDITION_H

