//Late is exported after Named.cpp's turn, and main makes a square and a holder.
#include "Shapes.h"


int Late(int x)
{
	return x * 2;
}


int main()
{
	CSquare			cSquare;
	CGaugeHolder	cHolder;

	cSquare.miSize = 3;
	return SquareArea(&cSquare) + Named(0, 0, 0, 0, 0, 0, 0, 0, 0, &cHolder) + Held(&cHolder) + First();
}
