/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef ACTION_I_ACTION_H
#define ACTION_I_ACTION_H

#include "ErrorHandler.h"
#include "StateMachineTypes.h"

/**
 * @brief Baza dla wszystkich akcji.
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class Action {
public:
        using EventType = EventT;

        Action () = default;
        Action (Action const &) = delete;
        Action &operator= (Action const &) = delete;
        Action (Action &&) = delete;
        Action &operator= (Action &&) = delete;
        virtual ~Action () = default;

        /**
         * @brief Uruchamia akcję.
         * @return Czy akcja zakończyła swoje działanie, czy nie (np. delay).
         */
        virtual bool run (EventType const &event) = 0;
};

#ifdef UNIT_TEST
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class TestAction : public Action<EventT> {
public:
};
#endif

/**
 * @brief Wykonuje dwie akcje po kolei.
 */
template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class AndAction : public Action<EventT> {
public:
        using EventType = EventT;

        AndAction (Action<EventT> *a, Action<EventT> *b) : a (a), b (b), current (nullptr) {}
        virtual ~AndAction () {}

        AndAction (AndAction const &) = delete;
        AndAction &operator= (AndAction const &) = delete;
        AndAction (AndAction const &&) noexcept = delete;
        AndAction &operator= (AndAction &&) noexcept = delete;

        bool run (EventType const &event);

private:
        Action<EventT> *a;
        Action<EventT> *b;
        Action<EventT> *current;
};

/*****************************************************************************/

template <typename EventT> bool AndAction<EventT>::run (EventType const &event)
{
        if (!current) {
                current = a;
        }

        if (current == a) {
                if (!a->run (event)) {
                        return false;
                }
                else {
                        current = b;
                }
        }

        if (current == b) {
                if (!b->run (event)) {
                        return false;
                }
                else {
                        current = nullptr;
                }
        }

        return true;
}

/*****************************************************************************/

template <typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> AndAction<EventT> *and_action (Action<EventT> *a, Action<EventT> *b)
{
        return new AndAction<EventT> (a, b);
}

/**
 * Szablon do tworzenia akcji, które mają funktor (na przykład lambdę).
 */
template <typename Func, typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> class FuncAction : public Action<EventT> {
public:
        using EventType = EventT;
        FuncAction (Func func) : func (func) {}
        virtual ~FuncAction () {}
        virtual bool run (EventType const &input) { return func (input); }

private:
        Func func;
};

/*****************************************************************************/

template <typename Func, typename EventT = LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE> FuncAction<Func, EventT> *func (Func func)
{
        return new FuncAction<Func, EventT> (func);
}

#endif // IACTION_H
