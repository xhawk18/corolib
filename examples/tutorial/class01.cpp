/**
 * @file class01.cpp
 * @brief
 *
 *
 * @author Johan Vanslembrouck (johan.vanslembrouck@capgemini.com, johan.vanslembrouck@gmail.com)
 */
 
#include "class01.h"

#include <corolib/print.h>

async_operation<int> Class01::start_operation()
{
    int index = get_free_index();
    print(PRI1, "%p: Class01::start_operation(): index = %d\n", this, index);
    async_operation<int> ret{ this, index };
    start_operation_impl(index);
    return ret;
}


/**
 * @brief Class01::async_op
 * 
 * Completion of the asynchronous operation has to be simulated.
 * There are 4 cases:
 * 1) m_useMode == USE_NONE: The application (function main) has to call completionHandler "manually".
 * 2) m_useMode == USE_EVENTQUEUE: async_op places the completionHandler in an eventQueue.
 *    The application (function main) will run the event loop to call the completionHandler(s) in the eventQueue.
 * 3) m_useMode == USE_THREAD: async_op starts a detached thread that will
 *    call the eventHandler after a delay of 1000 ms. 
 * 4) m_useMode == USE_IMMEDIATE_COMPLETION: async_op calls the completionHandler immediately.
 *
 * @param idx
 */
void Class01::async_op(std::function<void(int)>&& completionHandler)
{
    switch (m_useMode)
    {
    case USE_NONE:
        // Nothing to be done here: eventHandler should be called "manually" by the application
        eventHandler = completionHandler;
        break;
    case USE_EVENTQUEUE:
        eventQueue.push(std::move(completionHandler));
        break;
    case USE_THREAD:
    {
        std::thread thread1([completionHandler]() {
            print(PRI1, "Class01::start_operation_impl(): thread1: std::this_thread::sleep_for(std::chrono::milliseconds(1000));\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            print(PRI1, "Class01::start_operation_impl(): thread1: this->eventHandler(10);\n");
            completionHandler(10);
            print(PRI1, "Class01::start_operation_impl(): thread1: return;\n");
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
 * @brief Class01::start_operation_impl simulates starting an asynchronous operation.
 *
 * @param idx
 */
void Class01::start_operation_impl(const int idx)
{
    print(PRI1, "%p: Class01::start_operation_impl(%d)\n", this, idx);
    async_op(
        [this, idx](int i)
        {
            print(PRI1, "Class01::eventHandler[%p, %d](%d)\n", this, idx, i);

            async_operation_base* om_async_operation = m_async_operations[idx];
            async_operation<int>* om_async_operation_t =
                dynamic_cast<async_operation<int>*>(om_async_operation);

            if (om_async_operation_t)
            {
                print(PRI1, "Class01::eventHandler[%p, %d](%d): om_async_operation_t->set_result(%d);\n", this, idx, i, i);
                om_async_operation_t->set_result(i);
                om_async_operation_t->completed();
            }
            else
            {
                // This can occur when the async_operation_base has gone out of scope.
                print(PRI1, "Class01::eventHandler[%p, %d](%d): Warning: om_async_operation_t == nullptr\n", this, idx, i);
            }
        });
}

EventQueue eventQueue;
