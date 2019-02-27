#ifndef ACTION_IACTION_H
#define ACTION_IACTION_H

#include "StateMachineTypes.h"

/**
 * @brief Baza dla wszystkich akcji.
 */
class Action {
public:
        using EventType = string;

        virtual ~Action () = default;

        /**
         * @brief Uruchamia akcję.
         * @return Czy akcja zakończyła swoje działanie, czy nie (np. delay).
         */
        virtual bool run (EventType const &event) = 0;
};

#ifdef UNIT_TEST
class TestAction : public Action {
public:
};
#endif

/**
 * @brief Wykonuje dwie akcje po kolei.
 */
class AndAction : public Action {
public:
        AndAction (Action *a, Action *b) : a (a), b (b), current (nullptr) {}
        virtual ~AndAction () {}
        bool run (EventType const &event);

private:
        Action *a;
        Action *b;
        Action *current;
};

extern AndAction *and_action (Action *a, Action *b);

/**
 * Szablon do tworzenia akcji, które mają funktor (na przykład lambdę).
 */
template <typename Func> class FuncAction : public Action {
public:
        FuncAction (Func func) : func (func) {}
        virtual ~FuncAction () {}
        virtual bool run (const char *input) { return func (input); }

private:
        Func func;
};

template <typename Func> FuncAction<Func> *func (Func func) { return new FuncAction<Func> (func); }

#endif // IACTION_H
