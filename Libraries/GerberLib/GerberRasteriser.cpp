#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "SupportLib/ColourBlend.h"
#include "GerberRasteriser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberRaster::Init(int iWidth, int iHeight, SFloat64Vec2* psOrigin, float64 fPixelsPerMillimetre)
{
	miWidth = iWidth;
	miHeight = iHeight;
	msOrigin = *psOrigin;
	mfPixelsPerMillimetre = fPixelsPerMillimetre;
	mpuiData = NULL;
	if ((iWidth <= 0) || (iHeight <= 0))
	{
		miWidth = 0;
		miHeight = 0;
		return false;
	}
	mpuiData = (uint8*)calloc((size)iWidth * iHeight, 1);
	return mpuiData != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberRaster::Init(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, float64 fPixelsPerMillimetre)
{
	SFloat64Vec2	sOrigin;

	//Covers the area; the left and top edges are on the minimum x and maximum y.
	sOrigin.Init(psMinimum->x, psMaximum->y);
	return Init((int)ceil((psMaximum->x - psMinimum->x) * fPixelsPerMillimetre), (int)ceil((psMaximum->y - psMinimum->y) * fPixelsPerMillimetre), &sOrigin, fPixelsPerMillimetre);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberRaster::Kill(void)
{
	free(mpuiData);
	mpuiData = NULL;
	miWidth = 0;
	miHeight = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberRaster::Clear(void)
{
	if (mpuiData)
	{
		memset(mpuiData, 0, (size)miWidth * miHeight);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8* CGerberRaster::GetData(void)
{
	return mpuiData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGerberRaster::GetWidth(void)
{
	return miWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGerberRaster::GetHeight(void)
{
	return miHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CGerberRaster::Get(int x, int y)
{
	return mpuiData[(size)y * miWidth + x];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec2* CGerberRaster::GetOrigin(void)
{
	return &msOrigin;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CGerberRaster::GetPixelsPerMillimetre(void)
{
	return mfPixelsPerMillimetre;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CGerberRaster::Area(void)
{
	size	i;
	size	uiCells;
	uint64	ulliSum;

	//The dark area in square millimetres.
	ulliSum = 0;
	uiCells = (size)miWidth * miHeight;
	for (i = 0; i < uiCells; i++)
	{
		ulliSum += mpuiData[i];
	}
	return (float64)ulliSum / 255.0 / (mfPixelsPerMillimetre * mfPixelsPerMillimetre);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberRasteriser::Init(float64 fTolerance)
{
	mcVector.Init(fTolerance);
	mpcRaster = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberRasteriser::Kill(void)
{
	mcVector.Kill();
	mpcRaster = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberRasteriser::Rasterise(CGerberVectorImage* pcImage, CGerberRaster* pcRaster)
{
	size	i;
	size	uiNumObjects;

	mpcRaster = pcRaster;
	mcVector.SetTransform(pcRaster->GetOrigin(), pcRaster->GetPixelsPerMillimetre(), true);
	uiNumObjects = pcImage->NumObjects();
	for (i = 0; i < uiNumObjects; i++)
	{
		if (!RasteriseObject(pcImage->GetObject(i)))
		{
			mpcRaster = NULL;
			return false;
		}
	}
	mpcRaster = NULL;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberRasteriser::Composite(bool bDark)
{
	int		x;
	int		y;
	int		iWidth;
	int32	iCoverage;
	int32	iDest;
	uint8*	puiCoverage;
	uint8*	puiDest;

	//Dark: d + c (1 - d).  Clear: d (1 - c).
	puiCoverage = mcVector.GetCoverage();
	iWidth = mcVector.GetWidth();
	for (y = 0; y < mcVector.GetHeight(); y++)
	{
		puiDest = mpcRaster->GetData() + (size)(mcVector.GetTop() + y) * mpcRaster->GetWidth() + mcVector.GetLeft();
		for (x = 0; x < iWidth; x++)
		{
			iCoverage = puiCoverage[(size)y * iWidth + x];
			if (iCoverage == 0)
			{
				continue;
			}
			iDest = puiDest[x];
			puiDest[x] = bDark ? BlendOverByte(iDest, iCoverage) : BlendEraseByte(iDest, iCoverage);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberRasteriser::RasteriseObject(CGerberVectorObject* pcObject)
{
	int		iLeft;
	int		iTop;
	int		iRight;
	int		iBottom;

	//Objects without an image (zero width draws) and objects outside the raster draw nothing.
	if (!mcVector.GetPixelBounds(pcObject, &iLeft, &iTop, &iRight, &iBottom))
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
	if (iRight > mpcRaster->GetWidth())
	{
		iRight = mpcRaster->GetWidth();
	}
	if (iBottom > mpcRaster->GetHeight())
	{
		iBottom = mpcRaster->GetHeight();
	}
	if ((iRight <= iLeft) || (iBottom <= iTop))
	{
		return true;
	}

	if (!mcVector.Rasterise(pcObject, iLeft, iTop, iRight - iLeft, iBottom - iTop))
	{
		return false;
	}
	Composite(pcObject->IsDark());
	return true;
}

