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
#include "BaseLib/Int64Vec3.h"
#include "Rasteriser.h"


//Shade is 0 .. 256.
#define RASTER_AMBIENT			64
#define RASTER_DIFFUSE			192

//Twice a triangle's area in subpixels squared, 16 pixels.  Below it each covered pixel is worked out exactly, and there
//is little to do because most triangles here cover a few pixels at most.  From it, gradients step across the triangle.
#define RASTER_SMALL_AREA		16

//From this many pixels across, it's worth working out where each row's covered pixels are rather than looking for them.
//A long thin triangle at an angle, as a board's outline makes, covers next to nothing of a very big box.
#define RASTER_WIDE_PIXELS		16

//Twice a triangle's area, or how far a pixel is inside one of its edges: subpixel times subpixel.
typedef SFixed<int64, int64, 8>		SRasterEdge;

//A depth or shade.
typedef SFixed<int64, int64, 0>		SRasterWideWhole;

//How much a depth or shade changes across a pixel.
typedef SFixed<int64, int64, 8>		SRasterGradient;

//A depth or shade part of the way across a triangle: a gradient times a distance in subpixels.
typedef SFixed<int64, int64, 12>	SRasterInterpolated;

//A distance across a triangle.
typedef SFixed<int64, int64, 4>		SRasterWideSubpixel;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::Init(void)
{
	masScreen.Init();
	mbCullBackFaces = true;

	//From over the viewer's shoulder.
	msLight.Init(SRasterUnit::FromRaw(4096), SRasterUnit::FromRaw(6144), SRasterUnit::FromRaw(14336));
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
	SRasterUnitVec3			sLight;
	SFixed<int32, int32, 28>	sDot;

	pcCamera->Prepare(pcTarget->miWidth, pcTarget->miHeight);
	pcCamera->ViewToMesh(&sLight, &msLight);

	uiNumVertices = pcMesh->masVertices.NumElements();
	GrowScreen(uiNumVertices);

	psVertex = pcMesh->masVertices.GetData();
	psScreen = masScreen.GetData();
	for (i = 0; i < uiNumVertices; i++)
	{
		pcCamera->Project(&psScreen[i], &psVertex[i].sPosition);

		sDot = psVertex[i].sNormal.x * sLight.x + psVertex[i].sNormal.y * sLight.y + psVertex[i].sNormal.z * sLight.z;
		if (sDot.miRaw < 0)
		{
			sDot.miRaw = 0;
		}
		psScreen[i].iShade = RASTER_AMBIENT + (SFixed<int64, int64, 28>::FromRaw(sDot.miRaw) * SRasterWideWhole::FromRaw(RASTER_DIFFUSE)).Floor();
	}

	DrawTriangles(&pcMesh->masOpaque, psScreen, pcTarget, false);
	DrawTriangles(&pcMesh->masTransparent, psScreen, pcTarget, true);
}


