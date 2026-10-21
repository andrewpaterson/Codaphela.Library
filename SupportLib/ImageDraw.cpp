#include "BaseLib/Logger.h"
#include "ImageAccessorCreator.h"
#include "ImageDraw.h"
#include "ImageCelBlitterCache.h"


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

	//A colour without opacity is opaque and one without red, green or blue has none of them.
	mfOpacity = pcColour->GetValue(IMAGE_OPACITY);
	if (IsNotANumber(&mfOpacity))
	{
		mfOpacity = 1.0f;
	}
	mfRed = pcColour->GetValue(IMAGE_DIFFUSE_RED);
	if (IsNotANumber(&mfRed))
	{
		mfRed = 0.0f;
	}
	mfGreen = pcColour->GetValue(IMAGE_DIFFUSE_GREEN);
	if (IsNotANumber(&mfGreen))
	{
		mfGreen = 0.0f;
	}
	mfBlue = pcColour->GetValue(IMAGE_DIFFUSE_BLUE);
	if (IsNotANumber(&mfBlue))
	{
		mfBlue = 0.0f;
	}
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
	DrawBox(pcRetangle->msTopLeft.x, pcRetangle->msTopLeft.y, pcRetangle->msBottomRight.x, pcRetangle->msBottomRight.y, bFilled);
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
// Draws one half of a filled circle iDiameter across.  The half's box has
// its top left at x, y and is (iDiameter + 1) / 2 wide and iDiameter high
// for the left and right halves (the other way round for the top and
// bottom halves).  The circle's centre is on the box's flat side so a half
// drawn against a bar iDiameter thick joins it without a gap.
//
// The edge is anti-aliased: a pixel's opacity is the colour's opacity
// times the part of the pixel's 16 x 16 samples inside the circle.  Its
// colour is not multiplied by its opacity.  The image must have opacity,
// red, green and blue channels.  Pixels are written, not blended, and
// pixels outside the circle are left alone, so the image should start
// clear.  The coverage is worked out with integers so every build draws
// the same pixels.
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawFilledHalfCircle(int x, int y, int iDiameter, EImageDrawHalf eHalf)
{
	CImageColourARGB	cColour;
	CImageAccessor*		pcAccessor;
	SImageColour		sColour;
	int					iHalfWidth;
	int					iWidth;
	int					iHeight;
	int64				iCentreX;  //In 32nds of a pixel from the box's top left, so samples sit in the middle of their 16ths.
	int64				iCentreY;
	int64				iRadiusSquared;
	int64				dx;
	int64				dy;
	int					px;
	int					py;
	int					sx;
	int					sy;
	int					iCoverage;
	float32				fOpacity;

	if (iDiameter <= 0)
	{
		return true;
	}
	if (!mpcImage->HasChannel(IMAGE_OPACITY))
	{
		return gcLogger.Error2(__METHOD__, " Cannot draw an anti-aliased half circle on an image without an opacity channel.", NULL);
	}

	iHalfWidth = (iDiameter + 1) / 2;
	switch (eHalf)
	{
	case IDH_Left:
		iWidth = iHalfWidth;
		iHeight = iDiameter;
		iCentreX = iHalfWidth * 32;
		iCentreY = iDiameter * 16;
		break;
	case IDH_Right:
		iWidth = iHalfWidth;
		iHeight = iDiameter;
		iCentreX = 0;
		iCentreY = iDiameter * 16;
		break;
	case IDH_Top:
		iWidth = iDiameter;
		iHeight = iHalfWidth;
		iCentreX = iDiameter * 16;
		iCentreY = iHalfWidth * 32;
		break;
	case IDH_Bottom:
		iWidth = iDiameter;
		iHeight = iHalfWidth;
		iCentreX = iDiameter * 16;
		iCentreY = 0;
		break;
	default:
		return gcLogger.Error2(__METHOD__, " Unknown half of a circle.", NULL);
	}
	iRadiusSquared = (int64)iDiameter * 16 * iDiameter * 16;

	cColour.Init(mfOpacity, mfRed, mfGreen, mfBlue);
	pcAccessor = CImageAccessorCreator::Create(mpcImage, &cColour);
	for (py = 0; py < iHeight; py++)
	{
		for (px = 0; px < iWidth; px++)
		{
			iCoverage = 0;
			for (sy = 0; sy < 16; sy++)
			{
				dy = py * 32 + sy * 2 + 1 - iCentreY;
				for (sx = 0; sx < 16; sx++)
				{
					dx = px * 32 + sx * 2 + 1 - iCentreX;
					if (dx * dx + dy * dy <= iRadiusSquared)
					{
						iCoverage++;
					}
				}
			}

			if (iCoverage > 0)
			{
				//Dividing by 256 is exact so the opacity only depends on the one multiply.
				fOpacity = (mfOpacity * (float32)iCoverage) / 256.0f;
				cColour.Init(fOpacity, mfRed, mfGreen, mfBlue);
				pcAccessor->MakeColour(&sColour, &cColour);
				pcAccessor->SafeSet(x + px, y + py, &sColour);
			}
		}
	}
	pcAccessor->Kill();
	cColour.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawCel(int x, int y, Ptr<CImageCel> pCel, CRectangle* pcClip)
{
	Ptr<CImageBlitter>	pBlitter;

	//The destination image keeps the blitters that draw onto it.  The blitter clips to the destination image and to pcClip if it is not NULL.
	pBlitter = mpcImage->GetBlitterCache()->CreateImageBlitter(pCel);
	if (pBlitter.IsNull())
	{
		gcLogger.Error2(__METHOD__, " Cannot draw cel, the source and destination image formats do not match.", NULL);
		return false;
	}

	return pBlitter->Blit(x, y, pcClip);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawCel(int x, int y, int iWidth, int iHeight, Ptr<CImageCel> pCel, CRectangle* pcClip)
{
	Ptr<CImageBlitter>	pBlitter;

	//The whole cel is stretched or shrunk to iWidth by iHeight.
	pBlitter = mpcImage->GetBlitterCache()->CreateSizingImageBlitter(pCel);
	if (pBlitter.IsNull())
	{
		gcLogger.Error2(__METHOD__, " Cannot draw cel, the source and destination image formats do not match.", NULL);
		return false;
	}

	return pBlitter->Blit(x, y, iWidth, iHeight, pcClip);
}

