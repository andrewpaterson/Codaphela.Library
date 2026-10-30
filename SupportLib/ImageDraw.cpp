#include "BaseLib/Logger.h"
#include "ImageAccessorCreator.h"
#include "BaseLib/FixedPoint.h"
#include "ColourBlend.h"
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

	mcRasteriser.Init();
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
	mcRasteriser.Kill();
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
void CImageDraw::DrawBox(CInt32Rect* pcRetangle, bool bFilled)
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
		SwapInt(&x0, &x1);
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
		SwapInt(&y0, &y1);
	}

	for (y = y0; y <= y1; y++)
	{
		mpcAccessor->SafeSet(x, y, &msColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Draws the part of a circle iDiameter across between two angles.  The
// whole circle fits in the square with its top left at x, y and sides
// iDiameter long, so its centre is on a pixel corner when iDiameter is even
// and in a pixel's middle when it is odd.  Angles are in degrees, 0 to the
// right and 90 up, and the part drawn goes anticlockwise from iStartAngle
// to iEndAngle.  90 to 180 is the top left quarter and 90 to 270 the left
// half.  An end angle 360 or more past the start draws the whole circle.
// Only the image's pixels are drawn so a circle can be clipped by the
// image's edges.  If bFilled is false only the curved edge is drawn, one
// pixel thick; the straight sides are left open.
//
// The edge is anti-aliased by the exact area of each pixel inside the part
// of the circle (or the one pixel thick ring when not filled); see
// BlendCoverage.
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawSemicircle(int x, int y, int iDiameter, int iStartAngle, int iEndAngle, bool bFilled)
{
	int			iSweep;
	float64		fRadius;
	float64		fInnerRadius;

	if (iDiameter <= 0)
	{
		return true;
	}

	iSweep = ((iEndAngle - iStartAngle) % 360 + 360) % 360;
	if ((iEndAngle - iStartAngle) >= 360)
	{
		iSweep = 360;
	}
	else if (iSweep == 0)
	{
		return true;
	}

	fRadius = iDiameter / 2.0;
	fInnerRadius = 0;
	if (!bFilled)
	{
		fInnerRadius = fRadius - 1;
	}
	return DrawRing(x + fRadius, y + fRadius, fRadius, fInnerRadius, iStartAngle, iStartAngle + iSweep);
}


//////////////////////////////////////////////////////////////////////////
//
// Fills pcPath, in pixels from the image's top left with y down.
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawPath(CVectorPath* pcPath)
{
	SFloat64Vec2	sOrigin;

	sOrigin.Init(0, 0);
	return DrawPath(pcPath, &sOrigin, 1, false);
}


//////////////////////////////////////////////////////////////////////////
//
// Fills pcPath placed on the image by an origin and a scale; pixel x is
// (x - origin) * scale and pixel y is (origin - y) * scale when y is up or
// (y - origin) * scale when it is down.
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawPath(CVectorPath* pcPath, SFloat64Vec2* psOrigin, float64 fPixelsPerUnit, bool bYUp)
{
	int		iLeft;
	int		iTop;
	int		iRight;
	int		iBottom;

	if (!mpcImage->HasChannel(IMAGE_OPACITY))
	{
		return gcLogger.Error2(__METHOD__, " Cannot draw an anti-aliased shape on an image without an opacity channel.", NULL);
	}

	pcPath->CalculateBounds();
	mcRasteriser.SetTransform(psOrigin, fPixelsPerUnit, bYUp);
	if (!mcRasteriser.GetPixelBounds(pcPath, &iLeft, &iTop, &iRight, &iBottom))
	{
		return true;
	}
	if (iLeft < 0)
	{
		iLeft = 0;
	}
	if (iTop < 0)
	{
		iTop = 0;
	}
	if (iRight > (int)mpcImage->GetWidth())
	{
		iRight = (int)mpcImage->GetWidth();
	}
	if (iBottom > (int)mpcImage->GetHeight())
	{
		iBottom = (int)mpcImage->GetHeight();
	}
	if ((iRight <= iLeft) || (iBottom <= iTop))
	{
		return true;
	}

	if (!mcRasteriser.Rasterise(pcPath, iLeft, iTop, iRight - iLeft, iBottom - iTop))
	{
		return gcLogger.Error2(__METHOD__, " Could not rasterise path.", NULL);
	}
	return BlendCoverage();
}


//////////////////////////////////////////////////////////////////////////
//
// Fills pcPath made with y up around 0, 0 with that point at pixel x, y.
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawCentredPath(CVectorPath* pcPath, float64 x, float64 y)
{
	SFloat64Vec2	sOrigin;

	sOrigin.Init(-x, y);
	return DrawPath(pcPath, &sOrigin, 1, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawCircle(float64 x, float64 y, float64 fRadius)
{
	return DrawSector(x, y, fRadius, 0, 360);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawSector(float64 x, float64 y, float64 fRadius, float64 fStartAngle, float64 fEndAngle)
{
	return DrawRing(x, y, fRadius, 0, fStartAngle, fEndAngle);
}


//////////////////////////////////////////////////////////////////////////
//
// The part of a ring between two radii from the start angle anticlockwise
// to the end angle.  No inner radius is a sector of a circle and an end 360
// or more past the start is all the way round.
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawRing(float64 x, float64 y, float64 fOuterRadius, float64 fInnerRadius, float64 fStartAngle, float64 fEndAngle)
{
	CVectorPath		cPath;
	SFloat64Vec2	sCentre;
	bool			bResult;

	cPath.Init();
	sCentre.Init(0, 0);
	AddVectorRing(&cPath, &sCentre, fOuterRadius, fInnerRadius, fStartAngle, fEndAngle);
	bResult = DrawCentredPath(&cPath, x, y);
	cPath.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A line fWidth wide from x0, y0 to x1, y1 with round ends.
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawStroke(float64 x0, float64 y0, float64 x1, float64 y1, float64 fWidth)
{
	CVectorPath		cPath;
	SFloat64Vec2	sStart;
	SFloat64Vec2	sEnd;
	bool			bResult;

	cPath.Init();
	sStart.Init(x0, y0);
	sEnd.Init(x1, y1);
	AddVectorStroke(&cPath, &sStart, &sEnd, fWidth / 2);
	bResult = DrawPath(&cPath);
	cPath.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawPolygon(SFloat64Vec2* asPoints, size uiNumPoints)
{
	CVectorPath		cPath;
	bool			bResult;

	cPath.Init();
	cPath.AddPolygon(asPoints, uiNumPoints, false);
	bResult = DrawPath(&cPath);
	cPath.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawRectangle(float64 fLeft, float64 fTop, float64 fRight, float64 fBottom)
{
	return DrawRoundedRectangle(fLeft, fTop, fRight, fBottom, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// A rectangle with quarter circle corners fRadius across; the radius is
// limited to half the shorter side.
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawRoundedRectangle(float64 fLeft, float64 fTop, float64 fRight, float64 fBottom, float64 fRadius)
{
	return DrawRoundedRectangle(fLeft, fTop, fRight, fBottom, fRadius, fRadius);
}


//////////////////////////////////////////////////////////////////////////
//
// The top corners are fTopRadius round and the bottom ones fBottomRadius,
// limited as AddVectorRoundedRectangle limits them.
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawRoundedRectangle(float64 fLeft, float64 fTop, float64 fRight, float64 fBottom, float64 fTopRadius, float64 fBottomRadius)
{
	CVectorPath		cPath;
	SFloat64Vec2	sMinimum;
	SFloat64Vec2	sMaximum;
	bool			bResult;

	if ((fRight <= fLeft) || (fBottom <= fTop))
	{
		return true;
	}
	cPath.Init();
	sMinimum.Init(fLeft, fTop);
	sMaximum.Init(fRight, fBottom);
	AddVectorRoundedRectangle(&cPath, &sMinimum, &sMaximum, fTopRadius, fBottomRadius);
	bResult = DrawPath(&cPath);
	cPath.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Blends the colour over the rasteriser's pixels.  A pixel's opacity is
// the colour's opacity times the part of the pixel covered.  Its colour is
// not multiplied by its opacity.  The image must have opacity, red, green
// and blue channels.  A pixel is blended over the one already there, so
// pieces drawn one on top of another (a border then the face inside it)
// mix along their edges.  Pixels not covered are left alone.  The coverage
// and blending are worked out with integers so every build draws the same
// pixels.
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::BlendCoverage(void)
{
	CImageAccessorCreator	cCreator;
	CImageAccessor*			pcAccessor;
	uint8					auiSource[4];  //Opacity, red, green then blue.
	uint8*					puiDest;
	uint8*					puiCoverage;
	int						x;
	int						y;
	int						iWidth;
	int						iCoverage;

	auiSource[0] = (uint8)(mfOpacity * 255.0f + 0.5f);
	auiSource[1] = (uint8)(mfRed * 255.0f + 0.5f);
	auiSource[2] = (uint8)(mfGreen * 255.0f + 0.5f);
	auiSource[3] = (uint8)(mfBlue * 255.0f + 0.5f);

	cCreator.Init(mpcImage);
	cCreator.AddAccess(IMAGE_OPACITY, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, PT_uint8);
	pcAccessor = cCreator.CreateAndKill();

	puiCoverage = mcRasteriser.GetCoverage();
	iWidth = mcRasteriser.GetWidth();
	for (y = 0; y < mcRasteriser.GetHeight(); y++)
	{
		for (x = 0; x < iWidth; x++)
		{
			iCoverage = puiCoverage[(lsize)y * iWidth + x];
			if (iCoverage > 0)
			{
				//Coverage is in 255ths and the blend takes 256ths.
				iCoverage = (int)DivideRound((int64)iCoverage * 256, 255);
				puiDest = (uint8*)pcAccessor->Get(mcRasteriser.GetLeft() + x, mcRasteriser.GetTop() + y);
				BlendStraightOverPixel(puiDest, auiSource, iCoverage);
				pcAccessor->Set(mcRasteriser.GetLeft() + x, mcRasteriser.GetTop() + y, puiDest);
			}
		}
	}
	pcAccessor->Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawCel(int x, int y, Ptr<CImageCel> pCel, CInt32Rect* pcClip)
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
bool CImageDraw::DrawCel(int x, int y, int iWidth, int iHeight, Ptr<CImageCel> pCel, CInt32Rect* pcClip)
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

