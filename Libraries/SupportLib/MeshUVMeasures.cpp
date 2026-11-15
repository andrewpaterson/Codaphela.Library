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
#include "MeshUVMeasures.h"


//The most cells a side of CountMeshUVOverlaps' grid has.
#define MESH_UV_OVERLAP_CELLS	1024


//////////////////////////////////////////////////////////////////////////
//
// The layer, or NULL if the mesh hasn't it.
//
//////////////////////////////////////////////////////////////////////////
static CMeshUVLayer* MeshUVLayer(CMesh* pcMesh, int iLayer)
{
	if (!pcMesh->mcUVs.InUse() || (iLayer < 0) || ((size)iLayer >= pcMesh->mcUVs.mcLayers.NumElements()))
	{
		return NULL;
	}
	return pcMesh->mcUVs.mcLayers.Get((size)iLayer);
}


//////////////////////////////////////////////////////////////////////////
//
// The face's three UVs, or false if it's missing any.
//
//////////////////////////////////////////////////////////////////////////
static bool MeshFaceUVs(CMeshUVLayer* pcLayer, int iFace, SFloat32Vec2** papsUVs)
{
	SMeshUVFace*	psUVFace;
	int				k;

	if ((lsize)iFace >= pcLayer->mcFaces.NumElements())
	{
		return false;
	}
	psUVFace = pcLayer->mcFaces.Get((lsize)iFace);
	for (k = 0; k < NUM_FACE_CORNERS; k++)
	{
		if ((psUVFace->aiUVs[k] < 0) || ((lsize)psUVFace->aiUVs[k] >= pcLayer->mcUVs.NumElements()))
		{
			return false;
		}
		papsUVs[k] = pcLayer->mcUVs.Get((lsize)psUVFace->aiUVs[k]);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Twice the UV triangle's area, positive anticlockwise with u right and v
// up.
//
//////////////////////////////////////////////////////////////////////////
static float64 MeshUVTwiceArea(SFloat32Vec2** papsUVs)
{
	return (((float64)papsUVs[1]->x - papsUVs[0]->x) * ((float64)papsUVs[2]->y - papsUVs[0]->y)) - (((float64)papsUVs[2]->x - papsUVs[0]->x) * ((float64)papsUVs[1]->y - papsUVs[0]->y));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CountMeshUVFacesMissing(CMesh* pcMesh, int iLayer)
{
	CMeshUVLayer*	pcLayer;
	SFloat32Vec2*	apsUVs[NUM_FACE_CORNERS];
	int				iFace;
	int				iMissing;

	pcLayer = MeshUVLayer(pcMesh, iLayer);
	if (pcLayer == NULL)
	{
		return pcMesh->NumFaces();
	}
	iMissing = 0;
	for (iFace = 0; iFace < pcMesh->NumFaces(); iFace++)
	{
		if (!MeshFaceUVs(pcLayer, iFace, apsUVs))
		{
			iMissing++;
		}
	}
	return iMissing;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CountMeshUVFacesTurned(CMesh* pcMesh, int iLayer)
{
	CMeshUVLayer*	pcLayer;
	SFloat32Vec2*	apsUVs[NUM_FACE_CORNERS];
	int				iFace;
	int				iTurned;

	pcLayer = MeshUVLayer(pcMesh, iLayer);
	if (pcLayer == NULL)
	{
		return 0;
	}
	iTurned = 0;
	for (iFace = 0; iFace < pcMesh->NumFaces(); iFace++)
	{
		if (MeshFaceUVs(pcLayer, iFace, apsUVs) && (MeshUVTwiceArea(apsUVs) <= 0.0))
		{
			iTurned++;
		}
	}
	return iTurned;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CountMeshUVsOutside(CMesh* pcMesh, int iLayer)
{
	CMeshUVLayer*	pcLayer;
	SFloat32Vec2*	psUV;
	lsize			u;
	int				iOutside;

	pcLayer = MeshUVLayer(pcMesh, iLayer);
	if (pcLayer == NULL)
	{
		return 0;
	}
	iOutside = 0;
	for (u = 0; u < pcLayer->mcUVs.NumElements(); u++)
	{
		psUV = pcLayer->mcUVs.Get(u);
		if ((psUV->x < 0.0f) || (psUV->x > 1.0f) || (psUV->y < 0.0f) || (psUV->y > 1.0f))
		{
			iOutside++;
		}
	}
	return iOutside;
}


//////////////////////////////////////////////////////////////////////////
//
// The separating axis test: two triangles' insides overlap unless, along
// the line across one of their six edges, one lies all on one side of the
// other, or touching it (within fTolerance).
//
//////////////////////////////////////////////////////////////////////////
static bool MeshUVTrianglesOverlap(SFloat32Vec2** papsA, SFloat32Vec2** papsB, float64 fTolerance)
{
	SFloat32Vec2**	papsEdges;
	float64			fAcrossX;
	float64			fAcrossY;
	float64			fValue;
	float64			fMinA;
	float64			fMaxA;
	float64			fMinB;
	float64			fMaxB;
	int				iTriangle;
	int				iEdge;
	int				k;

	for (iTriangle = 0; iTriangle < 2; iTriangle++)
	{
		papsEdges = (iTriangle == 0) ? papsA : papsB;
		for (iEdge = 0; iEdge < 3; iEdge++)
		{
			fAcrossX = -((float64)papsEdges[(iEdge + 1) % 3]->y - papsEdges[iEdge]->y);
			fAcrossY = (float64)papsEdges[(iEdge + 1) % 3]->x - papsEdges[iEdge]->x;
			if ((fAcrossX == 0.0) && (fAcrossY == 0.0))
			{
				continue;
			}
			fMinA = (fAcrossX * papsA[0]->x) + (fAcrossY * papsA[0]->y);
			fMaxA = fMinA;
			fMinB = (fAcrossX * papsB[0]->x) + (fAcrossY * papsB[0]->y);
			fMaxB = fMinB;
			for (k = 1; k < 3; k++)
			{
				fValue = (fAcrossX * papsA[k]->x) + (fAcrossY * papsA[k]->y);
				fMinA = (fValue < fMinA) ? fValue : fMinA;
				fMaxA = (fValue > fMaxA) ? fValue : fMaxA;
				fValue = (fAcrossX * papsB[k]->x) + (fAcrossY * papsB[k]->y);
				fMinB = (fValue < fMinB) ? fValue : fMinB;
				fMaxB = (fValue > fMaxB) ? fValue : fMaxB;
			}
			fValue = fTolerance * sqrt((fAcrossX * fAcrossX) + (fAcrossY * fAcrossY));
			if ((fMaxA <= fMinB + fValue) || (fMaxB <= fMinA + fValue))
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A triangle's three UVs.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUVTriangle(SFloat32Vec2* pasUVs, int* paiCorners, int iTriangle, SFloat32Vec2** papsUVs)
{
	int		k;

	for (k = 0; k < NUM_FACE_CORNERS; k++)
	{
		papsUVs[k] = &pasUVs[paiCorners[(iTriangle * NUM_FACE_CORNERS) + k]];
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The triangles with area are put in the cells of a grid over their UVs
// their bounding boxes cover, so only triangles in a cell together are
// compared.  A pair is counted in the first cell both cover, so once.
//
//////////////////////////////////////////////////////////////////////////
int CountUVTriangleOverlaps(SFloat32Vec2* pasUVs, int* paiCorners, int iNumTriangles)
{
	SFloat32Vec2*	apsUVs[NUM_FACE_CORNERS];
	SFloat32Vec2*	apsOther[NUM_FACE_CORNERS];
	CArrayLargeInt	aiFaces;  //The triangles with area.
	CArrayLargeInt	aiBoxes;  //Each one's cells, x0, y0, x1, y1.
	CArrayLargeInt	aiCellStarts;
	CArrayLargeInt	aiCellFaces;
	CArrayLargeInt	aiFilled;
	float64			fMinU;
	float64			fMinV;
	float64			fMaxU;
	float64			fMaxV;
	float64			fCellU;
	float64			fCellV;
	float64			fTolerance;
	int				iNumFaces;
	int				iCells;
	int				iFace;
	int				i;
	int				j;
	int				k;
	int				x;
	int				y;
	int				iCell;
	int				iOverlaps;
	int*			piBox;
	int*			piOther;
	int				aiBox[4];

	aiFaces.Init();
	fMinU = 1e30;
	fMinV = 1e30;
	fMaxU = -1e30;
	fMaxV = -1e30;
	for (iFace = 0; iFace < iNumTriangles; iFace++)
	{
		MeshUVTriangle(pasUVs, paiCorners, iFace, apsUVs);
		if (MeshUVTwiceArea(apsUVs) != 0.0)
		{
			aiFaces.Add(iFace);
			for (k = 0; k < NUM_FACE_CORNERS; k++)
			{
				fMinU = (apsUVs[k]->x < fMinU) ? apsUVs[k]->x : fMinU;
				fMinV = (apsUVs[k]->y < fMinV) ? apsUVs[k]->y : fMinV;
				fMaxU = (apsUVs[k]->x > fMaxU) ? apsUVs[k]->x : fMaxU;
				fMaxV = (apsUVs[k]->y > fMaxV) ? apsUVs[k]->y : fMaxV;
			}
		}
	}
	iNumFaces = (int)aiFaces.NumElements();
	if (iNumFaces < 2)
	{
		aiFaces.Kill();
		return 0;
	}

	iCells = (int)sqrt((float64)iNumFaces);
	iCells = (iCells < 1) ? 1 : ((iCells > MESH_UV_OVERLAP_CELLS) ? MESH_UV_OVERLAP_CELLS : iCells);
	fCellU = ((fMaxU - fMinU) > 0.0) ? ((fMaxU - fMinU) / iCells) : 1.0;
	fCellV = ((fMaxV - fMinV) > 0.0) ? ((fMaxV - fMinV) / iCells) : 1.0;
	fTolerance = ((fMaxU - fMinU) + (fMaxV - fMinV)) * 1e-7;

	aiBoxes.Init();
	aiBoxes.Resize((lsize)iNumFaces * 4);
	aiCellStarts.Init();
	aiCellStarts.Resize(((lsize)iCells * iCells) + 1);
	aiCellStarts.SetArrayValues(0);
	for (i = 0; i < iNumFaces; i++)
	{
		MeshUVTriangle(pasUVs, paiCorners, aiFaces.GetValue((lsize)i), apsUVs);
		aiBox[0] = (int)((apsUVs[0]->x - fMinU) / fCellU);
		aiBox[1] = (int)((apsUVs[0]->y - fMinV) / fCellV);
		aiBox[2] = aiBox[0];
		aiBox[3] = aiBox[1];
		for (k = 1; k < NUM_FACE_CORNERS; k++)
		{
			x = (int)((apsUVs[k]->x - fMinU) / fCellU);
			y = (int)((apsUVs[k]->y - fMinV) / fCellV);
			aiBox[0] = (x < aiBox[0]) ? x : aiBox[0];
			aiBox[1] = (y < aiBox[1]) ? y : aiBox[1];
			aiBox[2] = (x > aiBox[2]) ? x : aiBox[2];
			aiBox[3] = (y > aiBox[3]) ? y : aiBox[3];
		}
		for (k = 0; k < 4; k++)
		{
			aiBox[k] = (aiBox[k] >= iCells) ? (iCells - 1) : aiBox[k];
			aiBoxes.SetValue(((lsize)i * 4) + (lsize)k, aiBox[k]);
		}
		for (y = aiBox[1]; y <= aiBox[3]; y++)
		{
			for (x = aiBox[0]; x <= aiBox[2]; x++)
			{
				iCell = (y * iCells) + x + 1;
				aiCellStarts.SetValue((lsize)iCell, aiCellStarts.GetValue((lsize)iCell) + 1);
			}
		}
	}
	for (iCell = 1; iCell <= iCells * iCells; iCell++)
	{
		aiCellStarts.SetValue((lsize)iCell, aiCellStarts.GetValue((lsize)iCell) + aiCellStarts.GetValue((lsize)(iCell - 1)));
	}

	aiCellFaces.Init();
	aiCellFaces.Resize((lsize)aiCellStarts.GetValue((lsize)(iCells * iCells)));
	aiFilled.Init();
	aiFilled.Resize((lsize)(iCells * iCells));
	aiFilled.SetArrayValues(0);
	for (i = 0; i < iNumFaces; i++)
	{
		piBox = aiBoxes.Get((lsize)i * 4);
		for (y = piBox[1]; y <= piBox[3]; y++)
		{
			for (x = piBox[0]; x <= piBox[2]; x++)
			{
				iCell = (y * iCells) + x;
				aiCellFaces.SetValue((lsize)(aiCellStarts.GetValue((lsize)iCell) + aiFilled.GetValue((lsize)iCell)), i);
				aiFilled.SetValue((lsize)iCell, aiFilled.GetValue((lsize)iCell) + 1);
			}
		}
	}

	iOverlaps = 0;
	for (iCell = 0; iCell < iCells * iCells; iCell++)
	{
		x = iCell % iCells;
		y = iCell / iCells;
		for (i = aiCellStarts.GetValue((lsize)iCell); i < aiCellStarts.GetValue((lsize)(iCell + 1)); i++)
		{
			piBox = aiBoxes.Get((lsize)aiCellFaces.GetValue((lsize)i) * 4);
			for (j = i + 1; j < aiCellStarts.GetValue((lsize)(iCell + 1)); j++)
			{
				piOther = aiBoxes.Get((lsize)aiCellFaces.GetValue((lsize)j) * 4);
				if ((((piBox[0] > piOther[0]) ? piBox[0] : piOther[0]) != x) || (((piBox[1] > piOther[1]) ? piBox[1] : piOther[1]) != y))
				{
					continue;
				}
				MeshUVTriangle(pasUVs, paiCorners, aiFaces.GetValue((lsize)aiCellFaces.GetValue((lsize)i)), apsUVs);
				MeshUVTriangle(pasUVs, paiCorners, aiFaces.GetValue((lsize)aiCellFaces.GetValue((lsize)j)), apsOther);
				if (MeshUVTrianglesOverlap(apsUVs, apsOther, fTolerance))
				{
					iOverlaps++;
				}
			}
		}
	}

	aiFilled.Kill();
	aiCellFaces.Kill();
	aiCellStarts.Kill();
	aiBoxes.Kill();
	aiFaces.Kill();
	return iOverlaps;
}


//////////////////////////////////////////////////////////////////////////
//
// The faces with all their UVs, as triangles of the layer's UVs.
//
//////////////////////////////////////////////////////////////////////////
int CountMeshUVOverlaps(CMesh* pcMesh, int iLayer)
{
	CMeshUVLayer*	pcLayer;
	SFloat32Vec2*	apsUVs[NUM_FACE_CORNERS];
	CArrayLargeInt	aiCorners;
	SMeshUVFace*	psUVFace;
	int				iFace;
	int				k;
	int				iOverlaps;

	pcLayer = MeshUVLayer(pcMesh, iLayer);
	if ((pcLayer == NULL) || (pcLayer->mcUVs.NumElements() == 0))
	{
		return 0;
	}
	aiCorners.Init();
	for (iFace = 0; iFace < pcMesh->NumFaces(); iFace++)
	{
		if (MeshFaceUVs(pcLayer, iFace, apsUVs))
		{
			psUVFace = pcLayer->mcFaces.Get((lsize)iFace);
			for (k = 0; k < NUM_FACE_CORNERS; k++)
			{
				aiCorners.Add(psUVFace->aiUVs[k]);
			}
		}
	}
	iOverlaps = 0;
	if (aiCorners.NumElements() > 0)
	{
		iOverlaps = CountUVTriangleOverlaps(pcLayer->mcUVs.Get(0), aiCorners.Get(0), (int)(aiCorners.NumElements() / NUM_FACE_CORNERS));
	}
	aiCorners.Kill();
	return iOverlaps;
}


//////////////////////////////////////////////////////////////////////////
//
// Each face is laid in its own plane (its first side along x), and the
// map from there to its texels is the 2 by 2 matrix J taking its two
// sides to its UV sides.  J's singular values are (Q + R) and |Q - R|,
// Q and R the lengths of (E, H) and (F, G), E = (a + d) / 2, F = (a - d)
// / 2, G = (c + b) / 2, H = (c - b) / 2 for J = (a, b; c, d).
//
//////////////////////////////////////////////////////////////////////////
void MeasureMeshUVStretch(SMeshUVStretch* psDest, CMesh* pcMesh, int iLayer, float32 fLayoutWidth, float32 fLayoutHeight, float32 fTexelsPerUnit)
{
	CMeshUVLayer*	pcLayer;
	SFloat32Vec2*	apsUVs[NUM_FACE_CORNERS];
	int*			piCorners;
	SFloat32Vec3	sSide1;
	SFloat32Vec3	sSide2;
	float64			fLength1;
	float64			fX2;
	float64			fY2;
	float64			fU1;
	float64			fV1;
	float64			fU2;
	float64			fV2;
	float64			fDeterminant;
	float64			a;
	float64			b;
	float64			c;
	float64			d;
	float64			fQ;
	float64			fR;
	float64			fMost;
	float64			fLeast;
	float64			fRatio;
	float64			fArea;
	float64			fDensity;
	float64			fWeighted;
	float64			fTotalArea;
	int				iFace;

	psDest->fWorstAngle = 1.0f;
	psDest->fMeanAngle = 1.0f;
	psDest->fLeastArea = 0.0f;
	psDest->fMostArea = 0.0f;
	psDest->iNumFaces = 0;
	pcLayer = MeshUVLayer(pcMesh, iLayer);
	if (pcLayer == NULL)
	{
		return;
	}

	fDensity = (float64)fTexelsPerUnit * fTexelsPerUnit;
	fWeighted = 0.0;
	fTotalArea = 0.0;
	for (iFace = 0; iFace < pcMesh->NumFaces(); iFace++)
	{
		if (!MeshFaceUVs(pcLayer, iFace, apsUVs))
		{
			continue;
		}
		piCorners = pcMesh->GetFaceCorners(iFace);
		sSide1 = *pcMesh->GetPosition(piCorners[1]) - *pcMesh->GetPosition(piCorners[0]);
		sSide2 = *pcMesh->GetPosition(piCorners[2]) - *pcMesh->GetPosition(piCorners[0]);
		fLength1 = sSide1.Magnitude();
		if (fLength1 <= 0.0)
		{
			continue;
		}
		fX2 = Float32Vec3Dot(&sSide2, &sSide1) / fLength1;
		fY2 = (float64)sSide2.SquareMagnitude() - (fX2 * fX2);
		if (fY2 <= 0.0)
		{
			continue;
		}
		fY2 = sqrt(fY2);
		if (fY2 <= 0.0)
		{
			continue;
		}

		fU1 = ((float64)apsUVs[1]->x - apsUVs[0]->x) * fLayoutWidth;
		fV1 = ((float64)apsUVs[1]->y - apsUVs[0]->y) * fLayoutHeight;
		fU2 = ((float64)apsUVs[2]->x - apsUVs[0]->x) * fLayoutWidth;
		fV2 = ((float64)apsUVs[2]->y - apsUVs[0]->y) * fLayoutHeight;
		fDeterminant = (fU1 * fV2) - (fU2 * fV1);
		if (fDeterminant == 0.0)
		{
			continue;
		}

		//J (fLength1, 0) = (fU1, fV1) and J (fX2, fY2) = (fU2, fV2).
		a = fU1 / fLength1;
		c = fV1 / fLength1;
		b = (fU2 - (a * fX2)) / fY2;
		d = (fV2 - (c * fX2)) / fY2;
		fQ = sqrt((((a + d) * 0.5) * ((a + d) * 0.5)) + (((c - b) * 0.5) * ((c - b) * 0.5)));
		fR = sqrt((((a - d) * 0.5) * ((a - d) * 0.5)) + (((c + b) * 0.5) * ((c + b) * 0.5)));
		fMost = fQ + fR;
		fLeast = fabs(fQ - fR);
		if (fLeast <= 0.0)
		{
			continue;
		}

		fRatio = fMost / fLeast;
		fArea = (fMost * fLeast) / fDensity;
		if ((psDest->iNumFaces == 0) || (fRatio > psDest->fWorstAngle))
		{
			psDest->fWorstAngle = (float32)fRatio;
		}
		if ((psDest->iNumFaces == 0) || (fArea < psDest->fLeastArea))
		{
			psDest->fLeastArea = (float32)fArea;
		}
		if ((psDest->iNumFaces == 0) || (fArea > psDest->fMostArea))
		{
			psDest->fMostArea = (float32)fArea;
		}
		fWeighted += fRatio * fLength1 * fY2;
		fTotalArea += fLength1 * fY2;
		psDest->iNumFaces++;
	}
	if (fTotalArea > 0.0)
	{
		psDest->fMeanAngle = (float32)(fWeighted / fTotalArea);
	}
}

