#include "ImageAccessorCreator.h"
#include "ImageDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::Init(Ptr<CImage> pImage)
{
	CImageColourRGB	cColour;

	mpImage = pImage;
	mpcAccessor = NULL;
	
	cColour.Init(1.0f, 1.0f, 1.0f);
	SetColour(&cColour);
	cColour.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::Kill(void)
{
	if (mpcAccessor)
	{
		mpcAccessor->Kill();
		mpcAccessor = NULL;
	}
	mpImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::SetColour(CImageColour* pcColour)
{
	if (mpcAccessor)
	{
		mpcAccessor->Kill();
		mpcAccessor = NULL;
	}

	mpcAccessor = CImageAccessorCreator::Create(mpImage, pcColour);
	mpcAccessor->MakeColour(&msColour, pcColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::DrawBox(CRectangle* pcRetangle, bool bFilled)
{
	DrawBox(pcRetangle->miLeft, pcRetangle->miTop, pcRetangle->miRight, pcRetangle->miBottom, bFilled);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::DrawBox(int32 iLeft, int32 iTop, int32 iRight, int32 iBottom, bool bFilled)
{
	int32		x;
	int32		y;

	if (bFilled)
	{
		for (y = iTop; y <= iBottom; y++)
		{
			for (x = iLeft; x <= iRight; x++)
			{
				mpcAccessor->SafeSet(x, y, &msColour);
			}
		}
	}
	else
	{
		for (x = iLeft; x <= iRight; x++)
		{
			mpcAccessor->SafeSet(x, iTop, &msColour);
			mpcAccessor->SafeSet(x, iBottom, &msColour);
		}

		for (y = iTop; y <= iBottom; y++)
		{
			mpcAccessor->SafeSet(iLeft, y, &msColour);
			mpcAccessor->SafeSet(iRight, y, &msColour);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::DrawBox(bool bFilled)
{
	DrawBox(0, 0, mpImage->GetWidth(), mpImage->GetHeight(), bFilled);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::DrawLine(int32 x0, int32 y0, int32 x1, int32 y1)
{
	int32 dx;
	int32 dy;
	int32 sx;
	int32 sy;
	int32 err;

	dx = IntAbs(x1 - x0);
	dy = IntAbs(y1 - y0);
	sx = (x0 < x1) ? 1 : -1;
	sy = (y0 < y1) ? 1 : -1;
	err = dx - dy;

	while (true)
	{
		mpcAccessor->SafeSet(x0, y0, &msColour);

		if ((x0 == x1) && (y0 == y1))
		{
			break;
		}

		int32 e2 = 2 * err;
		if (e2 > -dy) 
		{
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx)
		{
			err += dx;
			y0 += sy;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::DrawHorizontalLine(int32 x0, int32 x1, int32 y)
{
	int32	x;

	if (x0 > x1)
	{
		SwapInt32(&x0, &x1);
	}

	for (x = x0; x <= x1; x++)
	{
		mpcAccessor->SafeSet(x, y, &msColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::DrawVerticalLine(int32 x, int32 y0, int32 y1)
{
	int32	y;

	if (y0 > y1)
	{
		SwapInt32(&y0, &y1);
	}

	for (y = y0; y <= y1; y++)
	{
		mpcAccessor->SafeSet(x, y, &msColour);
	}
}

