#include "Counter.h"


int Thrice(int i);


//Called through a pointer that may change, so Thrice itself is needed: it can't be inlined here.
int (*volatile gpfThrice)(int) = Thrice;


//0, 1, 3, 7; and 3 * 7.
int main()
{
	CCounter	cCounter;

	cCounter.Init();
	cCounter.Next();
	cCounter.Next();
	return gpfThrice(cCounter.Next());
}

