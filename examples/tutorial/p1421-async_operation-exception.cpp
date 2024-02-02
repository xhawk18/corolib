/**
 * @file p1421-async_operation-exception.cpp
 * @brief
 *
 *
 * @author Johan Vanslembrouck (johan.vanslembrouck@capgemini.com, johan.vanslembrouck@gmail.com)
 */

#include <corolib/print.h>
#include <corolib/async_task.h>

using namespace corolib;

#include "p1420.h"

void completionflow(Class1420& obj1420)
{
    // Begin manual event completion
    print(PRI1, "completionflow(): std::this_thread::sleep_for(std::chrono::milliseconds(1000));\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    print(PRI1, "completionflow(): before object02.eventHandler(10);\n");
    obj1420.m_object02.eventHandler(10);
    print(PRI1, "completionflow(): after object02.eventHandler(10);\n");

    try {
        print(PRI1, "completionflow(): before object01.eventHandler(-1);\n");
        obj1420.m_object01.eventHandler(-1);
        print(PRI1, "completionflow(): after object01.eventHandler(-1);\n");
    }
    catch (...) {
        print(PRI1, "completionflow: caught exception after object01.eventHandler(-1);\n");
    }

    print(PRI1, "completionflow(): std::this_thread::sleep_for(std::chrono::milliseconds(1000));\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    print(PRI1, "completionflow(): before object01.eventHandler(10);\n");
    obj1420.m_object01.eventHandler(10);
    print(PRI1, "completionflow(): after object01.eventHandler(10);\n");

    print(PRI1, "completionflow(): before object02.eventHandler(10);\n");
    obj1420.m_object02.eventHandler(10);
    print(PRI1, "completionflow(): after object02.eventHandler(10);\n");

    print(PRI1, "completionflow(): std::this_thread::sleep_for(std::chrono::milliseconds(1000));\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // End manual event completion
}

int main()
{
    set_priority(0x01);        // Use 0x03 to follow the flow in corolib

    Class01 object01;
    Class01 object02;
    Class1420 obj1420{ object01, object02 };
    async_task<int> a = obj1420.coroutine1();

    print(PRI1, "main(): completionflow(obj1410);\n");
    completionflow(obj1420);

    print(PRI1, "main(): int v = a.get_result(false);\n");
    int v = a.get_result(false);
    print(PRI1, "main(): v = %d\n", v);

    print(PRI1, "main(): std::this_thread::sleep_for(std::chrono::milliseconds(1000));\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    print(PRI1, "main(): return 0;\n");
    return 0;
}
