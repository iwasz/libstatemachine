#ifndef ICONDITION_H
#define ICONDITION_H

#include "StateMachineTypes.h"
#include "StringQueue.h"

struct Condition {

        using EventType = string;
        using Types = StateMachineTypes<EventType>;
        using EventQueue = Types::EventQueue;

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
        virtual bool check (EventQueue &eventQueue, uint8_t inputNum, EventType &retainedInput) const;

protected:
        /**
         * @brief Wywołuje check i kopiuje input jeśli retainInput == true
         * @param data
         * @param retainedInput
         * @param inputConsumed
         * @return
         */
        virtual bool checkAndRetain (EventType const &event, EventType &retainedEvent) const;

        /**
         * @brief Sprawdza jakiś warunek.
         * @param data Dane wejściowe z kolejki.
         * @return Czy warunek jest spełniony
         */
        virtual bool checkImpl (EventType const &event) const = 0;

        friend class NotCondition;
        friend class AndCondition;
        friend class OrCondition;
        friend class SequenceCondition;

        InputRetention retainInput;
};

///**
// * Można podać np. lambdę.
// */
// template <typename Func> class FuncCondition : public Condition {
// public:
//        FuncCondition (Func func) : func (func) {}
//        virtual ~FuncCondition () {}
//        virtual bool checkImpl (const char *data) const { return func (data); }

// private:
//        Func func;
//};

#endif // ICONDITION_H