//////////////////////////////////////////////////////////////////////////
//
// The screen holds every instance's vertices, one after another, so that the
// transparent triangles drawn after all the opaque ones use them as they are.
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::Draw(SRasterInstance* asInstances, size uiNumInstances, CRasterCamera* pcCamera, CRasterTarget* pcTarget)
{
	SRasterUnitVec3			sLight;
	SRasterScreenVertex*	psScreen;
	size					uiNumVertices;
	size					i;

	pcCamera->Prepare(pcTarget->miWidth, pcTarget->miHeight);
	pcCamera->ViewToMesh(&sLight, &msLight);

	uiNumVertices = 0;
	for (i = 0; i < uiNumInstances; i++)
	{
		uiNumVertices += asInstances[i].pcMesh->masVertices.NumElements();
	}
	GrowScreen(uiNumVertices);

	psScreen = masScreen.GetData();
	for (i = 0; i < uiNumInstances; i++)
	{
		ProjectInstance(&asInstances[i], pcCamera, &sLight, psScreen);
		DrawTriangles(&asInstances[i].pcMesh->masOpaque, psScreen, pcTarget, false);
		psScreen += asInstances[i].pcMesh->masVertices.NumElements();
	}

	psScreen = masScreen.GetData();
	for (i = 0; i < uiNumInstances; i++)
	{
		DrawTriangles(&asInstances[i].pcMesh->masTransparent, psScreen, pcTarget, true);
		psScreen += asInstances[i].pcMesh->masVertices.NumElements();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::GrowScreen(size uiNumVertices)
{
	if (masScreen.NumElements() < uiNumVertices)
	{
		masScreen.Resize(uiNumVertices);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An instance's vertices on the screen, to psScreen.  psLight is towards the
// light in the space the instances are in.  Turning it back into the mesh's
// space, once, is the same as turning every normal out of it.
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::ProjectInstance(SRasterInstance* psInstance, CRasterCamera* pcCamera, SRasterUnitVec3* psLight, SRasterScreenVertex* psScreen)
{
	size					i;
	size					uiNumVertices;
	SRasterVertex*			psVertex;
	SInt64Vec3				sAxes[3];
	SInt64Vec3				sPosition;
	SInt64Vec3				sNormal;
	SInt64Vec3				sLight;
	SInt32Vec3				sMoved;
	int64					iDot;

	for (i = 0; i < 3; i++)
	{
		sAxes[i].Init(psInstance->asAxes[i].x, psInstance->asAxes[i].y, psInstance->asAxes[i].z);
	}
	sLight.Init((sAxes[0].x * psLight->x.miRaw + sAxes[1].x * psLight->y.miRaw + sAxes[2].x * psLight->z.miRaw) >> RASTER_AXIS_BITS,
				(sAxes[0].y * psLight->x.miRaw + sAxes[1].y * psLight->y.miRaw + sAxes[2].y * psLight->z.miRaw) >> RASTER_AXIS_BITS,
				(sAxes[0].z * psLight->x.miRaw + sAxes[1].z * psLight->y.miRaw + sAxes[2].z * psLight->z.miRaw) >> RASTER_AXIS_BITS);

	uiNumVertices = psInstance->pcMesh->masVertices.NumElements();
	psVertex = psInstance->pcMesh->masVertices.GetData();
	for (i = 0; i < uiNumVertices; i++)
	{
		sPosition.Init(psVertex[i].sPosition.x, psVertex[i].sPosition.y, psVertex[i].sPosition.z);
		sMoved.Init((int32)(Int64Vec3Dot(&sPosition, &sAxes[0]) >> RASTER_AXIS_BITS) + psInstance->sOffset.x,
					(int32)(Int64Vec3Dot(&sPosition, &sAxes[1]) >> RASTER_AXIS_BITS) + psInstance->sOffset.y,
					(int32)(Int64Vec3Dot(&sPosition, &sAxes[2]) >> RASTER_AXIS_BITS) + psInstance->sOffset.z);
		pcCamera->Project(&psScreen[i], &sMoved);

		sNormal.Init(psVertex[i].sNormal.x.miRaw, psVertex[i].sNormal.y.miRaw, psVertex[i].sNormal.z.miRaw);
		iDot = Int64Vec3Dot(&sNormal, &sLight);
		if (iDot < 0)
		{
			iDot = 0;
		}
		psScreen[i].iShade = RASTER_AMBIENT + (int32)((iDot * RASTER_DIFFUSE) >> (SRasterUnit::Bits * 2));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasteriser::DrawTriangles(CArrayRasterTriangle* pasTriangles, SRasterScreenVertex* psScreen, CRasterTarget* pcTarget, bool bTransparent)
{
	size				i;
	size				uiNumTriangles;
	SRasterTriangle*	psTriangle;

	uiNumTriangles = pasTriangles->NumElements();
	psTriangle = pasTriangles->GetData();
	for (i = 0; i < uiNumTriangles; i++)
	{
		DrawTriangle(pcTarget, &psScreen[psTriangle[i].aiVertex[0]], &psScreen[psTriangle[i].aiVertex[1]], &psScreen[psTriangle[i].aiVertex[2]], psTriangle[i].uiColour, bTransparent);
	}
}


static SRasterSubpixel Min3(SRasterSubpixel a, SRasterSubpixel b, SRasterSubpixel c)
{
	SRasterSubpixel	s;

	s = (a < b) ? a : b;
	return (s < c) ? s : c;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SRasterSubpixel Max3(SRasterSubpixel a, SRasterSubpixel b, SRasterSubpixel c)
{
	SRasterSubpixel	s;

	s = (a > b) ? a : b;
	return (s > c) ? s : c;
}


//////////////////////////////////////////////////////////////////////////
//
// Positive inside a triangle with a positive area.  Edges that are a top or a left edge own the pixels exactly on them;
// the rest are moved inwards by the smallest amount.
//
//////////////////////////////////////////////////////////////////////////
static SRasterEdge RasterEdgeBias(SRasterSubpixelVec2* psFrom, SRasterSubpixelVec2* psTo)
{
	SRasterSubpixel	sDeltaX;
	SRasterSubpixel	sDeltaY;

	sDeltaX = psTo->x - psFrom->x;
	sDeltaY = psTo->y - psFrom->y;
	return SRasterEdge::FromRaw(((sDeltaY.miRaw < 0) || ((sDeltaY.miRaw == 0) && (sDeltaX.miRaw > 0))) ? 0 : -1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SRasterEdge RasterEdge(SRasterSubpixelVec2* psFrom, SRasterSubpixelVec2* psTo, SRasterSubpixel sX, SRasterSubpixel sY)
{
	return (psTo->x - psFrom->x) * (sY - psFrom->y) - (psTo->y - psFrom->y) * (sX - psFrom->x);
}


//////////////////////////////////////////////////////////////////////////
//
// The weights are how far a pixel is inside each edge, so a value is the weighted sum over the area.  Depths and shades
// on one triangle share one format.
//
//////////////////////////////////////////////////////////////////////////
static int32 RasterWeigh(SRasterEdge s0, SRasterEdge s1, SRasterEdge s2, SRasterWideWhole sA, SRasterWideWhole sB, SRasterWideWhole sC, SRasterEdge sArea)
{
	return (int32)(s0 * sA + s1 * sB + s2 * sC).DivideFloor(sArea).miRaw;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SRasterGradient RasterGradient(SRasterWideWhole sDeltaB, SRasterWideWhole sDeltaC, SRasterSubpixel sDistanceB, SRasterSubpixel sDistanceC, SRasterEdge sArea)
{
	return (sDeltaB * sDistanceC.To<SRasterWideSubpixel>() - sDeltaC * sDistanceB.To<SRasterWideSubpixel>()).To<SFixed<int64, int64, 16> >().DivideFloor(sArea);
}


//////////////////////////////////////////////////////////////////////////
//
// Narrows a row's pixels to those an edge lets in: where its value, sValue
// at pixel iX and changing by sStep each pixel, isn't negative.
//
//////////////////////////////////////////////////////////////////////////
static void RasterEdgeSpan(SRasterEdge sValue, SRasterEdge sStep, int32 iX, int32* piFirstX, int32* piLastX)
{
	int64	iValue;
	int64	iStep;
	int64	iPixels;

	iValue = sValue.miRaw;
	iStep = sStep.miRaw;
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
	SRasterEdge				sArea;
	int32					iMinX;
	int32					iMaxX;
	int32					iMinY;
	int32					iMaxY;
	int32					x;
	int32					y;
	SRasterSubpixel			sSampleX;
	SRasterSubpixel			sSampleY;
	SRasterSubpixel			sOnePixel;
	SRasterEdge				sBias0;
	SRasterEdge				sBias1;
	SRasterEdge				sBias2;
	SRasterEdge				sRow0;
	SRasterEdge				sRow1;
	SRasterEdge				sRow2;
	SRasterEdge				s0;
	SRasterEdge				s1;
	SRasterEdge				s2;
	SRasterEdge				sStepX0;
	SRasterEdge				sStepX1;
	SRasterEdge				sStepX2;
	SRasterEdge				sStepY0;
	SRasterEdge				sStepY1;
	SRasterEdge				sStepY2;
	SRasterWideWhole		sDepthA;
	SRasterWideWhole		sDepthB;
	SRasterWideWhole		sDepthC;
	SRasterWideWhole		sShadeA;
	SRasterWideWhole		sShadeB;
	SRasterWideWhole		sShadeC;
	int32					iMinDepth;
	int32					iMaxDepth;
	SRasterGradient			sDepthX;
	SRasterGradient			sDepthY;
	SRasterGradient			sShadeX;
	SRasterGradient			sShadeY;
	SRasterInterpolated		sDepth;
	SRasterInterpolated		sShade;
	SRasterInterpolated		sStepDepth;
	SRasterInterpolated		sStepShade;
	SRasterWideSubpixel		sOffsetX;
	SRasterWideSubpixel		sOffsetY;
	SRasterSubpixel			sDeltaBX;
	SRasterSubpixel			sDeltaBY;
	SRasterSubpixel			sDeltaCX;
	SRasterSubpixel			sDeltaCY;
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

	sArea = RasterEdge(&psA->sPosition, &psB->sPosition, psC->sPosition.x, psC->sPosition.y);
	if (sArea.miRaw == 0)
	{
		return;
	}

	//The mesh's front faces wind anticlockwise looking at them with y up.  With y down their area is negative.
	if (sArea.miRaw < 0)
	{
		psSwap = psB;
		psB = psC;
		psC = psSwap;
		sArea = -sArea;
	}
	else if (mbCullBackFaces)
	{
		return;
	}

	//A pixel's sample is at its centre.
	sOnePixel = SRasterSubpixel::FromInt(1);
	iMinX = (Min3(psA->sPosition.x, psB->sPosition.x, psC->sPosition.x) + SRasterSubpixel::FromRaw(7)).Floor();
	iMinY = (Min3(psA->sPosition.y, psB->sPosition.y, psC->sPosition.y) + SRasterSubpixel::FromRaw(7)).Floor();
	iMaxX = (Max3(psA->sPosition.x, psB->sPosition.x, psC->sPosition.x) - SRasterSubpixel::FromRaw(8)).Floor();
	iMaxY = (Max3(psA->sPosition.y, psB->sPosition.y, psC->sPosition.y) - SRasterSubpixel::FromRaw(8)).Floor();
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

	sSampleX = SRasterSubpixel::FromInt(iMinX) + SRasterSubpixel::FromRaw(8);
	sSampleY = SRasterSubpixel::FromInt(iMinY) + SRasterSubpixel::FromRaw(8);

	sBias0 = RasterEdgeBias(&psB->sPosition, &psC->sPosition);
	sBias1 = RasterEdgeBias(&psC->sPosition, &psA->sPosition);
	sBias2 = RasterEdgeBias(&psA->sPosition, &psB->sPosition);

	sRow0 = RasterEdge(&psB->sPosition, &psC->sPosition, sSampleX, sSampleY) + sBias0;
	sRow1 = RasterEdge(&psC->sPosition, &psA->sPosition, sSampleX, sSampleY) + sBias1;
	sRow2 = RasterEdge(&psA->sPosition, &psB->sPosition, sSampleX, sSampleY) + sBias2;

	sStepX0 = (psB->sPosition.y - psC->sPosition.y) * sOnePixel;
	sStepX1 = (psC->sPosition.y - psA->sPosition.y) * sOnePixel;
	sStepX2 = (psA->sPosition.y - psB->sPosition.y) * sOnePixel;
	sStepY0 = (psC->sPosition.x - psB->sPosition.x) * sOnePixel;
	sStepY1 = (psA->sPosition.x - psC->sPosition.x) * sOnePixel;
	sStepY2 = (psB->sPosition.x - psA->sPosition.x) * sOnePixel;

	sDepthA = SRasterWideWhole::FromRaw(psA->iDepth);
	sDepthB = SRasterWideWhole::FromRaw(psB->iDepth);
	sDepthC = SRasterWideWhole::FromRaw(psC->iDepth);
	sShadeA = SRasterWideWhole::FromRaw(psA->iShade);
	sShadeB = SRasterWideWhole::FromRaw(psB->iShade);
	sShadeC = SRasterWideWhole::FromRaw(psC->iShade);

	bSmall = sArea < SRasterEdge::FromInt(RASTER_SMALL_AREA);
	sDepthX = SRasterGradient::FromRaw(0);
	sDepthY = SRasterGradient::FromRaw(0);
	sShadeX = SRasterGradient::FromRaw(0);
	sShadeY = SRasterGradient::FromRaw(0);
	iMinDepth = 0;
	iMaxDepth = 0;
	if (!bSmall)
	{
		sDeltaBX = psB->sPosition.x - psA->sPosition.x;
		sDeltaBY = psB->sPosition.y - psA->sPosition.y;
		sDeltaCX = psC->sPosition.x - psA->sPosition.x;
		sDeltaCY = psC->sPosition.y - psA->sPosition.y;
		sDepthX = RasterGradient(sDepthB - sDepthA, sDepthC - sDepthA, sDeltaBY, sDeltaCY, sArea);
		sDepthY = RasterGradient(sDepthC - sDepthA, sDepthB - sDepthA, sDeltaCX, sDeltaBX, sArea);
		sShadeX = RasterGradient(sShadeB - sShadeA, sShadeC - sShadeA, sDeltaBY, sDeltaCY, sArea);
		sShadeY = RasterGradient(sShadeC - sShadeA, sShadeB - sShadeA, sDeltaCX, sDeltaBX, sArea);
		iMinDepth = psA->iDepth;
		iMinDepth = (psB->iDepth < iMinDepth) ? psB->iDepth : iMinDepth;
		iMinDepth = (psC->iDepth < iMinDepth) ? psC->iDepth : iMinDepth;
		iMaxDepth = psA->iDepth;
		iMaxDepth = (psB->iDepth > iMaxDepth) ? psB->iDepth : iMaxDepth;
		iMaxDepth = (psC->iDepth > iMaxDepth) ? psC->iDepth : iMaxDepth;
	}
	sStepDepth = sDepthX.To<SRasterInterpolated>();
	sStepShade = sShadeX.To<SRasterInterpolated>();

	puiPixelRow = &pcTarget->mpuiPixels[(lsize)iMinY * (lsize)pcTarget->miStride];
	piDepthRow = &pcTarget->mpiDepths[(lsize)iMinY * (lsize)pcTarget->miWidth];
	sDepth = SRasterInterpolated::FromRaw(0);
	sShade = SRasterInterpolated::FromRaw(0);
	bWide = (iMaxX - iMinX) >= RASTER_WIDE_PIXELS;
	for (y = iMinY; y <= iMaxY; y++)
	{
		s0 = sRow0;
		s1 = sRow1;
		s2 = sRow2;
		iNextX = -1;
		iFirstX = iMinX;
		iLastX = iMaxX;
		if (bWide)
		{
			//A triangle is convex, so a row covers one run of pixels and each edge says where it can start or stop.
			RasterEdgeSpan(s0, sStepX0, iMinX, &iFirstX, &iLastX);
			RasterEdgeSpan(s1, sStepX1, iMinX, &iFirstX, &iLastX);
			RasterEdgeSpan(s2, sStepX2, iMinX, &iFirstX, &iLastX);
			if (iFirstX > iMinX)
			{
				s0 += SRasterEdge::FromRaw(sStepX0.miRaw * (iFirstX - iMinX));
				s1 += SRasterEdge::FromRaw(sStepX1.miRaw * (iFirstX - iMinX));
				s2 += SRasterEdge::FromRaw(sStepX2.miRaw * (iFirstX - iMinX));
			}
		}
		for (x = iFirstX; x <= iLastX; x++)
		{
			if ((s0.miRaw | s1.miRaw | s2.miRaw) >= 0)
			{
				if (bSmall)
				{
					iDepth = RasterWeigh(s0 - sBias0, s1 - sBias1, s2 - sBias2, sDepthA, sDepthB, sDepthC, sArea);
					iShade = 0;  //Worked out below, if the pixel is to be drawn: most of a mesh's pixels are behind another.
				}
				else
				{
					//Each row's run of covered pixels only needs its start working out.
					if (iNextX != x)
					{
						sOffsetX = (SRasterSubpixel::FromInt(x) + SRasterSubpixel::FromRaw(8) - psA->sPosition.x).To<SRasterWideSubpixel>();
						sOffsetY = (SRasterSubpixel::FromInt(y) + SRasterSubpixel::FromRaw(8) - psA->sPosition.y).To<SRasterWideSubpixel>();
						sDepth = sDepthA.To<SRasterInterpolated>() + sDepthX * sOffsetX + sDepthY * sOffsetY;
						sShade = sShadeA.To<SRasterInterpolated>() + sShadeX * sOffsetX + sShadeY * sOffsetY;
					}
					iDepth = sDepth.Floor();
					iShade = sShade.Floor();
					sDepth += sStepDepth;
					sShade += sStepShade;
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
						iShade = RasterWeigh(s0 - sBias0, s1 - sBias1, s2 - sBias2, sShadeA, sShadeB, sShadeC, sArea);
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
			s0 += sStepX0;
			s1 += sStepX1;
			s2 += sStepX2;
		}
		sRow0 += sStepY0;
		sRow1 += sStepY1;
		sRow2 += sStepY2;
		puiPixelRow += pcTarget->miStride;
		piDepthRow += pcTarget->miWidth;
	}
}

