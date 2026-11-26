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
#include "MarchingCubes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarchingCubes::Init(void)
{
	int		iInside;

	for (iInside = 0; iInside < MARCHING_CUBES_CASES; iInside++)
	{
		MakeCase(&masCases[iInside], iInside, 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMarchingCubes::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// A face of the cube is the corners whose bit iAxis is iSide, in the
// order its other two axes (u and v, which with iAxis are right handed)
// go anticlockwise seen from outside: from the far side, (0, 0), (1, 0),
// (1, 1), (0, 1), and the other way from the near side.  Along that walk
// a crossing goes into the inside or comes out of it, and they take
// turns, so each going in is joined to the next coming out, which keeps
// two inside corners across a face from each other apart.  A face whose
// bit (iAxis * 2) + iSide is set in iJoinedFaces joins each going in to
// the last coming out, which joins them.  Either way the cube on the other
// side of the face must do the same.  An edge is
// on two faces, walked one way on each, so it is where one face's line
// ends and the other's starts, and the lines make loops.  A loop runs
// anticlockwise seen from outside (an inside corner alone makes x, y, z),
// so its fan does too.
//
//////////////////////////////////////////////////////////////////////////
void CMarchingCubes::MakeCase(SMarchingCubesCase* psCase, int iInside, int iJoinedFaces)
{
	static const int	aaiNearOrder[4][2] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
	static const int	aaiFarOrder[4][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
	const				int			(*paaiOrder)[2];
	int					aiNext[MARCHING_CUBES_EDGES];
	int					aiCorners[4];
	int					aiLoop[MARCHING_CUBES_EDGES];
	bool				abUsed[MARCHING_CUBES_EDGES];
	int					aiEvents[4];
	bool				abGoesIn[4];
	int					iNumEvents;
	int					iAxis;
	int					iSide;
	int					iU;
	int					iV;
	int					i;
	int					j;
	int					iEdge;
	int					iLoopLength;
	bool				bInside;
	bool				bNextInside;

	for (i = 0; i < MARCHING_CUBES_EDGES; i++)
	{
		aiNext[i] = -1;
		abUsed[i] = false;
	}

	for (iAxis = 0; iAxis < 3; iAxis++)
	{
		iU = (iAxis + 1) % 3;
		iV = (iAxis + 2) % 3;
		for (iSide = 0; iSide < 2; iSide++)
		{
			paaiOrder = (iSide == 1) ? aaiFarOrder : aaiNearOrder;
			for (i = 0; i < 4; i++)
			{
				aiCorners[i] = (iSide << iAxis) | (paaiOrder[i][0] << iU) | (paaiOrder[i][1] << iV);
			}

			iNumEvents = 0;
			for (i = 0; i < 4; i++)
			{
				bInside = (iInside & (1 << aiCorners[i])) != 0;
				bNextInside = (iInside & (1 << aiCorners[(i + 1) % 4])) != 0;
				if (bInside != bNextInside)
				{
					aiEvents[iNumEvents] = CornersEdge(aiCorners[i], aiCorners[(i + 1) % 4]);
					abGoesIn[iNumEvents] = bNextInside;
					iNumEvents++;
				}
			}

			for (i = 0; i < iNumEvents; i++)
			{
				if (abGoesIn[i])
				{
					if (iJoinedFaces & (1 << ((iAxis * 2) + iSide)))
					{
						aiNext[aiEvents[i]] = aiEvents[(i + iNumEvents - 1) % iNumEvents];
					}
					else
					{
						aiNext[aiEvents[i]] = aiEvents[(i + 1) % iNumEvents];
					}
				}
			}
		}
	}

	psCase->iNumFaces = 0;
	psCase->iNumPieces = 0;
	for (iEdge = 0; iEdge < MARCHING_CUBES_EDGES; iEdge++)
	{
		psCase->aiEdgePieces[iEdge] = -1;
	}
	for (iEdge = 0; iEdge < MARCHING_CUBES_EDGES; iEdge++)
	{
		if ((aiNext[iEdge] == -1) || abUsed[iEdge])
		{
			continue;
		}

		iLoopLength = 0;
		i = iEdge;
		while ((i != -1) && !abUsed[i])
		{
			abUsed[i] = true;
			psCase->aiEdgePieces[i] = (int8)psCase->iNumPieces;
			aiLoop[iLoopLength] = i;
			iLoopLength++;
			i = aiNext[i];
		}
		psCase->iNumPieces++;

		for (j = 1; j < iLoopLength - 1; j++)
		{
			psCase->aaiFaces[psCase->iNumFaces][0] = (int8)aiLoop[0];
			psCase->aaiFaces[psCase->iNumFaces][1] = (int8)aiLoop[j];
			psCase->aaiFaces[psCase->iNumFaces][2] = (int8)aiLoop[j + 1];
			psCase->iNumFaces++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The case of a cube with these distances at its corners, its ambiguous
// faces decided by the asymptotic decider (Nielson and Hamann 1991): on a
// face whose corners go inside, outside, inside, outside, the two inside
// corners are joined where the face's bilinear interpolation is inside
// at its saddle point, (v0 v2 - v1 v3) / (v0 + v2 - v1 - v3).  It depends
// on the face's corners alone, so the cubes either side agree.
//
//////////////////////////////////////////////////////////////////////////
void CMarchingCubes::DecideCase(SMarchingCubesCase* psCase, float32* pafDistances)
{
	int		iInside;
	int		iJoinedFaces;
	int		iCorner;
	int		iAxis;
	int		iSide;
	int		iU;
	int		iV;
	int		aiCorners[4];
	float32	afValues[4];
	float32	fDenominator;
	float32	fSaddle;
	int		i;

	iInside = 0;
	for (iCorner = 0; iCorner < MARCHING_CUBES_CORNERS; iCorner++)
	{
		if (IsosurfaceInside(pafDistances[iCorner]))
		{
			iInside |= 1 << iCorner;
		}
	}

	iJoinedFaces = 0;
	for (iAxis = 0; iAxis < 3; iAxis++)
	{
		iU = (iAxis + 1) % 3;
		iV = (iAxis + 2) % 3;
		for (iSide = 0; iSide < 2; iSide++)
		{
			aiCorners[0] = (iSide << iAxis);
			aiCorners[1] = (iSide << iAxis) | (1 << iU);
			aiCorners[2] = (iSide << iAxis) | (1 << iU) | (1 << iV);
			aiCorners[3] = (iSide << iAxis) | (1 << iV);
			for (i = 0; i < 4; i++)
			{
				afValues[i] = pafDistances[aiCorners[i]];
			}
			if ((IsosurfaceInside(afValues[0]) == IsosurfaceInside(afValues[2])) && (IsosurfaceInside(afValues[1]) == IsosurfaceInside(afValues[3])) && (IsosurfaceInside(afValues[0]) != IsosurfaceInside(afValues[1])))
			{
				fDenominator = afValues[0] + afValues[2] - afValues[1] - afValues[3];
				if (fDenominator != 0.0f)
				{
					fSaddle = ((afValues[0] * afValues[2]) - (afValues[1] * afValues[3])) / fDenominator;
					if (IsosurfaceInside(fSaddle))
					{
						iJoinedFaces |= 1 << ((iAxis * 2) + iSide);
					}
				}
			}
		}
	}
	MakeCase(psCase, iInside, iJoinedFaces);
}


//////////////////////////////////////////////////////////////////////////
//
// One corner for each crossing, then each cell's case's triangles on the
// crossings of its edges.
//
//////////////////////////////////////////////////////////////////////////
void CMarchingCubes::Generate(CMeshEditor* pcMeshEditor, CIsosurfaceGrid* pcGrid, int iFaceName)
{
	SMarchingCubesCase*	psCase;
	int					iFirstCorner;
	int					i;
	int					x;
	int					y;
	int					z;
	int					iCorner;
	int					iInside;
	int					iFace;
	int					iEdge;
	int					iStart;
	int					aiFaceCorners[3];

	iFirstCorner = pcMeshEditor->NumCorners();
	for (i = 0; i < pcGrid->NumCrossings(); i++)
	{
		pcMeshEditor->AddCorner(&pcGrid->masCrossings.Get((lsize)i)->sPosition);
	}

	for (z = 0; z < pcGrid->miCellsZ; z++)
	{
		for (y = 0; y < pcGrid->miCellsY; y++)
		{
			for (x = 0; x < pcGrid->miCellsX; x++)
			{
				iInside = 0;
				for (iCorner = 0; iCorner < MARCHING_CUBES_CORNERS; iCorner++)
				{
					if (IsosurfaceInside(pcGrid->GetCornerDistance(x + (iCorner & 1), y + ((iCorner >> 1) & 1), z + ((iCorner >> 2) & 1))))
					{
						iInside |= 1 << iCorner;
					}
				}

				psCase = &masCases[iInside];
				for (iFace = 0; iFace < psCase->iNumFaces; iFace++)
				{
					for (i = 0; i < 3; i++)
					{
						iEdge = psCase->aaiFaces[iFace][i];
						iStart = EdgeStartCorner(iEdge);
						aiFaceCorners[i] = iFirstCorner + pcGrid->maiCrossings.GetValue((pcGrid->CornerIndex(x + (iStart & 1), y + ((iStart >> 1) & 1), z + ((iStart >> 2) & 1)) * 3) + (lsize)(iEdge / 4));
					}
					pcMeshEditor->AddFace(aiFaceCorners[0], aiFaceCorners[1], aiFaceCorners[2], iFaceName);
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMarchingCubes::EdgeStartCorner(int iEdge)
{
	int		iAxis;
	int		k;

	iAxis = iEdge / 4;
	k = iEdge % 4;
	return ((k & 1) << ((iAxis + 1) % 3)) | (((k >> 1) & 1) << ((iAxis + 2) % 3));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMarchingCubes::EdgeEndCorner(int iEdge)
{
	return EdgeStartCorner(iEdge) | (1 << (iEdge / 4));
}


//////////////////////////////////////////////////////////////////////////
//
// The edge between two corners that differ in one bit, or -1.
//
//////////////////////////////////////////////////////////////////////////
int CMarchingCubes::CornersEdge(int iCorner1, int iCorner2)
{
	int		iDifference;
	int		iAxis;
	int		iStart;

	iDifference = iCorner1 ^ iCorner2;
	if (iDifference == 1)
	{
		iAxis = 0;
	}
	else if (iDifference == 2)
	{
		iAxis = 1;
	}
	else if (iDifference == 4)
	{
		iAxis = 2;
	}
	else
	{
		return -1;
	}

	iStart = iCorner1 & ~iDifference;
	return (iAxis * 4) + ((iStart >> ((iAxis + 1) % 3)) & 1) + (((iStart >> ((iAxis + 2) % 3)) & 1) << 1);
}

