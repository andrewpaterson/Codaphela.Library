/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "BaseLib/ArrayInt.h"
#include "BaseLib/Float32Vec2.h"
#include "StandardLib/Objects.h"
#include "MeshUVMeasures.h"
#include "MeshUnwrap.h"


//The turns a chart's bounding rectangle is tried at, across a right angle, for the smallest.
#define MESH_UNWRAP_TURNS	90

//How many times charts that overlap themselves laid flat are grown again, at half the angle each time.  A chart of one face
//can't overlap itself, which is where halving ends.
#define MESH_UNWRAP_REGROWS	12


//Each chart's UVs in texels, while a layout is made: added chart by chart, so a chart's are maiChartStarts[c] to [c + 1].
struct SMeshUnwrapUVs
{
	CArrayLargeFloat32Vec2	masUVs;
	CArrayLargeInt			maiChartStarts;
	CArrayLargeInt			maiFaceUVs;  //Three for each face.
	CArrayLargeInt			maiSizes;  //Each chart's width and height in texels, once turned.
	CArrayLargeInt			maiPlaces;  //Where its rectangle went, gutters and all.

	void	Init(void);
	void	Kill(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SMeshUnwrapUVs::Init(void)
{
	masUVs.Init();
	maiChartStarts.Init();
	maiFaceUVs.Init();
	maiSizes.Init();
	maiPlaces.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SMeshUnwrapUVs::Kill(void)
{
	maiPlaces.Kill();
	maiSizes.Kill();
	maiFaceUVs.Kill();
	maiChartStarts.Kill();
	masUVs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshUnwrap::Init(float32 fTexelsPerUnit, EMeshUnwrapCharts eCharts)
{
	PreInit();
	CMeshModifier::Init();
	mfTexelsPerUnit = fTexelsPerUnit;
	meCharts = eCharts;
	mfChartAngle = MESH_CHARTS_ANGLE;
	miPageSize = MESH_UNWRAP_PAGE;
	miGutter = MESH_UNWRAP_GUTTER;
	miLayoutWidth = 0;
	miLayoutHeight = 0;
	miNumCharts = 0;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshUnwrap::Class(void)
{
	CMeshModifier::Class();
	U_Float32(mfTexelsPerUnit);
	U_Enum(meCharts);
	U_Float32(mfChartAngle);
	U_SInt(miPageSize);
	U_SInt(miGutter);
	U_SInt(miLayoutWidth);
	U_SInt(miLayoutHeight);
	U_SInt(miNumCharts);
}


//////////////////////////////////////////////////////////////////////////
//
// Two units across psNormal, with it right handed (u, v, n), so a face
// anticlockwise seen from the normal's side is anticlockwise in u and v.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUnwrapBasis(SFloat32Vec3* psNormal, SFloat32Vec3* psU, SFloat32Vec3* psV)
{
	SFloat32Vec3	sAxis;
	float32			fX;
	float32			fY;
	float32			fZ;

	fX = fabsf(psNormal->x);
	fY = fabsf(psNormal->y);
	fZ = fabsf(psNormal->z);
	if ((fX <= fY) && (fX <= fZ))
	{
		sAxis.Init(1, 0, 0);
	}
	else if (fY <= fZ)
	{
		sAxis.Init(0, 1, 0);
	}
	else
	{
		sAxis.Init(0, 0, 1);
	}
	Float32Vec3Cross(psU, &sAxis, psNormal);
	psU->Normalize();
	Float32Vec3Cross(psV, psNormal, psU);
}


//////////////////////////////////////////////////////////////////////////
//
// Each chart's corners laid flat across its normal, in texels: a UV for
// each corner in each chart it's in.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUnwrapFlatten(SMeshUnwrapUVs* psUVs, CMesh* pcMesh, CMeshCharts* pcCharts, float32 fTexelsPerUnit)
{
	CArrayLargeInt	aiCornerChart;  //The chart a corner last had a UV in,
	CArrayLargeInt	aiCornerUV;  //and that UV.
	SFloat32Vec3	sU;
	SFloat32Vec3	sV;
	SFloat32Vec3*	psPosition;
	int*			piCorners;
	int				iChart;
	int				iFace;
	int				iCorner;
	int				i;
	int				k;

	aiCornerChart.Init();
	aiCornerChart.Resize((lsize)pcMesh->NumCorners());
	aiCornerChart.SetArrayValues(-1);
	aiCornerUV.Init();
	aiCornerUV.Resize((lsize)pcMesh->NumCorners());
	psUVs->maiFaceUVs.Resize((lsize)pcMesh->NumFaces() * NUM_FACE_CORNERS);
	psUVs->maiChartStarts.ReInit();
	psUVs->masUVs.ReInit();

	for (iChart = 0; iChart < pcCharts->NumCharts(); iChart++)
	{
		psUVs->maiChartStarts.Add((int)psUVs->masUVs.NumElements());
		MeshUnwrapBasis(pcCharts->masNormals.Get((lsize)iChart), &sU, &sV);
		for (i = 0; i < pcCharts->NumFaces(iChart); i++)
		{
			iFace = pcCharts->GetFace(iChart, i);
			piCorners = pcMesh->GetFaceCorners(iFace);
			for (k = 0; k < NUM_FACE_CORNERS; k++)
			{
				iCorner = piCorners[k];
				if (aiCornerChart.GetValue((lsize)iCorner) != iChart)
				{
					psPosition = pcMesh->GetPosition(iCorner);
					aiCornerChart.SetValue((lsize)iCorner, iChart);
					aiCornerUV.SetValue((lsize)iCorner, (int)psUVs->masUVs.NumElements());
					psUVs->masUVs.Add()->Init(Float32Vec3Dot(psPosition, &sU) * fTexelsPerUnit, Float32Vec3Dot(psPosition, &sV) * fTexelsPerUnit);
				}
				psUVs->maiFaceUVs.SetValue(((lsize)iFace * NUM_FACE_CORNERS) + (lsize)k, aiCornerUV.GetValue((lsize)iCorner));
			}
		}
	}
	psUVs->maiChartStarts.Add((int)psUVs->masUVs.NumElements());

	aiCornerUV.Kill();
	aiCornerChart.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The charts whose faces overlap each other laid flat.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUnwrapOverlapping(CArrayInt* paiDest, SMeshUnwrapUVs* psUVs, CMeshCharts* pcCharts)
{
	CArrayLargeInt	aiCorners;
	int				iChart;
	int				iFace;
	int				i;
	int				k;

	paiDest->ReInit();
	aiCorners.Init();
	for (iChart = 0; iChart < pcCharts->NumCharts(); iChart++)
	{
		if (pcCharts->NumFaces(iChart) < 2)
		{
			continue;
		}
		aiCorners.ReInit();
		for (i = 0; i < pcCharts->NumFaces(iChart); i++)
		{
			iFace = pcCharts->GetFace(iChart, i);
			for (k = 0; k < NUM_FACE_CORNERS; k++)
			{
				aiCorners.Add(psUVs->maiFaceUVs.GetValue(((lsize)iFace * NUM_FACE_CORNERS) + (lsize)k));
			}
		}
		if (CountUVTriangleOverlaps(psUVs->masUVs.Get(0), aiCorners.Get(0), pcCharts->NumFaces(iChart)) > 0)
		{
			paiDest->Add(iChart);
		}
	}
	aiCorners.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each chart turned to whichever of MESH_UNWRAP_TURNS turns across a right
// angle gives the smallest bounding rectangle, moved to start at 0, 0,
// and its size rounded up to whole texels.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUnwrapOrient(SMeshUnwrapUVs* psUVs, int iNumCharts)
{
	SFloat32Vec2*	psUV;
	float32			fCos;
	float32			fSin;
	float32			fBestCos;
	float32			fBestSin;
	float32			fBestArea;
	float32			fArea;
	float32			fU;
	float32			fV;
	float32			fMinU;
	float32			fMinV;
	float32			fMaxU;
	float32			fMaxV;
	int				iChart;
	int				iTurn;
	int				i;
	int				iFirst;
	int				iLast;

	psUVs->maiSizes.Resize((lsize)iNumCharts * 2);
	for (iChart = 0; iChart < iNumCharts; iChart++)
	{
		iFirst = psUVs->maiChartStarts.GetValue((lsize)iChart);
		iLast = psUVs->maiChartStarts.GetValue((lsize)iChart + 1);
		fBestCos = 1.0f;
		fBestSin = 0.0f;
		fBestArea = -1.0f;
		fMinU = 0.0f;
		fMaxU = 0.0f;
		fMinV = 0.0f;
		fMaxV = 0.0f;
		for (iTurn = 0; iTurn < MESH_UNWRAP_TURNS; iTurn++)
		{
			fCos = cosf((float32)iTurn * 1.5707963f / MESH_UNWRAP_TURNS);
			fSin = sinf((float32)iTurn * 1.5707963f / MESH_UNWRAP_TURNS);
			for (i = iFirst; i < iLast; i++)
			{
				psUV = psUVs->masUVs.Get((lsize)i);
				fU = (psUV->x * fCos) - (psUV->y * fSin);
				fV = (psUV->x * fSin) + (psUV->y * fCos);
				if (i == iFirst)
				{
					fMinU = fU;
					fMaxU = fU;
					fMinV = fV;
					fMaxV = fV;
				}
				fMinU = (fU < fMinU) ? fU : fMinU;
				fMaxU = (fU > fMaxU) ? fU : fMaxU;
				fMinV = (fV < fMinV) ? fV : fMinV;
				fMaxV = (fV > fMaxV) ? fV : fMaxV;
			}
			fArea = (fMaxU - fMinU) * (fMaxV - fMinV);
			if ((fBestArea < 0.0f) || (fArea < fBestArea))
			{
				fBestArea = fArea;
				fBestCos = fCos;
				fBestSin = fSin;
			}
		}

		for (i = iFirst; i < iLast; i++)
		{
			psUV = psUVs->masUVs.Get((lsize)i);
			fU = (psUV->x * fBestCos) - (psUV->y * fBestSin);
			fV = (psUV->x * fBestSin) + (psUV->y * fBestCos);
			psUV->Init(fU, fV);
			if (i == iFirst)
			{
				fMinU = fU;
				fMaxU = fU;
				fMinV = fV;
				fMaxV = fV;
			}
			fMinU = (fU < fMinU) ? fU : fMinU;
			fMaxU = (fU > fMaxU) ? fU : fMaxU;
			fMinV = (fV < fMinV) ? fV : fMinV;
			fMaxV = (fV > fMaxV) ? fV : fMaxV;
		}
		for (i = iFirst; i < iLast; i++)
		{
			psUV = psUVs->masUVs.Get((lsize)i);
			psUV->Init(psUV->x - fMinU, psUV->y - fMinV);
		}
		psUVs->maiSizes.SetValue((lsize)iChart * 2, (int)ceilf(fMaxU - fMinU));
		psUVs->maiSizes.SetValue(((lsize)iChart * 2) + 1, (int)ceilf(fMaxV - fMinV));
		if (psUVs->maiSizes.GetValue((lsize)iChart * 2) < 1)
		{
			psUVs->maiSizes.SetValue((lsize)iChart * 2, 1);
		}
		if (psUVs->maiSizes.GetValue(((lsize)iChart * 2) + 1) < 1)
		{
			psUVs->maiSizes.SetValue(((lsize)iChart * 2) + 1, 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Where a rectangle iWidth by iHeight goes on the skyline (each step its
// x, its height and its width, left to right, across iLayoutWidth): the
// step it starts at whose highest under it is lowest, the leftmost of
// those.  It's then put there and the skyline raised under it.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUnwrapPlace(CArrayInt* paiSkyline, int iLayoutWidth, int iWidth, int iHeight, int* piX, int* piY)
{
	int		iNumSteps;
	int		i;
	int		j;
	int		x;
	int		y;
	int		iBestX;
	int		iBestY;
	int		iRight;
	int		iStepX;
	int		iStepY;
	int		iStepWidth;
	CArrayInt	aiNew;

	iNumSteps = (int)paiSkyline->NumElements() / 3;
	iBestX = 0;
	iBestY = -1;
	for (i = 0; i < iNumSteps; i++)
	{
		x = paiSkyline->GetValue((size)(i * 3));
		if (x + iWidth > iLayoutWidth)
		{
			break;
		}
		y = 0;
		for (j = i; j < iNumSteps; j++)
		{
			if (paiSkyline->GetValue((size)(j * 3)) >= x + iWidth)
			{
				break;
			}
			if (paiSkyline->GetValue((size)((j * 3) + 1)) > y)
			{
				y = paiSkyline->GetValue((size)((j * 3) + 1));
			}
		}
		if ((iBestY == -1) || (y < iBestY))
		{
			iBestX = x;
			iBestY = y;
		}
	}
	*piX = iBestX;
	*piY = iBestY;

	//The steps left of it, it, what's left of a step it covers part of, and the steps right of it.
	aiNew.Init();
	iRight = iBestX + iWidth;
	for (i = 0; i < iNumSteps; i++)
	{
		iStepX = paiSkyline->GetValue((size)(i * 3));
		iStepY = paiSkyline->GetValue((size)((i * 3) + 1));
		iStepWidth = paiSkyline->GetValue((size)((i * 3) + 2));
		if (iStepX + iStepWidth <= iBestX)
		{
			aiNew.Add(iStepX);
			aiNew.Add(iStepY);
			aiNew.Add(iStepWidth);
		}
		if (iStepX == iBestX)
		{
			aiNew.Add(iBestX);
			aiNew.Add(iBestY + iHeight);
			aiNew.Add(iWidth);
		}
		if ((iStepX + iStepWidth > iRight) && (iStepX < iRight))
		{
			aiNew.Add(iRight);
			aiNew.Add(iStepY);
			aiNew.Add(iStepX + iStepWidth - iRight);
		}
		else if (iStepX >= iRight)
		{
			aiNew.Add(iStepX);
			aiNew.Add(iStepY);
			aiNew.Add(iStepWidth);
		}
	}
	paiSkyline->ReInit();
	for (i = 0; i < (int)aiNew.NumElements(); i++)
	{
		paiSkyline->Add(aiNew.GetValue((size)i));
	}
	aiNew.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The charts' rectangles, a gutter round each, tallest first along a
// skyline across a layout as wide as the square root of their area (or
// the widest), its sides in whole pages.  Returns the layout's size.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUnwrapPack(SMeshUnwrapUVs* psUVs, int iNumCharts, int iGutter, int iPageSize, int* piWidth, int* piHeight)
{
	CArrayLargeInt	aiOrder;
	CArrayInt		aiSkyline;
	float64			fArea;
	int				iWidest;
	int				iChart;
	int				iGutters;
	int				iLayoutWidth;
	int				iLayoutHeight;
	int				i;
	int				j;
	int				x;
	int				y;

	iGutters = 2 * iGutter;
	fArea = 0.0;
	iWidest = 0;
	for (iChart = 0; iChart < iNumCharts; iChart++)
	{
		x = psUVs->maiSizes.GetValue((lsize)iChart * 2) + iGutters;
		y = psUVs->maiSizes.GetValue(((lsize)iChart * 2) + 1) + iGutters;
		fArea += (float64)x * y;
		iWidest = (x > iWidest) ? x : iWidest;
	}
	iLayoutWidth = (int)ceil(sqrt(fArea));
	iLayoutWidth = (iWidest > iLayoutWidth) ? iWidest : iLayoutWidth;
	iLayoutWidth = ((iLayoutWidth + iPageSize - 1) / iPageSize) * iPageSize;

	aiOrder.Init();
	for (iChart = 0; iChart < iNumCharts; iChart++)
	{
		aiOrder.Add(iChart);
	}
	for (i = 1; i < iNumCharts; i++)
	{
		iChart = aiOrder.GetValue((lsize)i);
		j = i - 1;
		while ((j >= 0) && (psUVs->maiSizes.GetValue(((lsize)aiOrder.GetValue((lsize)j) * 2) + 1) < psUVs->maiSizes.GetValue(((lsize)iChart * 2) + 1)))
		{
			aiOrder.SetValue((lsize)j + 1, aiOrder.GetValue((lsize)j));
			j--;
		}
		aiOrder.SetValue((lsize)(j + 1), iChart);
	}

	psUVs->maiPlaces.Resize((lsize)iNumCharts * 2);
	aiSkyline.Init();
	aiSkyline.Add(0);
	aiSkyline.Add(0);
	aiSkyline.Add(iLayoutWidth);
	iLayoutHeight = 0;
	for (i = 0; i < iNumCharts; i++)
	{
		iChart = aiOrder.GetValue((lsize)i);
		MeshUnwrapPlace(&aiSkyline, iLayoutWidth, psUVs->maiSizes.GetValue((lsize)iChart * 2) + iGutters, psUVs->maiSizes.GetValue(((lsize)iChart * 2) + 1) + iGutters, &x, &y);
		psUVs->maiPlaces.SetValue((lsize)iChart * 2, x);
		psUVs->maiPlaces.SetValue(((lsize)iChart * 2) + 1, y);
		y += psUVs->maiSizes.GetValue(((lsize)iChart * 2) + 1) + iGutters;
		iLayoutHeight = (y > iLayoutHeight) ? y : iLayoutHeight;
	}
	*piWidth = iLayoutWidth;
	*piHeight = ((iLayoutHeight + iPageSize - 1) / iPageSize) * iPageSize;
	aiSkyline.Kill();
	aiOrder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A copy of the mesh with UV layer 0 made again: charts, laid flat (and
// those that overlap themselves grown again smaller until none do),
// turned, packed, and each UV its place across the layout.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshUnwrap::Modify(Ptr<CMesh> pMesh)
{
	Ptr<CMesh>		pOutput;
	CMeshCharts		cCharts;
	SMeshUnwrapUVs	sUVs;
	CMeshUVLayer*	pcLayer;
	SMeshUVFace*	psUVFace;
	SFloat32Vec2*	psUV;
	CArrayInt		aiOverlapping;
	float32			fAngle;
	int				iRegrow;
	int				iNumCharts;
	int				iChart;
	int				iFace;
	int				i;
	int				k;
	float32			fX;
	float32			fY;

	if ((pMesh->NumFaces() == 0) || (mfTexelsPerUnit <= 0.0f))
	{
		return pMesh;
	}

	cCharts.Init();
	if (meCharts == MUC_Grown)
	{
		cCharts.Grow(&pMesh, mfChartAngle);
	}
	else
	{
		cCharts.MakeBox(&pMesh);
	}
	sUVs.Init();
	aiOverlapping.Init();
	fAngle = mfChartAngle;
	for (iRegrow = 0; iRegrow <= MESH_UNWRAP_REGROWS; iRegrow++)
	{
		MeshUnwrapFlatten(&sUVs, &pMesh, &cCharts, mfTexelsPerUnit);
		MeshUnwrapOverlapping(&aiOverlapping, &sUVs, &cCharts);
		if ((aiOverlapping.NumElements() == 0) || (iRegrow == MESH_UNWRAP_REGROWS))
		{
			break;
		}
		fAngle *= 0.5f;
		cCharts.Regrow(&pMesh, &aiOverlapping, fAngle);
	}
	aiOverlapping.Kill();
	iNumCharts = cCharts.NumCharts();
	MeshUnwrapOrient(&sUVs, iNumCharts);
	MeshUnwrapPack(&sUVs, iNumCharts, miGutter, miPageSize, &miLayoutWidth, &miLayoutHeight);
	miNumCharts = iNumCharts;

	pOutput = OMalloc<CMesh>();
	pOutput->CopyFrom(&pMesh);
	pOutput->mcUVs.mbInUse = true;
	pcLayer = pOutput->mcUVs.EnsureLayerExists(0, pOutput->GetConnectivity());
	pcLayer->mcUVs.ReInit();
	for (iChart = 0; iChart < iNumCharts; iChart++)
	{
		fX = (float32)(sUVs.maiPlaces.GetValue((lsize)iChart * 2) + miGutter);
		fY = (float32)(sUVs.maiPlaces.GetValue(((lsize)iChart * 2) + 1) + miGutter);
		for (i = sUVs.maiChartStarts.GetValue((lsize)iChart); i < sUVs.maiChartStarts.GetValue((lsize)iChart + 1); i++)
		{
			psUV = sUVs.masUVs.Get((lsize)i);
			pcLayer->mcUVs.Add()->Init((psUV->x + fX) / (float32)miLayoutWidth, (psUV->y + fY) / (float32)miLayoutHeight);
		}
	}
	for (iFace = 0; iFace < pMesh->NumFaces(); iFace++)
	{
		psUVFace = pcLayer->mcFaces.Get((lsize)iFace);
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			psUVFace->aiUVs[k] = sUVs.maiFaceUVs.GetValue(((lsize)iFace * NUM_FACE_CORNERS) + (lsize)k);
		}
	}

	sUVs.Kill();
	cCharts.Kill();
	return pOutput;
}

