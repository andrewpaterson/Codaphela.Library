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
#include "BaseLib/Float64SparseMatrix.h"
#include "MeshUVConformal.h"


//////////////////////////////////////////////////////////////////////////
//
// The UV furthest from UV iFrom.
//
//////////////////////////////////////////////////////////////////////////
static int MeshUVConformalFurthest(SFloat32Vec2* pasUVs, int iNumUVs, int iFrom)
{
	SFloat32Vec2	sOffset;
	float32			fDistance;
	float32			fFurthest;
	int				iFurthest;
	int				i;

	iFurthest = iFrom;
	fFurthest = 0.0f;
	for (i = 0; i < iNumUVs; i++)
	{
		sOffset = pasUVs[i] - pasUVs[iFrom];
		fDistance = sOffset.SquareMagnitude();
		if (fDistance > fFurthest)
		{
			fFurthest = fDistance;
			iFurthest = i;
		}
	}
	return iFurthest;
}


//////////////////////////////////////////////////////////////////////////
//
// fValue times a triangle corner's u (or v) into row iRow: into the
// UV's column, or, pinned (no column), its known part moved to b.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUVConformalAdd(CFloat64SparseMatrix* pcMatrix, float64* pafB, int iRow, int iColumn, float64 fPinned, float64 fValue)
{
	if (iColumn >= 0)
	{
		pcMatrix->Add(iRow, iColumn, fValue);
	}
	else
	{
		pafB[iRow] -= fValue * fPinned;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A triangle's two rows.  Laid in its own plane, (x, y) anticlockwise as
// seen from outside, u and v are conformal where the Cauchy-Riemann
// equations hold: du/dx = dv/dy and du/dy = -dv/dx.  A linear function's
// gradient on a triangle is the sum over its corners of each one's value
// times its opposite side e (anticlockwise) turned a right angle, (-ey,
// ex), over twice the area, so the rows are
//
//     sum (-ey u - ex v) = 0
//     sum ( ex u - ey v) = 0
//
// each weighted by the root of the area over twice it, so a triangle
// counts by its area.  One with no area is left out.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUVConformalTriangle(CFloat64SparseMatrix* pcMatrix, float64* pafB, int iTriangle, SFloat32Vec2* pasUVs, SFloat32Vec3* pasPositions, int* paiCorners, int* paiColumns)
{
	SFloat32Vec3	sSide1;
	SFloat32Vec3	sSide2;
	float64			afX[NUM_FACE_CORNERS];
	float64			afY[NUM_FACE_CORNERS];
	float64			fLength1;
	float64			fHeight;
	float64			fWeight;
	float64			fEX;
	float64			fEY;
	int*			piCorners;
	int				iColumn;
	int				iRow;
	int				k;

	piCorners = &paiCorners[iTriangle * NUM_FACE_CORNERS];
	sSide1 = pasPositions[piCorners[1]] - pasPositions[piCorners[0]];
	sSide2 = pasPositions[piCorners[2]] - pasPositions[piCorners[0]];
	fLength1 = sSide1.Magnitude();
	if (fLength1 <= 0.0)
	{
		return;
	}
	afX[0] = 0.0;
	afY[0] = 0.0;
	afX[1] = fLength1;
	afY[1] = 0.0;
	afX[2] = Float32Vec3Dot(&sSide2, &sSide1) / fLength1;
	fHeight = (float64)sSide2.SquareMagnitude() - (afX[2] * afX[2]);
	if (fHeight <= 0.0)
	{
		return;
	}
	afY[2] = sqrt(fHeight);
	fWeight = 1.0 / sqrt(2.0 * fLength1 * afY[2]);

	iRow = iTriangle * 2;
	for (k = 0; k < NUM_FACE_CORNERS; k++)
	{
		fEX = (afX[(k + 2) % NUM_FACE_CORNERS] - afX[(k + 1) % NUM_FACE_CORNERS]) * fWeight;
		fEY = (afY[(k + 2) % NUM_FACE_CORNERS] - afY[(k + 1) % NUM_FACE_CORNERS]) * fWeight;
		iColumn = paiColumns[piCorners[k]];
		MeshUVConformalAdd(pcMatrix, pafB, iRow, iColumn, pasUVs[piCorners[k]].x, -fEY);
		MeshUVConformalAdd(pcMatrix, pafB, iRow, (iColumn >= 0) ? iColumn + 1 : -1, pasUVs[piCorners[k]].y, -fEX);
		MeshUVConformalAdd(pcMatrix, pafB, iRow + 1, iColumn, pasUVs[piCorners[k]].x, fEX);
		MeshUVConformalAdd(pcMatrix, pafB, iRow + 1, (iColumn >= 0) ? iColumn + 1 : -1, pasUVs[piCorners[k]].y, -fEY);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Twice the area of the triangles in UV, anticlockwise positive, and
// in 3D.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUVConformalAreas(SFloat32Vec2* pasUVs, SFloat32Vec3* pasPositions, int* paiCorners, int iNumTriangles, float64* pfUVArea, float64* pfArea)
{
	SFloat32Vec3	sSide1;
	SFloat32Vec3	sSide2;
	SFloat32Vec3	sNormal;
	SFloat32Vec2	sUV1;
	SFloat32Vec2	sUV2;
	int*			piCorners;
	int				iTriangle;

	*pfUVArea = 0.0;
	*pfArea = 0.0;
	for (iTriangle = 0; iTriangle < iNumTriangles; iTriangle++)
	{
		piCorners = &paiCorners[iTriangle * NUM_FACE_CORNERS];
		sUV1 = pasUVs[piCorners[1]] - pasUVs[piCorners[0]];
		sUV2 = pasUVs[piCorners[2]] - pasUVs[piCorners[0]];
		*pfUVArea += ((float64)sUV1.x * sUV2.y) - ((float64)sUV2.x * sUV1.y);
		sSide1 = pasPositions[piCorners[1]] - pasPositions[piCorners[0]];
		sSide2 = pasPositions[piCorners[2]] - pasPositions[piCorners[0]];
		Float32Vec3Cross(&sNormal, &sSide1, &sSide2);
		*pfArea += sNormal.Magnitude();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each UV not pinned is two columns, u then v; each triangle two rows.
//
//////////////////////////////////////////////////////////////////////////
bool FlattenConformal(SFloat32Vec2* pasUVs, SFloat32Vec3* pasPositions, int iNumUVs, int* paiCorners, int iNumTriangles, float32 fTexelsPerUnit)
{
	CFloat64SparseMatrix	cMatrix;
	CArrayLargeInt			aiColumns;
	CArrayLargeFloat64		afX;
	CArrayLargeFloat64		afB;
	CArrayLargeFloat32Vec2	asUVs;
	float64					fUVArea;
	float64					fArea;
	float32					fScale;
	int						iPinA;
	int						iPinB;
	int						iNumColumns;
	int						iColumn;
	int						iTriangle;
	int						i;
	bool					bFlat;

	iPinA = MeshUVConformalFurthest(pasUVs, iNumUVs, 0);
	iPinB = MeshUVConformalFurthest(pasUVs, iNumUVs, iPinA);
	if (iPinA == iPinB)
	{
		return false;
	}

	aiColumns.Init();
	iNumColumns = 0;
	for (i = 0; i < iNumUVs; i++)
	{
		if ((i == iPinA) || (i == iPinB))
		{
			aiColumns.Add(-1);
		}
		else
		{
			aiColumns.Add(iNumColumns);
			iNumColumns += 2;
		}
	}
	afX.Init();
	afX.Resize((lsize)iNumColumns);
	afB.Init();
	afB.Resize((lsize)iNumTriangles * 2);
	afB.SetArrayValues(0.0);
	cMatrix.Init(iNumTriangles * 2, iNumColumns);
	for (iTriangle = 0; iTriangle < iNumTriangles; iTriangle++)
	{
		MeshUVConformalTriangle(&cMatrix, afB.GetData(), iTriangle, pasUVs, pasPositions, paiCorners, aiColumns.GetData());
	}

	bFlat = false;
	if ((iNumColumns == 0) || Float64SparseLeastSquaresDirect(&cMatrix, afX.GetData(), afB.GetData()))
	{
		asUVs.Init();
		for (i = 0; i < iNumUVs; i++)
		{
			iColumn = aiColumns.GetValue((lsize)i);
			if (iColumn >= 0)
			{
				asUVs.Add()->Init((float32)afX.GetValue((lsize)iColumn), (float32)afX.GetValue((lsize)iColumn + 1));
			}
			else
			{
				asUVs.Add(&pasUVs[i]);
			}
		}
		MeshUVConformalAreas(asUVs.Get(0), pasPositions, paiCorners, iNumTriangles, &fUVArea, &fArea);
		if (fUVArea > 0.0)
		{
			fScale = (float32)(fTexelsPerUnit * sqrt(fArea / fUVArea));
			for (i = 0; i < iNumUVs; i++)
			{
				pasUVs[i] = *asUVs.Get((lsize)i) * fScale;
			}
			bFlat = true;
		}
		asUVs.Kill();
	}

	cMatrix.Kill();
	afB.Kill();
	afX.Kill();
	aiColumns.Kill();
	return bFlat;
}

