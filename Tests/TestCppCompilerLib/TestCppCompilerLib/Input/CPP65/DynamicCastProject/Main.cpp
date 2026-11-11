#include "Shapes.h"


int CountSquares(CShape* pcShape);
int SquareSides(void);


int main()
{
	CSquare		cSquare;
	CCircle		cCircle;
	CShape*		pcShape;

	pcShape = &cCircle;
	return CountSquares(&cSquare) + CountSquares(&cCircle) * 10 + SquareSides() * 100 + ((dynamic_cast<CCircle*>(pcShape) == &cCircle) ? 1000 : 0);
}
