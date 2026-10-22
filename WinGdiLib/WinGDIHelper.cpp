#include "WinGDIHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CopyRectangleToGDIRect(RECT* psRect, CInt32Rect* pcRectangle)
{
	psRect->left = pcRectangle->msTopLeft.x;
	psRect->top = pcRectangle->msTopLeft.y;
	psRect->right = pcRectangle->msBottomRight.x;
	psRect->bottom = pcRectangle->msBottomRight.y;
}

