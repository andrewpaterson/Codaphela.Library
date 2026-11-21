//Names every class and needs two whole (CSquare, with its base, for the call; the holder's instance for Count).
//It calls Late (Last.cpp, exported after its turn), so it is compiled a second time; and it writes out the
//instance CGaugeHolder is the name of, which is warned about once.
#include "Shapes.h"


int SquareArea(CSquare* pcSquare)
{
	return pcSquare->Area();
}


int Named(CTriangle* pcTriangle, CCircle* pcCircle, CHexagon* pcHexagon, COctagon* pcOctagon, CCube* pcCube, CPrism* pcPrism, CBall* pcBall, CTile* pcTile, CCoin* pcCoin, CGaugeHolder* pcHolder)
{
	return Late((pcTriangle ? 1 : 0) + (pcCircle ? 1 : 0) + (pcHexagon ? 1 : 0) + (pcOctagon ? 1 : 0) + (pcCube ? 1 : 0) + (pcPrism ? 1 : 0) + (pcBall ? 1 : 0) + (pcTile ? 1 : 0) + (pcCoin ? 1 : 0) + (pcHolder ? 1 : 0));
}


int Held(CGaugeHolder* pcHolder)
{
	CHolderTemplate<CGauge>*	pcInstance;

	pcInstance = pcHolder;
	return pcInstance->Count();
}
