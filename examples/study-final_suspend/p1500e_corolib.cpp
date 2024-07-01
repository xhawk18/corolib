/**
 * @file p1500e_corolib.cpp
 * @brief
 * 
 * @author Johan Vanslembrouck (johan.vanslembrouck@gmail.com)
 */

#include <thread>

#include <corolib/async_task.h>
#include <corolib/tracker.h>
#include <corolib/print.h>

using namespace corolib;

class Class
{
public:
    async_task<int> coroutine4()
    {
        print(PRI1, "coroutine4(): co_return 1;\n");
        co_return 1;
    }

    async_task<int> coroutine3()
    {
        print(PRI1, "coroutine3(): int v = co_await coroutine4();\n");
        int v = co_await coroutine4();
        print(PRI1, "coroutine3(): co_return %d;\n", v + 1);
        co_return v + 1;
    }

    async_task<int> coroutine2()
    {
        print(PRI1, "coroutine2(): int v = co_await coroutine3();\n");
        int v = co_await coroutine3();
        print(PRI1, "coroutine2(): co_return %d;\n", v + 1);
        co_return v + 1;
    }

    async_task<int> coroutine1()
    {
        print(PRI1, "coroutine1(): int v = co_await coroutine2();\n");
        int v = co_await coroutine2();
        print(PRI1, "coroutine1(): co_return %d;\n", v + 1);
        co_return v + 1;
    }
};

int main()
{
    set_print_level(0x01);        // Use 0x03 to follow the flow in corolib

    Class obj;
    print(PRI1, "main(): task a = obj.coroutine1();\n");
    async_task<int> a = obj.coroutine1();
	
    print(PRI1, "main(): int v = a.get_result();\n");
    int v = a.get_result();
    print(PRI1, "main(): v = %d;\n", v);
	
    print(PRI1, "main(): return 0;\n");
    return 0;
}
