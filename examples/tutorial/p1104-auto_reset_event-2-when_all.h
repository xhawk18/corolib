/**
 * @file p1104-auto_reset_event-2-when_all.h
 * @brief
 * Uses 2 auto_reset_event objects that will be resumed from main().
 * Coroutines coroutine4a and coroutine4b are co_awaited in coroutine3 using when_all.
 * 
 * @author Johan Vanslembrouck (johan.vanslembrouck@capgemini.com, johan.vanslembrouck@gmail.com)
 */

#ifndef _P1104_H_
#define _P1104_H_

#include <corolib/print.h>
#include <corolib/async_task.h>
#include <corolib/async_operation.h>
#include <corolib/auto_reset_event.h>
#include <corolib/when_all.h>

using namespace corolib;

extern auto_reset_event are1;
extern auto_reset_event are2;

class Class1104
{
public:
    async_task<int> coroutine4a()
    {
        print(PRI1, "coroutine4a(): co_await are1;\n");
        co_await are1;
        print(PRI1, "coroutine4a(): co_return 1;\n");
        co_return 1;
    }

    async_task<int> coroutine4b()
    {
        print(PRI1, "coroutine4b(): co_await are2;\n");
        co_await are2;
        print(PRI1, "coroutine4b(): co_return 1;\n");
        co_return 2;
    }

    async_task<int> coroutine3()
    {
        print(PRI1, "coroutine3(): async_task<int> a = coroutine4a();\n");
        async_task<int> a = coroutine4a();
        print(PRI1, "coroutine3(): async_task<int> b = coroutine4b();\n");
        async_task<int> b = coroutine4b();
        print(PRI1, "coroutine3(): when_all wa(a, b);\n");
        when_all wa(a, b);
        print(PRI1, "coroutine3(): co_await wa;\n");
        co_await wa;
        print(PRI1, "coroutine3(): int v1 = a.get_result();\n");
        int v1 = a.get_result();
        print(PRI1, "coroutine3(): int v2 = b.get_result();\n");
        int v2 = b.get_result();
        print(PRI1, "coroutine3(): co_return v+1 = %d;\n", v1 + v2 + 1);
        co_return v1 + v2 + 1;
    }

    async_task<int> coroutine2()
    {
        print(PRI1, "coroutine2(): int v = co_await coroutine3;\n");
        int v = co_await coroutine3();
        print(PRI1, "coroutine2(): co_return v+1 = %d;\n", v+1);
        co_return v+1;
    }

    async_task<int> coroutine1()
    {
        print(PRI1, "coroutine1(): int v = co_await coroutine2();\n");
        int v = co_await coroutine2();
        print(PRI1, "coroutine1(): co_return v+1 = %d;\n", v+1);
        co_return v+1;
    }
};

#endif
