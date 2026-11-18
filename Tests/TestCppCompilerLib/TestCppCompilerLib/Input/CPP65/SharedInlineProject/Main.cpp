#include "Shared.h"

//Each one's object is the same in A.cpp and B.cpp: the counts go on across them, and the function pointers are equal.
int main(void)
{
	CounterA();
	CounterB();
	int iCount = CounterA();
	TicksA();
	int iTicks = TicksB();
	TotalA(5L);
	long lTotal = TotalB(7L);
	return iCount * 1000 + iTicks * 100 + (int)lTotal * 2 + (TwiceA() == TwiceB()) + ((TwiceB()(3) == 6) ? 0 : 1000);
}
