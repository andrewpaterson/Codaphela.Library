/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ProceduralLib

Codaphela ProceduralLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ProceduralLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ProceduralLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include <string.h>
#include "MarchingCubes.h"
#include "DualContouring.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDualContouring::Init(void)
{
	maiCellCorners.Init();
	masCellPieces.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDualContouring::Kill(void)
{
	masCellPieces.Kill();
	maiCellCorners.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A vertex for each cell with a crossing on any of its edges, then a quad
// for each crossed edge.  The four cells around an edge along axis a are
// taken anticlockwise seen from its far end (u and v, the other two axes,
// make a right handed set with a), so the quad faces along a, which is
// out when the edge starts inside, and is turned round when it doesn't.
// An edge on the grid's side has cells on one side only and makes
// nothing: the grid is to have the surface inside it.
//
//////////////////////////////////////////////////////////////////////////
void CDualContouring::Generate(CMeshEditor* pcMeshEditor, CIsosurfaceGrid* pcGrid, CShape* pcShape, int iFaceName)
{
	static const int	aaiAround[4][2] = {{-1, -1}, {0, -1}, {0, 0}, {-1, 0}};
	int					x;
	int					y;
	int					z;
	int					iAxis;
	int					i;
	int					iCorner;
	int					aiEdge[3];
	int					aiCell[3];
	int					aiCells[3];
	int					aiCorners[4];
	int					iSwap;
	int					iPiece;
	int					iLocalEdge;
	bool				bInGrid;
	lsize				uiNumCells;
	lsize				uiCell;
	SFloat32Vec3		sVertex;
	SMarchingCubesCase	sCase;
	SFloat32Vec3		asNormals[MARCHING_CUBES_EDGES];
	int					iStart;

	uiNumCells = (lsize)pcGrid->miCellsX * (lsize)pcGrid->miCellsY * (lsize)pcGrid->miCellsZ;
	maiCellCorners.Resize(uiNumCells);
	maiCellCorners.SetArrayValues(-1);
	masCellPieces.Resize(uiNumCells);

	iCorner = pcMeshEditor->NumCorners();
	for (z = 0; z < pcGrid->miCellsZ; z++)
	{
		for (y = 0; y < pcGrid->miCellsY; y++)
		{
			for (x = 0; x < pcGrid->miCellsX; x++)
			{
				uiCell = CellIndex(pcGrid, x, y, z);
				CellCase(&sCase, pcGrid, x, y, z);
				for (iLocalEdge = 0; iLocalEdge < MARCHING_CUBES_EDGES; iLocalEdge++)
				{
					if (sCase.aiEdgePieces[iLocalEdge] >= 0)
					{
						iStart = CMarchingCubes::EdgeStartCorner(iLocalEdge);
						asNormals[iLocalEdge] = pcGrid->GetCrossing(x + (iStart & 1), y + ((iStart >> 1) & 1), z + ((iStart >> 2) & 1), iLocalEdge / 4)->sNormal;
					}
				}
				IsosurfaceJoinPieces(&sCase, asNormals);
				memcpy(masCellPieces.Get(uiCell)->aiEdgePieces, sCase.aiEdgePieces, sizeof(sCase.aiEdgePieces));
				if (sCase.iNumPieces > 0)
				{
					maiCellCorners.SetValue(uiCell, iCorner);
					for (iPiece = 0; iPiece < sCase.iNumPieces; iPiece++)
					{
						FitCell(&sVertex, pcGrid, x, y, z, &sCase, iPiece);
						pcMeshEditor->AddCorner(&sVertex);
						iCorner++;
					}
				}
			}
		}
	}

	aiCells[0] = pcGrid->miCellsX;
	aiCells[1] = pcGrid->miCellsY;
	aiCells[2] = pcGrid->miCellsZ;
	for (z = 0; z <= pcGrid->miCellsZ; z++)
	{
		for (y = 0; y <= pcGrid->miCellsY; y++)
		{
			for (x = 0; x <= pcGrid->miCellsX; x++)
			{
				for (iAxis = 0; iAxis < 3; iAxis++)
				{
					aiEdge[0] = x;
					aiEdge[1] = y;
					aiEdge[2] = z;
					if ((aiEdge[iAxis] >= aiCells[iAxis]) || (pcGrid->GetCrossing(x, y, z, iAxis) == NULL))
					{
						continue;
					}

					bInGrid = true;
					for (i = 0; i < 4; i++)
					{
						aiCell[0] = x;
						aiCell[1] = y;
						aiCell[2] = z;
						aiCell[(iAxis + 1) % 3] += aaiAround[i][0];
						aiCell[(iAxis + 2) % 3] += aaiAround[i][1];
						if ((aiCell[0] < 0) || (aiCell[1] < 0) || (aiCell[2] < 0) || (aiCell[0] >= aiCells[0]) || (aiCell[1] >= aiCells[1]) || (aiCell[2] >= aiCells[2]))
						{
							bInGrid = false;
							break;
						}
						uiCell = CellIndex(pcGrid, aiCell[0], aiCell[1], aiCell[2]);
						iLocalEdge = (iAxis * 4) + ((aaiAround[i][0] == -1) ? 1 : 0) + ((aaiAround[i][1] == -1) ? 2 : 0);
						aiCorners[i] = maiCellCorners.GetValue(uiCell) + masCellPieces.Get(uiCell)->aiEdgePieces[iLocalEdge];
					}
					if (!bInGrid)
					{
						continue;
					}

					if (!IsosurfaceInside(pcGrid->GetCornerDistance(x, y, z)))
					{
						iSwap = aiCorners[1];
						aiCorners[1] = aiCorners[3];
						aiCorners[3] = iSwap;
					}
					IsosurfaceAddQuad(pcMeshEditor, pcShape, aiCorners, &pcGrid->GetCrossing(x, y, z, iAxis)->sNormal, iFaceName);
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The cell's marching cubes case, its ambiguous faces decided by their
// corners' distances.
//
//////////////////////////////////////////////////////////////////////////
void CDualContouring::CellCase(SMarchingCubesCase* psDest, CIsosurfaceGrid* pcGrid, int x, int y, int z)
{
	float32	afDistances[MARCHING_CUBES_CORNERS];
	int		iCorner;

	for (iCorner = 0; iCorner < MARCHING_CUBES_CORNERS; iCorner++)
	{
		afDistances[iCorner] = pcGrid->GetCornerDistance(x + (iCorner & 1), y + ((iCorner >> 1) & 1), z + ((iCorner >> 2) & 1));
	}
	CMarchingCubes::DecideCase(psDest, afDistances);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CDualContouring::CellIndex(CIsosurfaceGrid* pcGrid, int x, int y, int z)
{
	return (lsize)x + ((lsize)pcGrid->miCellsX * ((lsize)y + ((lsize)pcGrid->miCellsY * (lsize)z)));
}


//////////////////////////////////////////////////////////////////////////
//
// The planes at the crossings on the cell's edges that are on iPiece of
// its surface, fitted from the cell's centre and kept in the cell.
//
//////////////////////////////////////////////////////////////////////////
void CDualContouring::FitCell(SFloat32Vec3* psDest, CIsosurfaceGrid* pcGrid, int x, int y, int z, SMarchingCubesCase* psCase, int iPiece)
{
	SIsosurfaceFit			sFit;
	SIsosurfaceCrossing*	psCrossing;
	SFloat32Bounds			sCell;
	SFloat32Vec3			sCentre;
	SFloat32Vec3			sHalf;
	int						iEdge;
	int						iStart;

	pcGrid->GetCornerPosition(&sCell.sMinimum, x, y, z);
	pcGrid->GetCornerPosition(&sCell.sMaximum, x + 1, y + 1, z + 1);
	sHalf.Init(pcGrid->mfCellSize * 0.5f, pcGrid->mfCellSize * 0.5f, pcGrid->mfCellSize * 0.5f);
	sCentre = sCell.sMinimum + sHalf;

	sFit.Init(&sCentre);
	for (iEdge = 0; iEdge < MARCHING_CUBES_EDGES; iEdge++)
	{
		if (psCase->aiEdgePieces[iEdge] != iPiece)
		{
			continue;
		}
		iStart = CMarchingCubes::EdgeStartCorner(iEdge);
		psCrossing = pcGrid->GetCrossing(x + (iStart & 1), y + ((iStart >> 1) & 1), z + ((iStart >> 2) & 1), iEdge / 4);
		if (psCrossing != NULL)
		{
			sFit.Add(&psCrossing->sPosition, &psCrossing->sNormal);
		}
	}
	sFit.Solve(psDest, &sCell);
}


//////////////////////////////////////////////////////////////////////////
//
// Each piece's normals added up, then pieces joined (smaller numbers
// taking larger ones) while any two not yet one are closer than two
// sheets, and numbered again from 0.
//
//////////////////////////////////////////////////////////////////////////
void IsosurfaceJoinPieces(SMarchingCubesCase* psCase, SFloat32Vec3* pasEdgeNormals)
{
	SFloat32Vec3	asSums[MARCHING_CUBES_EDGES];
	int				aiJoined[MARCHING_CUBES_EDGES];
	int				aiNumbers[MARCHING_CUBES_EDGES];
	int				iEdge;
	int				iPiece;
	int				i;
	int				j;
	int				iFrom;
	int				iNumPieces;
	float32			fLengths;
	bool			bChanged;

	if (psCase->iNumPieces < 2)
	{
		return;
	}

	for (iPiece = 0; iPiece < psCase->iNumPieces; iPiece++)
	{
		asSums[iPiece].Init(0, 0, 0);
		aiJoined[iPiece] = iPiece;
	}
	for (iEdge = 0; iEdge < MARCHING_CUBES_EDGES; iEdge++)
	{
		if (psCase->aiEdgePieces[iEdge] >= 0)
		{
			asSums[psCase->aiEdgePieces[iEdge]] += pasEdgeNormals[iEdge];
		}
	}

	do
	{
		bChanged = false;
		for (i = 0; i < psCase->iNumPieces; i++)
		{
			for (j = i + 1; j < psCase->iNumPieces; j++)
			{
				if (aiJoined[i] == aiJoined[j])
				{
					continue;
				}
				fLengths = (float32)(asSums[i].Magnitude() * asSums[j].Magnitude());
				if (Float32Vec3Dot(&asSums[i], &asSums[j]) > (ISOSURFACE_SHEETS_COSINE * fLengths))
				{
					iFrom = aiJoined[j];
					for (iPiece = 0; iPiece < psCase->iNumPieces; iPiece++)
					{
						if (aiJoined[iPiece] == iFrom)
						{
							aiJoined[iPiece] = aiJoined[i];
						}
					}
					bChanged = true;
				}
			}
		}
	}
	while (bChanged);

	iNumPieces = 0;
	for (iPiece = 0; iPiece < psCase->iNumPieces; iPiece++)
	{
		aiNumbers[iPiece] = -1;
	}
	for (iPiece = 0; iPiece < psCase->iNumPieces; iPiece++)
	{
		if (aiNumbers[aiJoined[iPiece]] == -1)
		{
			aiNumbers[aiJoined[iPiece]] = iNumPieces;
			iNumPieces++;
		}
	}
	for (iEdge = 0; iEdge < MARCHING_CUBES_EDGES; iEdge++)
	{
		if (psCase->aiEdgePieces[iEdge] >= 0)
		{
			psCase->aiEdgePieces[iEdge] = (int8)aiNumbers[aiJoined[psCase->aiEdgePieces[iEdge]]];
		}
	}
	psCase->iNumPieces = iNumPieces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static float32 IsosurfaceTwiceArea(CMesh* pcMesh, int iCorner1, int iCorner2, int iCorner3)
{
	SFloat32Vec3	sSide1;
	SFloat32Vec3	sSide2;
	SFloat32Vec3	sCross;

	sSide1 = *pcMesh->GetPosition(iCorner2) - *pcMesh->GetPosition(iCorner1);
	sSide2 = *pcMesh->GetPosition(iCorner3) - *pcMesh->GetPosition(iCorner1);
	Float32Vec3Cross(&sCross, &sSide1, &sSide2);
	return (float32)sCross.Magnitude();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsosurfaceFacesAlong(CMesh* pcMesh, int iCorner1, int iCorner2, int iCorner3, SFloat32Vec3* psNormal)
{
	SFloat32Vec3	sSide1;
	SFloat32Vec3	sSide2;
	SFloat32Vec3	sCross;

	sSide1 = *pcMesh->GetPosition(iCorner2) - *pcMesh->GetPosition(iCorner1);
	sSide2 = *pcMesh->GetPosition(iCorner3) - *pcMesh->GetPosition(iCorner1);
	Float32Vec3Cross(&sCross, &sSide1, &sSide2);
	return Float32Vec3Dot(&sCross, psNormal) > 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
// A split with a triangle facing against the crossing's normal folds over
// its neighbours.  Where both diagonals' middles are as near the surface
// (within a thousandth of the quad's longer diagonal), the split whose
// smaller triangle is larger: three corners along one edge of a box make
// a sliver, which could face either way.
//
//////////////////////////////////////////////////////////////////////////
void IsosurfaceAddQuad(CMeshEditor* pcMeshEditor, CShape* pcShape, int* paiCorners, SFloat32Vec3* psNormal, int iFaceName)
{
	SFloat32Vec3	sMiddle02;
	SFloat32Vec3	sMiddle13;
	SFloat32Vec3	sDiagonal;
	CMesh*			pcMesh;
	float32			fDistance02;
	float32			fDistance13;
	float32			fLongest;
	float32			fSmaller02;
	float32			fSmaller13;
	bool			bSplit02;
	bool			bAlong02;
	bool			bAlong13;

	pcMesh = &pcMeshEditor->mpMesh;
	sMiddle02 = (*pcMesh->GetPosition(paiCorners[0]) + *pcMesh->GetPosition(paiCorners[2])) * 0.5f;
	sMiddle13 = (*pcMesh->GetPosition(paiCorners[1]) + *pcMesh->GetPosition(paiCorners[3])) * 0.5f;
	fDistance02 = fabsf(pcShape->Distance(&sMiddle02));
	fDistance13 = fabsf(pcShape->Distance(&sMiddle13));

	sDiagonal = *pcMesh->GetPosition(paiCorners[2]) - *pcMesh->GetPosition(paiCorners[0]);
	fLongest = (float32)sDiagonal.Magnitude();
	sDiagonal = *pcMesh->GetPosition(paiCorners[3]) - *pcMesh->GetPosition(paiCorners[1]);
	fLongest = fmaxf(fLongest, (float32)sDiagonal.Magnitude());

	bAlong02 = IsosurfaceFacesAlong(pcMesh, paiCorners[0], paiCorners[1], paiCorners[2], psNormal) && IsosurfaceFacesAlong(pcMesh, paiCorners[0], paiCorners[2], paiCorners[3], psNormal);
	bAlong13 = IsosurfaceFacesAlong(pcMesh, paiCorners[0], paiCorners[1], paiCorners[3], psNormal) && IsosurfaceFacesAlong(pcMesh, paiCorners[1], paiCorners[2], paiCorners[3], psNormal);
	if (bAlong02 != bAlong13)
	{
		bSplit02 = bAlong02;
	}
	else if (fabsf(fDistance02 - fDistance13) > (fLongest * 0.001f))
	{
		bSplit02 = fDistance02 < fDistance13;
	}
	else
	{
		fSmaller02 = fminf(IsosurfaceTwiceArea(pcMesh, paiCorners[0], paiCorners[1], paiCorners[2]), IsosurfaceTwiceArea(pcMesh, paiCorners[0], paiCorners[2], paiCorners[3]));
		fSmaller13 = fminf(IsosurfaceTwiceArea(pcMesh, paiCorners[0], paiCorners[1], paiCorners[3]), IsosurfaceTwiceArea(pcMesh, paiCorners[1], paiCorners[2], paiCorners[3]));
		bSplit02 = fSmaller02 >= fSmaller13;
	}

	if (bSplit02)
	{
		pcMeshEditor->AddFace(paiCorners[0], paiCorners[1], paiCorners[2], iFaceName);
		pcMeshEditor->AddFace(paiCorners[0], paiCorners[2], paiCorners[3], iFaceName);
	}
	else
	{
		pcMeshEditor->AddFace(paiCorners[0], paiCorners[1], paiCorners[3], iFaceName);
		pcMeshEditor->AddFace(paiCorners[1], paiCorners[2], paiCorners[3], iFaceName);
	}
}

