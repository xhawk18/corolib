/**
 * @file p1132-coroutines-async-callstack-1rmi-cs.cpp
 * @brief Variant of p1112-async-callstack-1rmi-cs.cpp.
 *
 * In this example a coroutine layer is added on top of the highest asynchronous layer, Layer03.
 * It shows that only a few simple modifications have to be made to Layer03.
 * 
 * This approach can be useful in case it is not possible or desirable to make modifications
 * to an existing asynchronous callstack, yet the application should be able to use coroutines.
 * 
 * @author Johan Vanslembrouck (johan.vanslembrouck@gmail.com)
 */

#include <stdio.h>
#include <stack>

#include <corolib/print.h>
#include <corolib/async_task.h>
#include <corolib/commservice.h>
#include <corolib/async_operation.h>

using namespace corolib;

#include "common.h"
#include "eventqueue.h"

class CallStack;

typedef std::function<void(CallStack*, int, int, int)>  lambda_cs_3int_t;
typedef std::function<void(CallStack*, int, int)>       lambda_cs_2int_t;
typedef std::function<void(CallStack*, int)>            lambda_cs_1int_t;
typedef std::function<void(CallStack*)>                 lambda_cs_t;

using namespace std;

struct StackElement
{
    StackElement(void* context_, void* lambda_)
        : context(context_)
        , lambda(lambda_)
    {
        printf("%p: StackElement::StackElement(): context = %p, lambda = %p\n", this, context, lambda);
    }

    ~StackElement()
    {
        printf("%p: StackElement:~StackElement(): context = %p, lambda = %p\n", this, context, lambda);
    }

    void* context;
    void* lambda;
};

/**
 * @brief Callstack allows building a stack of lambdas that is passed
 * along the layers when one layer calls the function of the lower layer.
 * When returning "up the call chain", every layer knows which lambda to use.
 *
 */
class CallStack
{
    stack<StackElement*> q;
public:

    void push(StackElement* val)
    {
        q.push(val);
    }

    void pop()
    {
        if (q.empty())
            printf("No elements\n");
        else
            q.pop();
    }

    StackElement* top()
    {
        return (q.empty()) ? nullptr : q.top();
    }

    StackElement* top_pop()
    {
        StackElement* val = (q.empty()) ? nullptr : q.top();
        q.pop();
        return val;
    }

    bool empty()
    {
        return (q.empty());
    }
};

class RemoteObject1
{
public:
    void sendc_op1(CallStack* callstack, int in1, int in2)
    {
        printf("RemoteObject1::sendc_op1(in1 = %d, in2 = %d)\n", in1, in2);

        StackElement* se = callstack->top();
        lambda_cs_3int_t* op = static_cast<lambda_cs_3int_t*>(se->lambda);

        eventQueue.push(
            [op, callstack, in1, in2]()
            {
                (*op)(callstack, 1, 2, in1 + in2);
                delete op;
            });
    }
};

RemoteObject1 remoteObj1;

/**
 * @brief Layer01 is the lowest level in the application stack
 * Lower layer: RemoteObject1
 * Upper layer: Layer02 (but not known by Layer01)
 *
 */
class Layer01
{
public:
    struct function1_cxt_t
    {
        int in1;
    };

    void function1(CallStack* callstack, int in1)
    {
        printf("Layer01::function1(in1 = %d)\n", in1);
        lambda_cs_3int_t* op = new lambda_cs_3int_t(
            [this](CallStack* callstack, int out1, int out2, int ret1)
            {
                this->function1_cb(callstack, out1, out2, ret1);
            });
        void* ctxt = new function1_cxt_t{ in1 };
        StackElement* se = new StackElement(ctxt, op);
        callstack->push(se);

        remoteObj1.sendc_op1(callstack, in1, in1);
    }

    void function1_cb(CallStack* callstack, int out1, int out2, int ret1)
    {
        printf("Layer01::function1_cb(out1 = %d, out2 = %d, ret1 = %d)\n", out1, out2, ret1);

        StackElement* se = callstack->top_pop();
        function1_cxt_t* ctxt = static_cast<function1_cxt_t*>(se->context);

        StackElement* se2 = callstack->top();
        lambda_cs_3int_t* op = static_cast<lambda_cs_3int_t*>(se2->lambda);
        (*op)(callstack, out1, out2, ctxt->in1 + ret1);
        delete op;
        delete ctxt;
        delete se;
    }
};

Layer01 layer01;

