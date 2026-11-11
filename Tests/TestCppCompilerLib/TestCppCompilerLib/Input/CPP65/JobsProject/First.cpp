//Outer calls Twice (Last.cpp), so First.cpp is compiled again to inline it.
#include "Jobs.h"
#include "Jobs.h"


int Outer(int x)
{
	return Twice(x) + 1;
}
