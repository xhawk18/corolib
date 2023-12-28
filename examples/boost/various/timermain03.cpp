/**
 * @file timermain03.cpp
 * @brief
 * 
 * @author Johan Vanslembrouck (johan.vanslembrouck@capgemini.com, johan.vanslembrouck@gmail.com)
 */

#include "timer03.h"

int main(int argc, char* argv[])
{
    set_priority(0x01);
    resume_multiple_coroutines(true);

    boost::asio::io_context ioContext;

    print(PRI1, "main: Timer02 c1(ioContext);\n");
    Timer03 c1(ioContext);

    print(PRI1, "main: async_task<int> t1 = c1.mainTask();\n");
    async_task<int> t1 = c1.mainTask();

    print(PRI1, "main: before ioContext.run();\n");
    ioContext.run();
    print(PRI1, "main: after ioContext.run();\n");

    print(PRI1, "main: int v = t1.get_result();\n");
    int v = t1.get_result();

    print(PRI1, "main: v = %d;\n", v);

    print(PRI1, "main: return 0;\n");
    return 0;
}
