/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RasterLib

Codaphela RasterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RasterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RasterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/FixedPoint.h"
#include "Rasteriser.h"


//Shade is 0 .. 256.
#define RASTER_AMBIENT			64
#define RASTER_DIFFUSE			192

//Twice a triangle's area in subpixels squared, 16 pixels.  Below it each covered pixel is worked out exactly, and there
//is little to do because most triangles here cover a few pixels at most.  From it, gradients step across the triangle.
#define RASTER_SMALL_AREA		(16 << (RASTER_SUBPIXEL_BITS * 2))

//From this many pixels across, it's worth working out where each row's covered pixels are rather than looking for them.
//A long thin triangle at an angle, as a board's outline makes, covers next to nothing of a very big box.
#define RASTER_WIDE_PIXELS		16

//Fraction bits of the depth and shade gradients.
#define RASTER_GRADIENT_BITS	12


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::Init(void)
{
	masScreen.Init();
	mbCullBackFaces = true;

	//From over the viewer's shoulder.
	msLight.Init(4096, 6144, 14336);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::Kill(void)
{
	masScreen.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::SetCullBackFaces(bool bCullBackFaces)
{
	mbCullBackFaces = bCullBackFaces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::Draw(CRasterMesh* pcMesh, CRasterCamera* pcCamera, CRasterTarget* pcTarget)
{
	size					i;
	size					uiNumVertices;
	SRasterVertex*			psVertex;
	SRasterScreenVertex*	psScreen;
	SInt64Vec3				sLight;
	SInt64Vec3				sNormal;
	int64					iDot;

	pcCamera->Prepare(pcTarget->miWidth, pcTarget->miHeight);
	pcCamera->ViewToMesh(&sLight, &msLight);

	uiNumVertices = pcMesh->masVertices.NumElements();
	masScreen.Resize(uiNumVertices);

	psVertex = pcMesh->masVertices.GetData();
	psScreen = masScreen.GetData();
	for (i = 0; i < uiNumVertices; i++)
	{
		pcCamera->Project(&psScreen[i], &psVertex[i].sPosition);

		sNormal.Init(psVertex[i].sNormal.x, psVertex[i].sNormal.y, psVertex[i].sNormal.z);
		iDot = Int64Vec3Dot(&sNormal, &sLight);
		if (iDot < 0)
		{
			iDot = 0;
		}
		psScreen[i].iShade = RASTER_AMBIENT + (int32)((iDot * RASTER_DIFFUSE) >> (RASTER_NORMAL_BITS * 2));
	}

	DrawTriangles(&pcMesh->masOpaque, pcTarget, false);
	DrawTriangles(&pcMesh->masTransparent, pcTarget, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::DrawTriangles(CArrayRasterTriangle* pasTriangles, CRasterTarget* pcTarget, bool bTransparent)
{
	size					i;
	size					uiNumTriangles;
	SRasterTriangle*		psTriangle;
	SRasterScreenVertex*	psScreen;

	uiNumTriangles = pasTriangles->NumElements();
	psTriangle = pasTriangles->GetData();
	psScreen = masScreen.GetData();
	for (i = 0; i < uiNumTriangles; i++)
	{
		DrawTriangle(pcTarget, &psScreen[psTriangle[i].aiVertex[0]], &psScreen[psTriangle[i].aiVertex[1]], &psScreen[psTriangle[i].aiVertex[2]], psTriangle[i].uiColour, bTransparent);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int32 Min3(int32 a, int32 b, int32 c)
{
	int32	i;

	i = (a < b) ? a : b;
	return (i < c) ? i : c;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int32 Max3(int32 a, int32 b, int32 c)
{
	int32	i;

	i = (a > b) ? a : b;
	return (i > c) ? i : c;
}


//////////////////////////////////////////////////////////////////////////
//
// Positive inside a triangle with a positive area.  Edges that are a top or a left edge own the pixels exactly on them;
// the rest are moved inwards by the smallest amount.
//
//////////////////////////////////////////////////////////////////////////
static int64 RasterEdgeBias(SInt32Vec2* psFrom, SInt32Vec2* psTo)
{
	int32	iDeltaX;
	int32	iDeltaY;

	iDeltaX = psTo->x - psFrom->x;
	iDeltaY = psTo->y - psFrom->y;
	return ((iDeltaY < 0) || ((iDeltaY == 0) && (iDeltaX > 0))) ? 0 : -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int64 RasterEdge(SInt32Vec2* psFrom, SInt32Vec2* psTo, int32 iX, int32 iY)
{
	return ((int64)psTo->x - psFrom->x) * ((int64)iY - psFrom->y) - ((int64)psTo->y - psFrom->y) * ((int64)iX - psFrom->x);
}


//////////////////////////////////////////////////////////////////////////
//
// Narrows a row's pixels to those an edge lets in: where its value, iValue
// at pixel iX and changing by iStep each pixel, isn't negative.
//
//////////////////////////////////////////////////////////////////////////
static void RasterEdgeSpan(int64 iValue, int64 iStep, int32 iX, int32* piFirstX, int32* piLastX)
{
	int64	iPixels;

	if (iStep > 0)
	{
		if (iValue < 0)
		{
			iPixels = (-iValue + iStep - 1) / iStep;
			if (iX + iPixels > *piFirstX)
			{
				*piFirstX = (iX + iPixels > *piLastX) ? *piLastX + 1 : (int32)(iX + iPixels);
			}
		}
	}
	else if (iValue < 0)
	{
		*piLastX = iX - 1;  //It only falls from here, or stays.
	}
	else if (iStep < 0)
	{
		iPixels = iValue / -iStep;
		if (iX + iPixels < *piLastX)
		{
			*piLastX = (int32)(iX + iPixels);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::DrawTriangle(CRasterTarget* pcTarget, SRasterScreenVertex* psA, SRasterScreenVertex* psB, SRasterScreenVertex* psC, uint32 uiColour, bool bTransparent)
{
	SRasterScreenVertex*	psSwap;
	int64					iArea;
	int32					iMinX;
	int32					iMaxX;
	int32					iMinY;
	int32					iMaxY;
	int32					x;
	int32					y;
	int32					iSampleX;
	int32					iSampleY;
	int64					iBias0;
	int64					iBias1;
	int64					iBias2;
	int64					iRow0;
	int64					iRow1;
	int64					iRow2;
	int64					i0;
	int64					i1;
	int64					i2;
	int64					iStepX0;
	int64					iStepX1;
	int64					iStepX2;
	int64					iStepY0;
	int64					iStepY1;
	int64					iStepY2;
	int64					iDepthA;
	int64					iDepthB;
	int64					iDepthC;
	int64					iShadeA;
	int64					iShadeB;
	int64					iShadeC;
	int32					iMinDepth;
	int32					iMaxDepth;
	int64					iDepthX;
	int64					iDepthY;
	int64					iShadeX;
	int64					iShadeY;
	int64					iDepthF;
	int64					iShadeF;
	int32					iDeltaBX;
	int32					iDeltaBY;
	int32					iDeltaCX;
	int32					iDeltaCY;
	int32					iNextX;
	int32					iFirstX;
	int32					iLastX;
	bool					bWide;
	bool					bSmall;
	int32					iDepth;
	int32					iShade;
	uint32					uiPixel;
	uint32					uiRB;
	uint32					uiG;
	uint32					uiAlpha;
	uint32*					puiPixelRow;
	int32*					piDepthRow;

	if ((psA->iDepth == 0) || (psB->iDepth == 0) || (psC->iDepth == 0))
	{
		return;
	}

	iArea = RasterEdge(&psA->sPosition, &psB->sPosition, psC->sPosition.x, psC->sPosition.y);
	if (iArea == 0)
	{
		return;
	}

	//The mesh's front faces wind anticlockwise looking at them with y up.  With y down their area is negative.
	if (iArea < 0)
	{
		psSwap = psB;
		psB = psC;
		psC = psSwap;
		iArea = -iArea;
	}
	else if (mbCullBackFaces)
	{
		return;
	}

	//A pixel's sample is at its centre.
	iMinX = (Min3(psA->sPosition.x, psB->sPosition.x, psC->sPosition.x) + (RASTER_SUBPIXELS / 2 - 1)) >> RASTER_SUBPIXEL_BITS;
	iMinY = (Min3(psA->sPosition.y, psB->sPosition.y, psC->sPosition.y) + (RASTER_SUBPIXELS / 2 - 1)) >> RASTER_SUBPIXEL_BITS;
	iMaxX = (Max3(psA->sPosition.x, psB->sPosition.x, psC->sPosition.x) - (RASTER_SUBPIXELS / 2)) >> RASTER_SUBPIXEL_BITS;
	iMaxY = (Max3(psA->sPosition.y, psB->sPosition.y, psC->sPosition.y) - (RASTER_SUBPIXELS / 2)) >> RASTER_SUBPIXEL_BITS;
	if (iMinX < 0)
	{
		iMinX = 0;
	}
	if (iMinY < 0)
	{
		iMinY = 0;
	}
	if (iMaxX >= pcTarget->miWidth)
	{
		iMaxX = pcTarget->miWidth - 1;
	}
	if (iMaxY >= pcTarget->miHeight)
	{
		iMaxY = pcTarget->miHeight - 1;
	}
	if ((iMinX > iMaxX) || (iMinY > iMaxY))
	{
		return;
	}

	iSampleX = (iMinX << RASTER_SUBPIXEL_BITS) + (RASTER_SUBPIXELS / 2);
	iSampleY = (iMinY << RASTER_SUBPIXEL_BITS) + (RASTER_SUBPIXELS / 2);

	iBias0 = RasterEdgeBias(&psB->sPosition, &psC->sPosition);
	iBias1 = RasterEdgeBias(&psC->sPosition, &psA->sPosition);
	iBias2 = RasterEdgeBias(&psA->sPosition, &psB->sPosition);

	iRow0 = RasterEdge(&psB->sPosition, &psC->sPosition, iSampleX, iSampleY) + iBias0;
	iRow1 = RasterEdge(&psC->sPosition, &psA->sPosition, iSampleX, iSampleY) + iBias1;
	iRow2 = RasterEdge(&psA->sPosition, &psB->sPosition, iSampleX, iSampleY) + iBias2;

	iStepX0 = (int64)(psB->sPosition.y - psC->sPosition.y) << RASTER_SUBPIXEL_BITS;
	iStepX1 = (int64)(psC->sPosition.y - psA->sPosition.y) << RASTER_SUBPIXEL_BITS;
	iStepX2 = (int64)(psA->sPosition.y - psB->sPosition.y) << RASTER_SUBPIXEL_BITS;
	iStepY0 = (int64)(psC->sPosition.x - psB->sPosition.x) << RASTER_SUBPIXEL_BITS;
	iStepY1 = (int64)(psA->sPosition.x - psC->sPosition.x) << RASTER_SUBPIXEL_BITS;
	iStepY2 = (int64)(psB->sPosition.x - psA->sPosition.x) << RASTER_SUBPIXEL_BITS;

	iDepthA = psA->iDepth;
	iDepthB = psB->iDepth;
	iDepthC = psC->iDepth;
	iShadeA = psA->iShade;
	iShadeB = psB->iShade;
	iShadeC = psC->iShade;

	bSmall = iArea < RASTER_SMALL_AREA;
	iDepthX = 0;
	iDepthY = 0;
	iShadeX = 0;
	iShadeY = 0;
	iMinDepth = 0;
	iMaxDepth = 0;
	if (!bSmall)
	{
		iDeltaBX = psB->sPosition.x - psA->sPosition.x;
		iDeltaBY = psB->sPosition.y - psA->sPosition.y;
		iDeltaCX = psC->sPosition.x - psA->sPosition.x;
		iDeltaCY = psC->sPosition.y - psA->sPosition.y;
		iDepthX = DivideFloor(((iDepthB - iDepthA) * iDeltaCY - (iDepthC - iDepthA) * iDeltaBY) << RASTER_GRADIENT_BITS, iArea);
		iDepthY = DivideFloor(((iDepthC - iDepthA) * iDeltaBX - (iDepthB - iDepthA) * iDeltaCX) << RASTER_GRADIENT_BITS, iArea);
		iShadeX = DivideFloor(((iShadeB - iShadeA) * iDeltaCY - (iShadeC - iShadeA) * iDeltaBY) << RASTER_GRADIENT_BITS, iArea);
		iShadeY = DivideFloor(((iShadeC - iShadeA) * iDeltaBX - (iShadeB - iShadeA) * iDeltaCX) << RASTER_GRADIENT_BITS, iArea);
		iMinDepth = Min3(psA->iDepth, psB->iDepth, psC->iDepth);
		iMaxDepth = Max3(psA->iDepth, psB->iDepth, psC->iDepth);
	}

	puiPixelRow = &pcTarget->mpuiPixels[(lsize)iMinY * (lsize)pcTarget->miStride];
	piDepthRow = &pcTarget->mpiDepths[(lsize)iMinY * (lsize)pcTarget->miWidth];
	iDepthF = 0;
	iShadeF = 0;
	bWide = (iMaxX - iMinX) >= RASTER_WIDE_PIXELS;
	for (y = iMinY; y <= iMaxY; y++)
	{
		i0 = iRow0;
		i1 = iRow1;
		i2 = iRow2;
		iNextX = -1;
		iFirstX = iMinX;
		iLastX = iMaxX;
		if (bWide)
		{
			//A triangle is convex, so a row covers one run of pixels and each edge says where it can start or stop.
			RasterEdgeSpan(i0, iStepX0, iMinX, &iFirstX, &iLastX);
			RasterEdgeSpan(i1, iStepX1, iMinX, &iFirstX, &iLastX);
			RasterEdgeSpan(i2, iStepX2, iMinX, &iFirstX, &iLastX);
			if (iFirstX > iMinX)
			{
				i0 += iStepX0 * (iFirstX - iMinX);
				i1 += iStepX1 * (iFirstX - iMinX);
				i2 += iStepX2 * (iFirstX - iMinX);
			}
		}
		for (x = iFirstX; x <= iLastX; x++)
		{
			if ((i0 | i1 | i2) >= 0)
			{
				if (bSmall)
				{
					iDepth = (int32)(((i0 - iBias0) * iDepthA + (i1 - iBias1) * iDepthB + (i2 - iBias2) * iDepthC) / iArea);
					iShade = 0;  //Worked out below, if the pixel is to be drawn: most of a mesh's pixels are behind another.
				}
				else
				{
					//Each row's run of covered pixels only needs its start working out.
					if (iNextX != x)
					{
						iSampleX = (x << RASTER_SUBPIXEL_BITS) + (RASTER_SUBPIXELS / 2);
						iSampleY = (y << RASTER_SUBPIXEL_BITS) + (RASTER_SUBPIXELS / 2);
						iDepthF = (iDepthA << RASTER_GRADIENT_BITS) + iDepthX * (iSampleX - psA->sPosition.x) + iDepthY * (iSampleY - psA->sPosition.y);
						iShadeF = (iShadeA << RASTER_GRADIENT_BITS) + iShadeX * (iSampleX - psA->sPosition.x) + iShadeY * (iSampleY - psA->sPosition.y);
					}
					iDepth = (int32)(iDepthF >> RASTER_GRADIENT_BITS);
					iShade = (int32)(iShadeF >> RASTER_GRADIENT_BITS);
					iDepthF += iDepthX << RASTER_SUBPIXEL_BITS;
					iShadeF += iShadeX << RASTER_SUBPIXEL_BITS;
					iNextX = x + 1;

					//Rounding must not move a pixel out of its triangle's depths, or 0 which is the empty depth.
					if (iDepth < iMinDepth)
					{
						iDepth = iMinDepth;
					}
					else if (iDepth > iMaxDepth)
					{
						iDepth = iMaxDepth;
					}
					if (iShade < 0)
					{
						iShade = 0;
					}
					else if (iShade > 256)
					{
						iShade = 256;
					}
				}

				if (iDepth > piDepthRow[x])
				{
					if (bSmall)
					{
						iShade = (int32)(((i0 - iBias0) * iShadeA + (i1 - iBias1) * iShadeB + (i2 - iBias2) * iShadeC) / iArea);
					}
					uiRB = (((uiColour & 0x00FF00FF) * (uint32)iShade) >> 8) & 0x00FF00FF;
					uiG = (((uiColour & 0x0000FF00) * (uint32)iShade) >> 8) & 0x0000FF00;
					if (bTransparent)
					{
						uiAlpha = uiColour >> 24;
						uiAlpha += uiAlpha >> 7;
						uiPixel = puiPixelRow[x];
						uiRB = ((uiRB * uiAlpha + (uiPixel & 0x00FF00FF) * (256 - uiAlpha)) >> 8) & 0x00FF00FF;
						uiG = ((uiG * uiAlpha + (uiPixel & 0x0000FF00) * (256 - uiAlpha)) >> 8) & 0x0000FF00;
					}
					else
					{
						piDepthRow[x] = iDepth;
					}
					puiPixelRow[x] = 0xFF000000 | uiRB | uiG;
				}
			}
			i0 += iStepX0;
			i1 += iStepX1;
			i2 += iStepX2;
		}
		iRow0 += iStepY0;
		iRow1 += iStepY1;
		iRow2 += iStepY2;
		puiPixelRow += pcTarget->miStride;
		piDepthRow += pcTarget->miWidth;
	}
}

