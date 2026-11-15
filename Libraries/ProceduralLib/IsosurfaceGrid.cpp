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
#include "IsosurfaceGrid.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsosurfaceInside(float32 fDistance)
{
	return fDistance < 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
// The size over the cell size is rounded up, less a little, so a box
// whose sides are whole cells has no cell more.
//
//////////////////////////////////////////////////////////////////////////
static int IsosurfaceNumCells(float32 fSize, float32 fCellSize)
{
	int		iCells;

	iCells = (int)ceilf((fSize / fCellSize) - 0.0001f);
	if (iCells < 1)
	{
		iCells = 1;
	}
	return iCells;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIsosurfaceGrid::Init(SFloat32Bounds* psBounds, float32 fCellSize)
{
	SFloat32Vec3	sSize;
	SFloat32Vec3	sCenter;

	psBounds->GetSize(&sSize);
	psBounds->GetCenter(&sCenter);
	mfCellSize = fCellSize;
	miCellsX = IsosurfaceNumCells(sSize.x, fCellSize);
	miCellsY = IsosurfaceNumCells(sSize.y, fCellSize);
	miCellsZ = IsosurfaceNumCells(sSize.z, fCellSize);
	msOrigin.Init(sCenter.x - ((float32)miCellsX * fCellSize * 0.5f), sCenter.y - ((float32)miCellsY * fCellSize * 0.5f), sCenter.z - ((float32)miCellsZ * fCellSize * 0.5f));

	mafDistances.Init();
	maiCrossings.Init();
	masCrossings.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIsosurfaceGrid::Kill(void)
{
	masCrossings.Kill();
	maiCrossings.Kill();
	mafDistances.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIsosurfaceGrid::Sample(CShape* pcShape)
{
	int						x;
	int						y;
	int						z;
	int						iAxis;
	int						aiNext[3];
	lsize					uiCorner;
	lsize					uiNext;
	lsize					uiNumCorners;
	float32					fDistance;
	float32					fNextDistance;
	SFloat32Vec3			sPosition;
	SFloat32Vec3			sNext;
	SIsosurfaceCrossing*	psCrossing;

	uiNumCorners = (lsize)(miCellsX + 1) * (lsize)(miCellsY + 1) * (lsize)(miCellsZ + 1);
	mafDistances.Resize(uiNumCorners);
	maiCrossings.Resize(uiNumCorners * 3);
	maiCrossings.SetArrayValues(-1);
	masCrossings.ReInit();

	for (z = 0; z <= miCellsZ; z++)
	{
		for (y = 0; y <= miCellsY; y++)
		{
			for (x = 0; x <= miCellsX; x++)
			{
				GetCornerPosition(&sPosition, x, y, z);
				mafDistances.SetValue(CornerIndex(x, y, z), pcShape->Distance(&sPosition));
			}
		}
	}

	for (z = 0; z <= miCellsZ; z++)
	{
		for (y = 0; y <= miCellsY; y++)
		{
			for (x = 0; x <= miCellsX; x++)
			{
				uiCorner = CornerIndex(x, y, z);
				fDistance = mafDistances.GetValue(uiCorner);
				for (iAxis = 0; iAxis < 3; iAxis++)
				{
					aiNext[0] = x + (iAxis == 0 ? 1 : 0);
					aiNext[1] = y + (iAxis == 1 ? 1 : 0);
					aiNext[2] = z + (iAxis == 2 ? 1 : 0);
					if ((aiNext[0] <= miCellsX) && (aiNext[1] <= miCellsY) && (aiNext[2] <= miCellsZ))
					{
						uiNext = CornerIndex(aiNext[0], aiNext[1], aiNext[2]);
						fNextDistance = mafDistances.GetValue(uiNext);
						if (IsosurfaceInside(fDistance) != IsosurfaceInside(fNextDistance))
						{
							GetCornerPosition(&sPosition, x, y, z);
							GetCornerPosition(&sNext, aiNext[0], aiNext[1], aiNext[2]);
							maiCrossings.SetValue((uiCorner * 3) + (lsize)iAxis, (int)masCrossings.NumElements());
							psCrossing = masCrossings.Add();
							IsosurfaceFindCrossing(psCrossing, pcShape, &sPosition, fDistance, &sNext, fNextDistance, mfCellSize * 1e-6f);
						}
					}
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The Illinois method: the secant through the two ends of the bracket,
// whose end that stays twice has its distance halved, so a curve bent
// one way can't hold one end still.  The bracket always holds a change of
// IsosurfaceInside, so an end exactly on the surface (zero, outside) is
// where the crossing is.  The normal is the shape's gradient there, or
// the edge's direction, inside to outside, where the shape has none.
//
//////////////////////////////////////////////////////////////////////////
void IsosurfaceFindCrossing(SIsosurfaceCrossing* psDest, CShape* pcShape, SFloat32Vec3* psStart, float32 fStartDistance, SFloat32Vec3* psEnd, float32 fEndDistance, float32 fTolerance)
{
	SFloat32Vec3	sAlong;
	float32			fA;
	float32			fB;
	float32			fDistanceA;
	float32			fDistanceB;
	float32			fT;
	float32			fDistance;
	float32			fLength;
	bool			bInsideA;
	int				iLastMoved;
	int				i;

	sAlong = *psEnd - *psStart;
	fA = 0.0f;
	fB = 1.0f;
	fDistanceA = fStartDistance;
	fDistanceB = fEndDistance;
	bInsideA = IsosurfaceInside(fStartDistance);
	iLastMoved = 0;
	fT = 0.0f;
	for (i = 0; i < ISOSURFACE_CROSSING_STEPS; i++)
	{
		fT = ((fA * fDistanceB) - (fB * fDistanceA)) / (fDistanceB - fDistanceA);
		psDest->sPosition = *psStart + (sAlong * fT);
		fDistance = pcShape->Distance(&psDest->sPosition);
		if (fabsf(fDistance) <= fTolerance)
		{
			break;
		}

		if (IsosurfaceInside(fDistance) == bInsideA)
		{
			fA = fT;
			fDistanceA = fDistance;
			if (iLastMoved == -1)
			{
				fDistanceB *= 0.5f;
			}
			iLastMoved = -1;
		}
		else
		{
			fB = fT;
			fDistanceB = fDistance;
			if (iLastMoved == 1)
			{
				fDistanceA *= 0.5f;
			}
			iLastMoved = 1;
		}
	}

	pcShape->Gradient(&psDest->sNormal, &psDest->sPosition);
	fLength = psDest->sNormal.Magnitude();
	if (fLength > 0.0f)
	{
		psDest->sNormal /= fLength;
	}
	else
	{
		psDest->sNormal = bInsideA ? sAlong : -sAlong;
		psDest->sNormal /= sAlong.Magnitude();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CIsosurfaceGrid::CornerIndex(int x, int y, int z)
{
	return (lsize)x + ((lsize)(miCellsX + 1) * ((lsize)y + ((lsize)(miCellsY + 1) * (lsize)z)));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIsosurfaceGrid::GetCornerPosition(SFloat32Vec3* psDest, int x, int y, int z)
{
	psDest->Init(msOrigin.x + ((float32)x * mfCellSize), msOrigin.y + ((float32)y * mfCellSize), msOrigin.z + ((float32)z * mfCellSize));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CIsosurfaceGrid::GetCornerDistance(int x, int y, int z)
{
	return mafDistances.GetValue(CornerIndex(x, y, z));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIsosurfaceCrossing* CIsosurfaceGrid::GetCrossing(int x, int y, int z, int iAxis)
{
	int		iCrossing;

	iCrossing = maiCrossings.GetValue((CornerIndex(x, y, z) * 3) + (lsize)iAxis);
	if (iCrossing == -1)
	{
		return NULL;
	}
	return masCrossings.Get((lsize)iCrossing);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIsosurfaceGrid::NumCrossings(void)
{
	return (int)masCrossings.NumElements();
}

