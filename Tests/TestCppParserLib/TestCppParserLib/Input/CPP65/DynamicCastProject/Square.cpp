#include "Shapes.h"


//This unit writes CSquare's vtable and type info too; the linker keeps one of each.
int CountSquares(CShape* pcShape)
{
	return dynamic_cast<CSquare*>(pcShape) ? 1 : 0;
}


int SquareSides(void)
{
	CSquare		cSquare;
	CShape*		pcShape;

	pcShape = &cSquare;
	return dynamic_cast<CSquare*>(pcShape)->Sides();
}
