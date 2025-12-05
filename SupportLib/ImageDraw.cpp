#include "StandardLib/ChannelsCopier.h"
#include "ImageAccessorCreator.h"
#include "ImageDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::Init(CImage* pcImage)
{
	CImageColourRGB	cColour;

	mpcImage = pcImage;
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
	mpcImage = NULL;
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

	mpcAccessor = CImageAccessorCreator::Create(mpcImage, pcColour);
	mpcAccessor->MakeColour(&msColour, pcColour);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::DrawPixel(int x, int y)
{
	mpcAccessor->SafeSet(x, y, &msColour);
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
void CImageDraw::DrawBox(int iLeft, int iTop, int iRight, int iBottom, bool bFilled)
{
	int		x;
	int		y;

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
	DrawBox(0, 0, mpcImage->GetWidth(), mpcImage->GetHeight(), bFilled);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::DrawLine(int x0, int y0, int x1, int y1)
{
	int dx;
	int dy;
	int sx;
	int sy;
	int err;

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

		int e2 = 2 * err;
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
void CImageDraw::DrawHorizontalLine(int x0, int x1, int y)
{
	int	x;

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
void CImageDraw::DrawVerticalLine(int x, int y0, int y1)
{
	int	y;

	if (y0 > y1)
	{
		SwapInt32(&y0, &y1);
	}

	for (y = y0; y <= y1; y++)
	{
		mpcAccessor->SafeSet(x, y, &msColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::DrawImage(int x, int y, CImage* pcSource)
{
	int					i;
	int					iSourceWidth;
	int					iSourceHeight;
	int					iDestWidth;
	int					iDestHeight;
	CChannelsCopier		cCopy;

	iSourceWidth = pcSource->GetWidth();
	iSourceHeight = pcSource->GetHeight();
	iDestWidth = mpcImage->GetWidth();
	iDestHeight = mpcImage->GetHeight();

	cCopy.Init(pcSource->GetChannels(), mpcImage->GetChannels());
	for (i = 0; i < iSourceHeight; i++)
	{
		cCopy.Copy(i * iSourceWidth, (y + i) * iDestWidth + x, iSourceWidth);
	}
	cCopy.Kill();
}

