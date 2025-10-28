#include "WinGDIHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CopyRectangleToGDIRect(RECT* psRect, CRectangle* pcRectangle)
{
	psRect->left = pcRectangle->miLeft;
	psRect->top = pcRectangle->miTop;
	psRect->right = pcRectangle->miRight;
	psRect->bottom = pcRectangle->miBottom;
}

