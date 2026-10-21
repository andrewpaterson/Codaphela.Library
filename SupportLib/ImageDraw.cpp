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
// The direction of iAngle degrees in 65536ths, rounded so the right
// angles are exact on every build.  y is up.
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::AngleDirection(int iAngle, int64* piX, int64* piY)
{
	float64	fRadians;

	fRadians = ((float64)iAngle * 3.14159265358979323846) / 180.0;
	*piX = (int64)floor(cos(fRadians) * 65536.0 + 0.5);
	*piY = (int64)floor(sin(fRadians) * 65536.0 + 0.5);
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
// The edge is anti-aliased: a pixel's opacity is the colour's opacity
// times the part of the pixel's 16 x 16 samples inside the part of the
// circle (or the one pixel thick ring when not filled).  Its colour is not
// multiplied by its opacity.  The image must have opacity, red, green and
// blue channels.  A pixel is blended over the one already there, so
// pieces drawn one on top of another (a border then the face inside it)
// mix along their edges.  Pixels outside the circle are left alone.  The
// coverage and blending are worked out with integers so every build draws
// the same pixels.
//
//////////////////////////////////////////////////////////////////////////
bool CImageDraw::DrawSemicircle(int x, int y, int iDiameter, int iStartAngle, int iEndAngle, bool bFilled)
{
	CImageAccessorCreator	cCreator;
	CImageAccessor*			pcAccessor;
	uint8					auiSource[4];  //Opacity, red, green then blue.
	uint8*					puiDest;
	int64					iCentreX;  //In 32nds of a pixel from the image's top left, so samples sit in the middle of their 16ths.
	int64					iCentreY;
	int64					iRadiusSquared;
	int64					iInnerRadius;
	int64					iInnerRadiusSquared;  //Samples this close to the centre or closer are not drawn.  -1 draws them all.
	int64					iDistanceSquared;
	int64					iStartX;
	int64					iStartY;
	int64					iEndX;
	int64					iEndY;
	int64					dx;
	int64					dy;
	int						iSweep;
	bool					bWhole;
	bool					bInside;
	int						iLeft;
	int						iTop;
	int						iRight;
	int						iBottom;
	int						px;
	int						py;
	int						sx;
	int						sy;
	int						iCoverage;

	if (iDiameter <= 0)
	{
		return true;
	}
	if (!mpcImage->HasChannel(IMAGE_OPACITY))
	{
		return gcLogger.Error2(__METHOD__, " Cannot draw an anti-aliased semicircle on an image without an opacity channel.", NULL);
	}

	bWhole = (iEndAngle - iStartAngle) >= 360;
	iSweep = ((iEndAngle - iStartAngle) % 360 + 360) % 360;
	if (!bWhole && (iSweep == 0))
	{
		return true;
	}
	AngleDirection(iStartAngle, &iStartX, &iStartY);
	AngleDirection(iEndAngle, &iEndX, &iEndY);

	iCentreX = (int64)x * 32 + (int64)iDiameter * 16;
	iCentreY = (int64)y * 32 + (int64)iDiameter * 16;
	iRadiusSquared = (int64)iDiameter * 16 * iDiameter * 16;
	iInnerRadiusSquared = -1;
	if (!bFilled)
	{
		iInnerRadius = (int64)iDiameter * 16 - 32;
		if (iInnerRadius > 0)
		{
			iInnerRadiusSquared = iInnerRadius * iInnerRadius;
		}
	}

	iLeft = x < 0 ? 0 : x;
	iTop = y < 0 ? 0 : y;
	iRight = x + iDiameter;
	if (iRight > (int)mpcImage->GetWidth())
	{
		iRight = (int)mpcImage->GetWidth();
	}
	iBottom = y + iDiameter;
	if (iBottom > (int)mpcImage->GetHeight())
	{
		iBottom = (int)mpcImage->GetHeight();
	}

	auiSource[0] = (uint8)(mfOpacity * 255.0f + 0.5f);
	auiSource[1] = (uint8)(mfRed * 255.0f + 0.5f);
	auiSource[2] = (uint8)(mfGreen * 255.0f + 0.5f);
	auiSource[3] = (uint8)(mfBlue * 255.0f + 0.5f);

	cCreator.Init(mpcImage);
	cCreator.AddAccess(IMAGE_OPACITY, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, PT_uint8);
	pcAccessor = cCreator.CreateAndKill();
	for (py = iTop; py < iBottom; py++)
	{
		for (px = iLeft; px < iRight; px++)
		{
			iCoverage = 0;
			for (sy = 0; sy < 16; sy++)
			{
				dy = (int64)py * 32 + sy * 2 + 1 - iCentreY;
				for (sx = 0; sx < 16; sx++)
				{
					dx = (int64)px * 32 + sx * 2 + 1 - iCentreX;
					iDistanceSquared = dx * dx + dy * dy;
					if ((iDistanceSquared <= iRadiusSquared) && (iDistanceSquared > iInnerRadiusSquared))
					{
						//A sample is never exactly on a side at a multiple of 90 degrees so quarters and halves drawn side by side
						//don't share one.  y is up for the angles.
						if (bWhole)
						{
							bInside = true;
						}
						else if (iSweep <= 180)
						{
							bInside = (iStartX * -dy - iStartY * dx >= 0) && (dx * iEndY - -dy * iEndX >= 0);
						}
						else
						{
							bInside = !((iEndX * -dy - iEndY * dx > 0) && (dx * iStartY - -dy * iStartX > 0));
						}

						if (bInside)
						{
							iCoverage++;
						}
					}
				}
			}

			if (iCoverage > 0)
			{
				puiDest = (uint8*)pcAccessor->Get(px, py);
				BlendPixel(puiDest, auiSource, iCoverage);
				pcAccessor->Set(px, py, puiDest);
			}
		}
	}
	pcAccessor->Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Blends the source colour, iCoverage 256ths of it, over the destination.
// Both are opacity, red, green then blue and neither is multiplied by its
// opacity.  The result is rounded to the nearest so drawing a colour over
// itself doesn't change it.
//
//////////////////////////////////////////////////////////////////////////
void CImageDraw::BlendPixel(uint8* puiDest, uint8* puiSource, int iCoverage)
{
	int64	iSourceOpacity;  //In 65280ths (255 * 256).
	int64	iDestOpacity;
	int64	iOpacity;  //In 65280ths of 65280ths.
	int		i;

	iSourceOpacity = (int64)puiSource[0] * iCoverage;
	if ((puiDest[0] == 0) || (iSourceOpacity == 65280))
	{
		puiDest[0] = (uint8)((iSourceOpacity + 128) / 256);
		for (i = 1; i < 4; i++)
		{
			puiDest[i] = puiSource[i];
		}
		return;
	}

	iDestOpacity = (int64)puiDest[0] * 256;
	iOpacity = iSourceOpacity * 65280 + iDestOpacity * (65280 - iSourceOpacity);
	if (iOpacity == 0)
	{
		return;
	}
	for (i = 1; i < 4; i++)
	{
		puiDest[i] = (uint8)(((int64)puiSource[i] * iSourceOpacity * 65280 + (int64)puiDest[i] * iDestOpacity * (65280 - iSourceOpacity) + iOpacity / 2) / iOpacity);
	}
	puiDest[0] = (uint8)((iOpacity + 65280 * 128) / (65280 * 256));
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

