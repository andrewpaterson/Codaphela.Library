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
#include "MeshUVConformal.h"
#include "BitmapPacker.h"
#include "MeshUnwrap.h"


//The turns a chart's bounding rectangle is tried at, across a right angle, for the smallest.
#define MESH_UNWRAP_TURNS	90

//How many times charts that fold or overlap themselves laid flat are grown again, at half the angle each time.  A chart of
//one face can't, which is where halving ends.
#define MESH_UNWRAP_REGROWS	12


//Each chart's UVs in texels, while a layout is made: added chart by chart, so a chart's are maiChartStarts[c] to [c + 1].
struct SMeshUnwrapUVs
{
	CArrayLargeFloat32Vec2	masUVs;
	CArrayLargeInt			maiUVCorners;  //The corner each UV is of.
	CArrayLargeInt			maiChartStarts;
	CArrayLargeInt			maiFaceUVs;  //Three for each face.
	CArrayLargeInt			maiSizes;  //Each chart's width and height in texels, once turned.
	CArrayLargeInt			maiPlaces;  //Where its UVs start in the layout, in texels.

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
	maiUVCorners.Init();
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
	maiUVCorners.Kill();
	masUVs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshUnwrap::Init(float32 fTexelsPerUnit, EMeshUnwrapCharts eCharts, EMeshUnwrapFlatten eFlatten)
{
	PreInit();
	CMeshModifier::Init();
	mfTexelsPerUnit = fTexelsPerUnit;
	meCharts = eCharts;
	meFlatten = eFlatten;
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
	U_Enum(meFlatten);
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
	psUVs->maiUVCorners.ReInit();

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
					psUVs->maiUVCorners.Add(iCorner);
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
// A chart's faces' corners, three for each, as indices into its own UVs.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUnwrapChartCorners(CArrayLargeInt* paiDest, SMeshUnwrapUVs* psUVs, CMeshCharts* pcCharts, int iChart)
{
	int		iFirst;
	int		iFace;
	int		i;
	int		k;

	paiDest->ReInit();
	iFirst = psUVs->maiChartStarts.GetValue((lsize)iChart);
	for (i = 0; i < pcCharts->NumFaces(iChart); i++)
	{
		iFace = pcCharts->GetFace(iChart, i);
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			paiDest->Add(psUVs->maiFaceUVs.GetValue(((lsize)iFace * NUM_FACE_CORNERS) + (lsize)k) - iFirst);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each chart laid flat again by LSCM, from where it's laid across its
// normal.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUnwrapConformal(SMeshUnwrapUVs* psUVs, CMesh* pcMesh, CMeshCharts* pcCharts, float32 fTexelsPerUnit)
{
	CArrayLargeInt			aiCorners;
	CArrayLargeFloat32Vec3	asPositions;
	int						iChart;
	int						iFirst;
	int						iLast;
	int						i;

	aiCorners.Init();
	asPositions.Init();
	for (iChart = 0; iChart < pcCharts->NumCharts(); iChart++)
	{
		iFirst = psUVs->maiChartStarts.GetValue((lsize)iChart);
		iLast = psUVs->maiChartStarts.GetValue((lsize)iChart + 1);
		asPositions.ReInit();
		for (i = iFirst; i < iLast; i++)
		{
			asPositions.Add(pcMesh->GetPosition(psUVs->maiUVCorners.GetValue((lsize)i)));
		}
		MeshUnwrapChartCorners(&aiCorners, psUVs, pcCharts, iChart);
		FlattenConformal(psUVs->masUVs.Get((lsize)iFirst), asPositions.Get(0), iLast - iFirst, aiCorners.Get(0), pcCharts->NumFaces(iChart), fTexelsPerUnit);
	}
	asPositions.Kill();
	aiCorners.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The charts with faces turned over or overlapping each other laid flat.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUnwrapFolded(CArrayInt* paiDest, SMeshUnwrapUVs* psUVs, CMeshCharts* pcCharts)
{
	CArrayLargeInt	aiCorners;
	SFloat32Vec2*	pasUVs;
	int				iChart;
	int				iNumFaces;

	paiDest->ReInit();
	aiCorners.Init();
	for (iChart = 0; iChart < pcCharts->NumCharts(); iChart++)
	{
		MeshUnwrapChartCorners(&aiCorners, psUVs, pcCharts, iChart);
		pasUVs = psUVs->masUVs.Get((lsize)psUVs->maiChartStarts.GetValue((lsize)iChart));
		iNumFaces = pcCharts->NumFaces(iChart);
		if ((CountUVTrianglesTurned(pasUVs, aiCorners.Get(0), iNumFaces) > 0) || (CountUVTriangleOverlaps(pasUVs, aiCorners.Get(0), iNumFaces) > 0))
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
// How many cells iCell texels across iTexels take, at least one.
//
//////////////////////////////////////////////////////////////////////////
static int MeshUnwrapCells(int iTexels, int iCell)
{
	return (iTexels > iCell) ? (iTexels + iCell - 1) / iCell : 1;
}


//////////////////////////////////////////////////////////////////////////
//
// fU into the range pfMin to pfMax (empty until pbAny).
//
//////////////////////////////////////////////////////////////////////////
static void MeshUnwrapInclude(float32 fU, float32* pfMin, float32* pfMax, bool* pbAny)
{
	if (!*pbAny || (fU < *pfMin))
	{
		*pfMin = fU;
	}
	if (!*pbAny || (fU > *pfMax))
	{
		*pfMax = fU;
	}
	*pbAny = true;
}


//////////////////////////////////////////////////////////////////////////
//
// The cells iCell texels square a UV triangle covers any of: in each row
// of cells, from the least to the most u of its corners in the row's band
// of v and of where its sides cross the band's edges.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUnwrapCoverCells(CBitRows* pcCells, SFloat32Vec2* pasCorners, int iCell)
{
	SFloat32Vec2*	psA;
	SFloat32Vec2*	psB;
	float32			afEdges[2];
	float32			fMinV;
	float32			fMaxV;
	float32			fMinU;
	float32			fMaxU;
	int				iRow;
	int				iLastRow;
	int				iLastColumn;
	int				x;
	int				k;
	int				e;
	bool			bAny;

	bAny = false;
	for (k = 0; k < NUM_FACE_CORNERS; k++)
	{
		MeshUnwrapInclude(pasCorners[k].y, &fMinV, &fMaxV, &bAny);
	}
	iLastRow = (int)(fMaxV / iCell);
	iLastRow = (iLastRow < pcCells->miHeight) ? iLastRow : pcCells->miHeight - 1;
	for (iRow = (int)(fMinV / iCell); iRow <= iLastRow; iRow++)
	{
		afEdges[0] = (float32)(iRow * iCell);
		afEdges[1] = (float32)((iRow + 1) * iCell);
		bAny = false;
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			psA = &pasCorners[k];
			psB = &pasCorners[(k + 1) % NUM_FACE_CORNERS];
			if ((psA->y >= afEdges[0]) && (psA->y <= afEdges[1]))
			{
				MeshUnwrapInclude(psA->x, &fMinU, &fMaxU, &bAny);
			}
			for (e = 0; e < 2; e++)
			{
				if (((psA->y < afEdges[e]) && (psB->y > afEdges[e])) || ((psA->y > afEdges[e]) && (psB->y < afEdges[e])))
				{
					MeshUnwrapInclude(psA->x + ((psB->x - psA->x) * ((afEdges[e] - psA->y) / (psB->y - psA->y))), &fMinU, &fMaxU, &bAny);
				}
			}
		}
		if (bAny)
		{
			iLastColumn = (int)(fMaxU / iCell);
			iLastColumn = (iLastColumn < pcCells->miWidth) ? iLastColumn : pcCells->miWidth - 1;
			for (x = (int)(fMinU / iCell); x <= iLastColumn; x++)
			{
				pcCells->Set(x, iRow);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A UV of a chart fWidth by fHeight (from 0, 0) turned iTurn quarters
// anticlockwise, still from 0, 0.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUnwrapTurned(SFloat32Vec2* psDest, SFloat32Vec2* psUV, int iTurn, float32 fWidth, float32 fHeight)
{
	if (iTurn == 1)
	{
		psDest->Init(fHeight - psUV->y, psUV->x);
	}
	else if (iTurn == 2)
	{
		psDest->Init(fWidth - psUV->x, fHeight - psUV->y);
	}
	else if (iTurn == 3)
	{
		psDest->Init(psUV->y, fWidth - psUV->x);
	}
	else
	{
		*psDest = *psUV;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The cells iCell texels square a chart's triangles cover, grown by a
// cell all round for its gutter: the chart's UVs (from 0, 0) start a cell
// in.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUnwrapChartCells(CBitRows* pcDest, SMeshUnwrapUVs* psUVs, CMeshCharts* pcCharts, int iChart, int iCell)
{
	CArrayLargeInt	aiCorners;
	CBitRows		cCovered;
	SFloat32Vec2*	pasUVs;
	SFloat32Vec2	asCorners[NUM_FACE_CORNERS];
	int				iTriangle;
	int				k;

	cCovered.Init(MeshUnwrapCells(psUVs->maiSizes.GetValue((lsize)iChart * 2), iCell), MeshUnwrapCells(psUVs->maiSizes.GetValue(((lsize)iChart * 2) + 1), iCell));
	aiCorners.Init();
	MeshUnwrapChartCorners(&aiCorners, psUVs, pcCharts, iChart);
	pasUVs = psUVs->masUVs.Get((lsize)psUVs->maiChartStarts.GetValue((lsize)iChart));
	for (iTriangle = 0; iTriangle < pcCharts->NumFaces(iChart); iTriangle++)
	{
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			asCorners[k] = pasUVs[aiCorners.GetValue(((lsize)iTriangle * NUM_FACE_CORNERS) + (lsize)k)];
		}
		MeshUnwrapCoverCells(&cCovered, asCorners, iCell);
	}
	aiCorners.Kill();

	cCovered.Grow(pcDest);
	cCovered.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A chart's UVs turned iTurn quarters anticlockwise within its whole
// cells, as its cells were turned, and its size with them.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUnwrapTurn(SMeshUnwrapUVs* psUVs, int iChart, int iTurn, int iCell)
{
	SFloat32Vec2*	psUV;
	int				iWidth;
	int				iHeight;
	int				i;

	iWidth = MeshUnwrapCells(psUVs->maiSizes.GetValue((lsize)iChart * 2), iCell) * iCell;
	iHeight = MeshUnwrapCells(psUVs->maiSizes.GetValue(((lsize)iChart * 2) + 1), iCell) * iCell;
	for (i = psUVs->maiChartStarts.GetValue((lsize)iChart); i < psUVs->maiChartStarts.GetValue((lsize)iChart + 1); i++)
	{
		psUV = psUVs->masUVs.Get((lsize)i);
		MeshUnwrapTurned(psUV, psUV, iTurn, (float32)iWidth, (float32)iHeight);
	}
	if ((iTurn % 2) == 1)
	{
		psUVs->maiSizes.SetValue((lsize)iChart * 2, iHeight);
		psUVs->maiSizes.SetValue(((lsize)iChart * 2) + 1, iWidth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each chart's cells (the cells its triangles cover, a gutter round
// them) at each quarter turn, biggest first, where they first fit
// (CBitmapPacker) across a layout as wide as the square root of their
// area (or the narrowest way of the widest), its sides in whole pages;
// the turn that reaches least high is kept.  Places are where each
// chart's UVs start, in texels.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUnwrapPack(SMeshUnwrapUVs* psUVs, CMeshCharts* pcCharts, int iCell, int iPageSize, int* piWidth, int* piHeight)
{
	CArrayLargeTemplate<CBitRows>	acCells;  //Each chart's, at each of four turns.
	CBitRows*		pacCells;
	CArrayLargeInt	aiOrder;
	CArrayLargeInt	aiCounts;
	CBitmapPacker	cPacker;
	CBitRows*		pcCells;
	float64			fArea;
	int				iNumCharts;
	int				iNarrowest;
	int				iWidth;
	int				iChart;
	int				iTurn;
	int				iBest;
	int				aiX[4];
	int				aiY[4];
	int				i;
	int				j;
	int				x;

	iNumCharts = pcCharts->NumCharts();
	acCells.Init();
	acCells.Resize((lsize)iNumCharts * 4);
	pacCells = acCells.GetData();
	aiCounts.Init();
	fArea = 0.0;
	iNarrowest = 1;
	for (iChart = 0; iChart < iNumCharts; iChart++)
	{
		pcCells = &pacCells[iChart * 4];
		MeshUnwrapChartCells(pcCells, psUVs, pcCharts, iChart, iCell);
		for (iTurn = 1; iTurn < 4; iTurn++)
		{
			pcCells[iTurn - 1].Turn(&pcCells[iTurn]);
		}
		aiCounts.Add(pcCells->Count());
		fArea += aiCounts.GetValue((lsize)iChart);
		x = (pcCells->miWidth < pcCells->miHeight) ? pcCells->miWidth : pcCells->miHeight;
		iNarrowest = (x > iNarrowest) ? x : iNarrowest;
	}
	iWidth = (int)ceil(sqrt(fArea) * iCell);
	iWidth = ((iNarrowest * iCell) > iWidth) ? (iNarrowest * iCell) : iWidth;
	iWidth = ((iWidth + iPageSize - 1) / iPageSize) * iPageSize;

	aiOrder.Init();
	for (i = 0; i < iNumCharts; i++)
	{
		iChart = i;
		j = i - 1;
		while ((j >= 0) && (aiCounts.GetValue((lsize)aiOrder.GetValue((lsize)j)) < aiCounts.GetValue((lsize)iChart)))
		{
			j--;
		}
		aiOrder.InsertAt(iChart, (lsize)(j + 1));
	}

	psUVs->maiPlaces.Resize((lsize)iNumCharts * 2);
	cPacker.Init(iWidth / iCell);
	for (i = 0; i < iNumCharts; i++)
	{
		iChart = aiOrder.GetValue((lsize)i);
		iBest = -1;
		for (iTurn = 0; iTurn < 4; iTurn++)
		{
			if (cPacker.Find(&pacCells[(iChart * 4) + iTurn], &aiX[iTurn], &aiY[iTurn]) && ((iBest < 0) || ((aiY[iTurn] + pacCells[(iChart * 4) + iTurn].miHeight) < (aiY[iBest] + pacCells[(iChart * 4) + iBest].miHeight))))
			{
				iBest = iTurn;
			}
		}
		MeshUnwrapTurn(psUVs, iChart, iBest, iCell);
		cPacker.Add(&pacCells[(iChart * 4) + iBest], aiX[iBest], aiY[iBest]);
		psUVs->maiPlaces.SetValue((lsize)iChart * 2, (aiX[iBest] + 1) * iCell);
		psUVs->maiPlaces.SetValue(((lsize)iChart * 2) + 1, (aiY[iBest] + 1) * iCell);
	}
	*piWidth = iWidth;
	*piHeight = (((cPacker.GetHeight() * iCell) + iPageSize - 1) / iPageSize) * iPageSize;

	cPacker.Kill();
	aiOrder.Kill();
	aiCounts.Kill();
	for (i = 0; i < iNumCharts * 4; i++)
	{
		pacCells[i].Kill();
	}
	acCells.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A copy of the mesh with UV layer 0 made again: charts, laid flat (and
// those that fold or overlap themselves grown again smaller until none
// do), turned, packed, and each UV its place across the layout.
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
	CArrayInt		aiFolded;
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
	aiFolded.Init();
	fAngle = mfChartAngle;
	for (iRegrow = 0; iRegrow <= MESH_UNWRAP_REGROWS; iRegrow++)
	{
		MeshUnwrapFlatten(&sUVs, &pMesh, &cCharts, mfTexelsPerUnit);
		if (meFlatten == MUF_Conformal)
		{
			MeshUnwrapConformal(&sUVs, &pMesh, &cCharts, mfTexelsPerUnit);
		}
		MeshUnwrapFolded(&aiFolded, &sUVs, &cCharts);
		if ((aiFolded.NumElements() == 0) || (iRegrow == MESH_UNWRAP_REGROWS))
		{
			break;
		}
		fAngle *= 0.5f;
		cCharts.Regrow(&pMesh, &aiFolded, fAngle);
	}
	aiFolded.Kill();
	iNumCharts = cCharts.NumCharts();
	MeshUnwrapOrient(&sUVs, iNumCharts);
	MeshUnwrapPack(&sUVs, &cCharts, (miGutter > 0) ? miGutter : 1, miPageSize, &miLayoutWidth, &miLayoutHeight);
	miNumCharts = iNumCharts;

	pOutput = OMalloc<CMesh>();
	pOutput->CopyFrom(&pMesh);
	pOutput->mcUVs.mbInUse = true;
	pcLayer = pOutput->mcUVs.EnsureLayerExists(0, pOutput->GetConnectivity());
	pcLayer->mcUVs.ReInit();
	pcLayer->miLayoutWidth = miLayoutWidth;
	pcLayer->miLayoutHeight = miLayoutHeight;
	for (iChart = 0; iChart < iNumCharts; iChart++)
	{
		fX = (float32)sUVs.maiPlaces.GetValue((lsize)iChart * 2);
		fY = (float32)sUVs.maiPlaces.GetValue(((lsize)iChart * 2) + 1);
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

