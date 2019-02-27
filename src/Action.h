#ifndef ACTION_IACTION_H
#define ACTION_IACTION_H

//#include "StringQueue.h"

/**
 * @brief Baza dla wszystkich akcji.
 */
class Action /*: public Operation*/ {
public:
        Action (/*Action *next = nullptr*/) /*: next (next)*/ {}
        virtual ~Action () {}

        /**
         * @brief Uruchamia akcję.
         * @return Czy akcja zakończyła swoje działanie, czy nie (np. delay).
         */
        virtual bool run (const char *input) = 0;

        //        Action *getNext () { return next; }
        //        void setNext (Action *n);

        //        Action *and_action (Action *action);

//private:
        //        Action *next;
};


#ifdef UNIT_TEST
class TestAction : public Action {
public:


};
#endif

#if 0
/**
 * @brief Wykonuje dwie akcje po kolei.
 */
class AndAction : public Action {
public:

        AndAction (Action *a, Action *b) : a (a), b (b), current (nullptr) {}
        virtual ~AndAction () {}
        bool run (const char *input);

private:
        Action *a;
        Action *b;
        Action *current;
};

extern AndAction *and_action (Action *a, Action *b);


/**
 * Szablon do tworzenia akcji, które mają funktor (na przykład lambdę).
 */
template <typename Func>
class FuncAction : public Action {
public:

        FuncAction (Func func) : func (func) {}
        virtual ~FuncAction () {}
        virtual bool run (const char *input) { return func (input); }

private:

        Func func;
};

template <typename Func>
FuncAction <Func> *func (Func func) { return new FuncAction <Func> (func); }
#endif

#endif // IACTION_H
