//Outer calls Twice (Last.cpp), so it can't be exported until First.cpp is compiled again with Twice inlined.
#include "Jobs.h"
#include "Jobs.h"


int Outer(int x)
{
	return Twice(x) + 1;
}
