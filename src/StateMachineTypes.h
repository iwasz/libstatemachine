/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#pragma once
#include "Hal.h"
#include <etl/cstring.h>
#include <etl/deque.h>

static constexpr size_t MAX_STATES_NUM = 64;
static constexpr size_t ACTION_QUEUE_SIZE = 8;
static constexpr size_t EVENT_QUEUE_SIZE = 16;
static constexpr size_t DEFERRED_EVENT_QUEUE_SIZE = 4;

using string = etl::string<64>;

#ifndef LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE
#define LIB_STATE_MACHINE_DEFAULT_EVENT_TYPE string
#endif

template <typename EventT> struct StateMachineTypes {

        using EventType = EventT;
        // using EventQueue = etl::queue_spsc_isr<EventType, EVENT_QUEUE_SIZE, InterruptControl, etl::memory_model::MEMORY_MODEL_SMALL>;
        using EventQueue = etl::deque<EventType, EVENT_QUEUE_SIZE>;
        // using EventQueue = Queue<EventType>;
};
