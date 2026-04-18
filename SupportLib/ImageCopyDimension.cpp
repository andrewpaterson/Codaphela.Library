#include "ImageCopyDimension.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SImageCopy::Init(int iDestX, int iDestY, int iSourceX1, int iSourceY1, int iSourceX2, int iSourceY2, int iDestWidth, int iDestHeight)
{
	if (iDestY < 0)
	{
		iSourceY1 += -iDestY;
		iDestY = 0;
	}
	if (iDestX < 0)
	{
		iSourceX1 += -iDestX;
		iDestX = 0;
	}

	if ((iSourceX1 >= iSourceX2) || (iSourceY1 >= iSourceY2))
	{
		bValid = false;
		return;
	}

	if (((iSourceX2 - iSourceX1) + iDestX) >= iDestWidth)
	{
		iSourceX2 = iSourceX1 + iDestWidth - iDestX;
	}
	if (((iSourceY2 - iSourceY1) + iDestY) >= iDestHeight)
	{
		iSourceY2 = iSourceY1 + iDestHeight - iDestY;
	}

	if ((iSourceX2 <= 0) || (iSourceY2 <= 0))
	{
		bValid = false;
		return;
	}

	uiDestX = iDestX;
	uiDestY = iDestY;
	uiSourceX1 = iSourceX1;
	uiSourceY1 = iSourceY1;
	uiSourceX2 = iSourceX2;
	uiSourceY2 = iSourceY2;
	bValid = true;
}

