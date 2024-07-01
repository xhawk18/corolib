/**
 * @file p1000_sa.cpp
 * @brief
 * 
 * @author Johan Vanslembrouck (johan.vanslembrouck@gmail.com)
 */

#include <thread>

#include "print.h"

#include "task_sa.h"
#include "class_sync.h"

int main()
{
    set_print_level(0x03);        // Use 0x03 to follow the flow in corolib

    Class obj;
    print("PRI1, main(): task a = obj.coroutine1();\n");
    task a = obj.coroutine1();

    print(PRI1, "main(): a.start();\n");
    a.start();

    print("PRI1, main(): int v = a.get_result();\n");
    int v = a.get_result();
    print("PRI1, main(): v = %d;\n", v);
	
    print("PRI1, main(): return 0;\n");
    return 0;
}
