//Calls Outer (First.cpp), which calls Twice (Last.cpp, exported after Middle.cpp's turn): Middle.cpp is compiled
//again because of it.
#include "Jobs.h"
#include "Jobs.h"


int UseOuter(int x)
{
	return Outer(x) * 3;
}
