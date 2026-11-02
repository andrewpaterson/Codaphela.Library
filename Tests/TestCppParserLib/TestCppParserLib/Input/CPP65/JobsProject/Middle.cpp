//Calls Outer (First.cpp), which is only exported when First.cpp is compiled again: Middle.cpp is compiled again
//because of it.
#include "Jobs.h"
#include "Jobs.h"


int UseOuter(int x)
{
	return Outer(x) * 3;
}
