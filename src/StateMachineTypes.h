/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef STATEMACHINETYPES_H
#define STATEMACHINETYPES_H

#include <etl/cstring.h>
#include <etl/queue_spsc_isr.h>
#include "collection/Queue.h"



static constexpr size_t MAX_STATES_NUM = 64;
static constexpr size_t ACTION_QUEUE_SIZE = 8;
static constexpr size_t EVENT_QUEUE_SIZE = 8;

using string = etl::string<64>;

/**
 * etl::queue_spsc_isr
 */
struct InterruptControl {

        static void lock ()
        {
#ifndef UNIT_TEST
                __disable_irq ();
#endif
        }

        static void unlock ()
        {
#ifndef UNIT_TEST
                __enable_irq ();
#endif
        }
};

template <typename EventT> struct StateMachineTypes {

        using EventType = EventT;
        // using EventQueue = etl::queue_spsc_isr<EventType, EVENT_QUEUE_SIZE, InterruptControl, etl::memory_model::MEMORY_MODEL_SMALL>;
        using EventQueue = Queue<EventType>;
};

#endif // STATEMACHINETYPES_H
