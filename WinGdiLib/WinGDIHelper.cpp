#include "WinGDIHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CopyRectangleToGDIRect(RECT* psRect, CRectangle* pcRectangle)
{
	psRect->left = pcRectangle->msTopLeft.x;
	psRect->top = pcRectangle->msTopLeft.y;
	psRect->right = pcRectangle->msBottomRight.x;
	psRect->bottom = pcRectangle->msBottomRight.y;
}

