/** 
 *  Filename: p1002-no-coroutines.cpp
 *  Description: 
 *
 *  Tested with Visual Studio 2019.
 *
 *  Author: Johan Vanslembrouck (johan.vanslembrouck@altran.com, johan.vanslembrouck@gmail.com)
 *
 */

#include <corolib/print.h>
#include <corolib/auto_reset_event.h>
#include <corolib/async_task.h>

using namespace corolib;

int function5()
{
	print(PRI1, "function5(): int v = 1;\n");
	int v = 1;
	print(PRI1, "function5(): creturn v+1 = %d;\n", v+1);
	return v+1;
}

int function4()
{
	print(PRI1, "function4(): int a = function5();\n");
	int a = function5();
	print(PRI1, "function4(): int v = a;\n");
	int v = a;
	print(PRI1, "function4(): return v+1 = %d;\n", v+1);
	return v+1;
}

int function3()
{
	print(PRI1, "function3(): int a1 = function4();\n");
	int a1 = function4();
	print(PRI1, "function3(): int v1 = a1;\n");
	int v1 = a1;

	print();
	print(PRI1, "function3(): int a2 = function4();\n");
	int a2 = function4();
	print(PRI1, "function3(): int v2 = a2;\n");
	int v2 = a2;

	print(PRI1, "function3(): return v1+v2+1 = %d;\n", v1+v2+1);
	return v1+v2+1;
}

int function2()
{
	print(PRI1, "function2(): int a = function3();\n");
	int a = function3();
	print(PRI1, "function2(): int v = a;\n");
	int v = a;
	print(PRI1, "function2(): return v+1 = %d;\n", v+1);
	return v+1;
}

int function1()
{
	print(PRI1, "function1(): int a = function2();\n");
	int a = function2();
	print(PRI1, "function1(): int v = a;\n");
	int v = a;
	print(PRI1, "function1(): return v+1 = %d;\n", v+1);
	return v+1;
}

int main()
{
	print(PRI1, "main(): int a = function1();\n");
	int a = function1();
	print(PRI1, "main(): int v = a;\n");
	int v = a;
	print(PRI1, "main(): v = %d\n", v);
	print(PRI1, "main(): return 0;\n");
	return 0;
}
