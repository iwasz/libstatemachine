/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef STRINGQUEUE_IUM_H
#define STRINGQUEUE_IUM_H

#include "collection/Queue.h"

static constexpr uint8_t STRING_QUEUE_BUFFER_SIZE = 128;

using StringQueue = Queue<char[STRING_QUEUE_BUFFER_SIZE]>;

#endif // STRINGQUEUE_H
