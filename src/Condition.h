/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#pragma once
#include "StateMachineTypes.h"

enum class InputRetention { IGNORE_INPUT, RETAIN_INPUT };

template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> struct Condition {

        using EventType = EventT;
        using Types = StateMachineTypes<EventType>;
        using EventQueue = typename Types::EventQueue;

        virtual ~Condition () = default;

        /**
         * @brief Wywołuje check i kopiuje input jeśli retainInput == true
         * @param data
         * @param retainedInput
         * @param inputConsumed
         * @return
         */
        virtual bool check (EventType const &event) const
        {
                result = checkImpl (event);
                return result;
        }

        /**
         * @brief Sprawdza jakiś warunek.
         * @param data Dane wejściowe z kolejki.
         * @return Czy warunek jest spełniony
         */
        virtual bool checkImpl (EventType const & /*event*/) const { return false; }

        virtual bool getResult () const { return result; }
        virtual void reset () { result = false; }

        Condition *next = nullptr;

        InputRetention getRetainInput () const { return retainInput; }

protected:
        Condition (InputRetention r = InputRetention::IGNORE_INPUT) : retainInput (r) {}

private:
        InputRetention retainInput;
        mutable bool result = false;
};

/*****************************************************************************/

/**
 * Można podać np. lambdę.
 */
template <typename EventT, typename Func> class FuncCondition : public Condition<EventT> {
public:
        using EventType = EventT;
        FuncCondition (Func func) : func (func) {}
        virtual ~FuncCondition () = default;
        virtual bool checkImpl (EventType const &event) const { return func (event); }

private:
        Func func;
};