/**
 * @brief Layer02 is the middle layer in the application stack
 * Lower layer: Layer01
 * Upper layer: Layer03 (but not known by Layer02)
 */
class Layer02
{
public:
    struct function1_cxt_t
    {
        int in1;
    };

    void function1(CallStack* callstack, int in1)
    {
        printf("Layer02::function1(in1 = %d)\n", in1);

        lambda_cs_3int_t* op = new lambda_cs_3int_t(
            [this](CallStack* callstack, int out1, int out2, int ret1)
            {
                this->function1_cb(callstack, out1, out2, ret1);
            });
        void* ctxt = new function1_cxt_t{ in1 };
        StackElement* se = new StackElement(ctxt, op);
        callstack->push(se);

        layer01.function1(callstack, in1);
    }

    void function1_cb(CallStack* callstack, int out1, int out2, int ret1)
    {
        printf("Layer02::function1_cb(out1 = %d, out2 = %d, ret1 = %d)\n", out1, out2, ret1);
        StackElement* se = callstack->top_pop();
        function1_cxt_t* ctxt = static_cast<function1_cxt_t*>(se->context);

        StackElement* se2 = callstack->top();
        lambda_cs_2int_t* op = static_cast<lambda_cs_2int_t*>(se2->lambda);

        (*op)(callstack, 1, ctxt->in1 + out2 + ret1);
        delete op;
        delete ctxt;
        delete se;
    }
};

Layer02 layer02;

/**
 * @brief Layer03 is the upper layer in the application stack
 * Lower layer: Layer02
 * Upper layer: application (but not known by Layer03)
 *
 */
class Layer03
{
public:
    // Synchronous function:
    // int function1(int in1);
  
    void function1(async_operation<int>& op1, int in1)
    {
        printf("Layer03::function1(in1 = %d)\n", in1);
        CallStack* callstack = new CallStack;

        lambda_cs_2int_t* p = new lambda_cs_2int_t(
            [this, in1, &op1](CallStack* callstack, int out1, int ret1)
            {
                op1.set_result(in1 + out1 + ret1);
                op1.completed();
                delete callstack;
            });

        void* ctxt = nullptr;
        StackElement* se = new StackElement(ctxt, p);
        callstack->push(se);

        layer02.function1(callstack, in1);
    }

    // Synchronous function:
    // int function2(int in1);

    void function2(async_operation<int>& op1, int in1)
    {
        printf("Layer03::function2(in1 = %d)\n", in1);
        CallStack* callstack = new CallStack;

        lambda_cs_2int_t* p = new lambda_cs_2int_t(
            [this, in1, &op1](CallStack* callstack, int out1, int ret1)
            {
                op1.set_result(in1 + out1 + ret1);
                op1.completed();
                delete callstack;
            });

        void* ctxt = nullptr;
        StackElement* se = new StackElement(ctxt, p);
        callstack->push(se);

        layer02.function1(callstack, in1);
    }
};

Layer03 layer03;

class Layer03Co : public CommService
{
public:
    async_task<int> coroutine1(int in1)
    {
        printf("Layer03::coroutine1(in1 = %d)\n", in1);
        int index = get_free_index();
        async_operation<int> op1{ this, index };
        layer03.function1(op1, in1);
        printf("Layer03::coroutine1(): int ret1 = co_await op1\n");
        int ret1 = co_await op1;
        printf("Layer03::coroutine1(): ret1 = %d\n", ret1);
        co_return ret1;
    }

    async_task<int> coroutine2(int in1)
    {
        printf("Layer03::coroutine2(in1 = %d)\n", in1);
        int index = get_free_index();
        async_operation<int> op1{ this, index };
        layer03.function2(op1, in1);
        printf("Layer03::coroutine2(): int ret1 = co_await op1\n");
        int ret1 = co_await op1;
        printf("Layer03::coroutine2(): ret1 = %d\n", ret1);
        co_return ret1;
    }
};

Layer03Co layer03co;

EventQueue eventQueue;

int main() {
    printf("main()\n");
    async_task<int> t1 = layer03co.coroutine1(2);
    async_task<int> t2 = layer03co.coroutine2(3);
    printf("main(): eventQueue.run();\n");
    eventQueue.run();
    int ret1 = t1.get_result();
    printf("main(): ret1 = %d\n", ret1);
    int ret2 = t2.get_result();
    printf("main(): ret2 = %d\n", ret2);
    return 0;
}
