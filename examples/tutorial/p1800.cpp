/**
 * @file p1800.cpp
 * @brief
 *
 * @author Johan Vanslembrouck (johan.vanslembrouck@capgemini.com, johan.vanslembrouck@gmail.com)
 */

#include <functional>

#include "p1800.h"
#include "eventqueue.h"

using namespace corolib;

EventQueue eventQueue;
extern UseMode useMode;

/**
 * @brief async_op
 * 
 */
void async_op(std::function<void(int)>&& completionHandler)
{
    switch (useMode)
    {
    case USE_NONE:
        // Nothing to be done here: completionHandler should be called "manually" by the application
        break;
    case USE_EVENTQUEUE:
        eventQueue.push(std::move(completionHandler));
        break;
    case USE_THREAD:
    {
        std::thread thread1([completionHandler]() {
            print(PRI1, "async_op: thread1: completionHandler(10);\n");
            completionHandler(10);
            print(PRI1, "async_op: thread1: return;\n");
            });
        thread1.detach();
        break;
    }
    case USE_IMMEDIATE_COMPLETION:
        completionHandler(10);
        break;
    }
}

/**
 * @brief start_operation_impl simulates starting an asynchronous operation
 * 
 * @param op is a reference to an async_operation<int> object.
 */
void start_operation_impl(async_operation<int>& op)
{
    print(PRI1, "start_operation_impl()\n");
    async_op(
        [&op](int i)
        {
            print(PRI1, "completionHandler(): op->set_result(%d)\n", i);
            op.set_result_and_complete(i);
        });
}

async_operation<int> op;
   
async_task<int> coroutine1()
{
    print(PRI1, "coroutine1(): async_operation<int> op;\n");
    op.auto_reset(true);
    int v1 = 0;
    int v = 0;
    
    do
    {
        print(PRI1, "coroutine1(): int v1 = co_await op;\n");
        v1 = co_await op;
        v += v1;
    }
    while (v1 != 0);
    
    print(PRI1, "coroutine5(): co_return v + 1 = %d;\n", v + 1);
    co_return v + 1;
}
