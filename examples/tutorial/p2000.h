/**
 * @file p2000.h
 * @brief
 *
 * @author Johan Vanslembrouck (johan.vanslembrouck@capgemini.com, johan.vanslembrouck@gmail.com)
 */

#ifndef _P2000_H_
#define _P2000_H_

#include <optional>

#include <corolib/print.h>
#include <corolib/async_task.h>
#include <corolib/async_operation.h>

using namespace corolib;

enum UseMode
{
    USE_NONE,
    USE_EVENTQUEUE,
    USE_THREAD,
    USE_THREAD_QUEUE,
    USE_IMMEDIATE_COMPLETION
};

extern async_operation<std::optional<int>> op1;
void start_operation_impl(async_operation<std::optional<int>>& op);

void task1();
void task2();
void task3();

#endif
