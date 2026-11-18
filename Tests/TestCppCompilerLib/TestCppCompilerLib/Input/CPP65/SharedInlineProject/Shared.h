//What is in an inline function is one thing in every unit that has it (CppParserTodo.txt question 5): its static
//locals, a lambda's closure, the lambda's static locals and its conversion to a function pointer.
#ifndef __SHARED_H__
#define __SHARED_H__

inline int Counter(void) { static int s = 0; return ++s; }
typedef int (*FTwice)(int);
inline FTwice Twice(void) { return [](int x) { return x * 2; }; }
inline int Ticks(void) { auto f = []() { static int k = 0; return ++k; }; return f(); }
template<class T> T Total(T t) { static T st = 0; st += t; return st; }

int CounterA(void);
int CounterB(void);
FTwice TwiceA(void);
FTwice TwiceB(void);
int TicksA(void);
int TicksB(void);
long TotalA(long l);
long TotalB(long l);

#endif // __SHARED_H__
