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
#include "MeshUVRigid.h"


//A triangle laid in its own plane at the texel scale: how each corner's UV makes the map's gradients.  A linear function's
//gradient on a triangle is the sum over its corners of each one's value times its opposite side e (anticlockwise) turned a
//right angle, (-ey, ex), over twice the area.
struct SMeshUVRigidTriangle
{
	float64	afAlongX[NUM_FACE_CORNERS];  //d/dx of the map is the sum of these times the corners' values,
	float64	afAlongY[NUM_FACE_CORNERS];  //d/dy these.
	float64	fWeight;  //The root of its area, so a triangle counts by its area.
};


//////////////////////////////////////////////////////////////////////////
//
// False for a triangle with no area.
//
//////////////////////////////////////////////////////////////////////////
static bool MeshUVRigidTriangle(SMeshUVRigidTriangle* psDest, SFloat32Vec3* pasPositions, int* piCorners, float32 fTexelsPerUnit)
{
	SFloat32Vec3	sSide1;
	SFloat32Vec3	sSide2;
	float64			afX[NUM_FACE_CORNERS];
	float64			afY[NUM_FACE_CORNERS];
	float64			fLength1;
	float64			fHeight;
	float64			fTwiceArea;
	int				k;

	sSide1 = pasPositions[piCorners[1]] - pasPositions[piCorners[0]];
	sSide2 = pasPositions[piCorners[2]] - pasPositions[piCorners[0]];
	fLength1 = sSide1.Magnitude();
	if (fLength1 <= 0.0)
	{
		return false;
	}
	afX[0] = 0.0;
	afY[0] = 0.0;
	afX[1] = fLength1 * fTexelsPerUnit;
	afY[1] = 0.0;
	afX[2] = Float32Vec3Dot(&sSide2, &sSide1) / fLength1;
	fHeight = (float64)sSide2.SquareMagnitude() - (afX[2] * afX[2]);
	if (fHeight <= 0.0)
	{
		return false;
	}
	afX[2] *= fTexelsPerUnit;
	afY[2] = sqrt(fHeight) * fTexelsPerUnit;
	fTwiceArea = afX[1] * afY[2];
	for (k = 0; k < NUM_FACE_CORNERS; k++)
	{
		psDest->afAlongX[k] = -(afY[(k + 2) % NUM_FACE_CORNERS] - afY[(k + 1) % NUM_FACE_CORNERS]) / fTwiceArea;
		psDest->afAlongY[k] = (afX[(k + 2) % NUM_FACE_CORNERS] - afX[(k + 1) % NUM_FACE_CORNERS]) / fTwiceArea;
	}
	psDest->fWeight = sqrt(fTwiceArea * 0.5);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The rows of one of u and v, two for each triangle (its gradient's x and
// y, weighted): into the matrix where the corner's a column, into
// pafPinned (the pinned UV's part, to take from b) where it's pinned.
//
//////////////////////////////////////////////////////////////////////////
static void MeshUVRigidRows(CFloat64SparseMatrix* pcMatrix, float64* pafPinned, SMeshUVRigidTriangle* pasTriangles, bool* pabUsed, int* paiCorners, int iNumTriangles, int iPin)
{
	int		iTriangle;
	int		iCorner;
	int		k;

	for (iTriangle = 0; iTriangle < iNumTriangles; iTriangle++)
	{
		pafPinned[iTriangle * 2] = 0.0;
		pafPinned[(iTriangle * 2) + 1] = 0.0;
		if (!pabUsed[iTriangle])
		{
			continue;
		}
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			iCorner = paiCorners[(iTriangle * NUM_FACE_CORNERS) + k];
			if (iCorner == iPin)
			{
				pafPinned[iTriangle * 2] += pasTriangles[iTriangle].afAlongX[k] * pasTriangles[iTriangle].fWeight;
				pafPinned[(iTriangle * 2) + 1] += pasTriangles[iTriangle].afAlongY[k] * pasTriangles[iTriangle].fWeight;
			}
			else
			{
				iCorner = (iCorner > iPin) ? (iCorner - 1) : iCorner;
				pcMatrix->Add(iTriangle * 2, iCorner, pasTriangles[iTriangle].afAlongX[k] * pasTriangles[iTriangle].fWeight);
				pcMatrix->Add((iTriangle * 2) + 1, iCorner, pasTriangles[iTriangle].afAlongY[k] * pasTriangles[iTriangle].fWeight);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The triangle's map's 2 by 2 matrix J (u and v along x and y) from the
// UVs, and its nearest turn, (c, -s; s, c) with (c, s) the unit along
// (J00 + J11, J10 - J01).
//
//////////////////////////////////////////////////////////////////////////
static void MeshUVRigidTurn(SMeshUVRigidTriangle* psTriangle, SFloat32Vec2* pasUVs, int* piCorners, float64* pfCos, float64* pfSin)
{
	float64	afJ[4];
	float64	fLength;
	int		k;

	afJ[0] = 0.0;
	afJ[1] = 0.0;
	afJ[2] = 0.0;
	afJ[3] = 0.0;
	for (k = 0; k < NUM_FACE_CORNERS; k++)
	{
		afJ[0] += psTriangle->afAlongX[k] * pasUVs[piCorners[k]].x;
		afJ[1] += psTriangle->afAlongY[k] * pasUVs[piCorners[k]].x;
		afJ[2] += psTriangle->afAlongX[k] * pasUVs[piCorners[k]].y;
		afJ[3] += psTriangle->afAlongY[k] * pasUVs[piCorners[k]].y;
	}
	*pfCos = afJ[0] + afJ[3];
	*pfSin = afJ[2] - afJ[1];
	fLength = sqrt((*pfCos * *pfCos) + (*pfSin * *pfSin));
	if (fLength > 0.0)
	{
		*pfCos /= fLength;
		*pfSin /= fLength;
	}
	else
	{
		*pfCos = 1.0;
		*pfSin = 0.0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// One UV (the first) is pinned, as moving every UV alike changes
// nothing; u and v are each a least squares problem with the same matrix,
// one column for each other UV.  The turned triangles' gradients are the
// answers wanted: u along x and y (c, -s), v (s, c).
//
//////////////////////////////////////////////////////////////////////////
bool FlattenRigid(SFloat32Vec2* pasUVs, SFloat32Vec3* pasPositions, int iNumUVs, int* paiCorners, int iNumTriangles, float32 fTexelsPerUnit)
{
	CArrayLargeTemplate<SMeshUVRigidTriangle>	asTriangles;
	CArrayLargeTemplatePrimitive<bool>			abUsed;
	CFloat64SparseMatrix						cMatrix;
	CFloat64SparseLeastSquares					cSolver;
	CArrayLargeFloat64							afPinned;
	CArrayLargeFloat64							afB;
	CArrayLargeFloat64							afX;
	CArrayLargeFloat64							afTurns;  //Each triangle's nearest turn's cos and sin.
	float64										fPinned;
	float64										fCos;
	float64										fSin;
	float64										fWeight;
	int											iTriangle;
	int											iRound;
	int											iAxis;
	int											i;
	bool										bFlat;

	if (iNumUVs < 2)
	{
		return false;
	}
	asTriangles.Init();
	asTriangles.Resize((lsize)iNumTriangles);
	abUsed.Init();
	for (iTriangle = 0; iTriangle < iNumTriangles; iTriangle++)
	{
		abUsed.Add(MeshUVRigidTriangle(asTriangles.Get((lsize)iTriangle), pasPositions, &paiCorners[iTriangle * NUM_FACE_CORNERS], fTexelsPerUnit));
	}
	afPinned.Init();
	afPinned.Resize((lsize)iNumTriangles * 2);
	cMatrix.Init(iNumTriangles * 2, iNumUVs - 1);
	MeshUVRigidRows(&cMatrix, afPinned.GetData(), asTriangles.GetData(), abUsed.GetData(), paiCorners, iNumTriangles, 0);

	bFlat = cSolver.Init(&cMatrix);
	if (bFlat)
	{
		afB.Init();
		afB.Resize((lsize)iNumTriangles * 2);
		afTurns.Init();
		afTurns.Resize((lsize)iNumTriangles * 2);
		afX.Init();
		afX.Resize((lsize)(iNumUVs - 1) * 2);
		for (iRound = 0; iRound < MESH_UV_RIGID_ROUNDS; iRound++)
		{
			for (iTriangle = 0; iTriangle < iNumTriangles; iTriangle++)
			{
				if (abUsed.GetValue((lsize)iTriangle))
				{
					MeshUVRigidTurn(asTriangles.Get((lsize)iTriangle), pasUVs, &paiCorners[iTriangle * NUM_FACE_CORNERS], &afTurns.GetData()[iTriangle * 2], &afTurns.GetData()[(iTriangle * 2) + 1]);
				}
			}
			for (iAxis = 0; iAxis < 2; iAxis++)
			{
				fPinned = (iAxis == 0) ? pasUVs[0].x : pasUVs[0].y;
				for (iTriangle = 0; iTriangle < iNumTriangles; iTriangle++)
				{
					afB.SetValue((lsize)iTriangle * 2, 0.0);
					afB.SetValue(((lsize)iTriangle * 2) + 1, 0.0);
					if (abUsed.GetValue((lsize)iTriangle))
					{
						fCos = afTurns.GetValue((lsize)iTriangle * 2);
						fSin = afTurns.GetValue(((lsize)iTriangle * 2) + 1);
						fWeight = asTriangles.Get((lsize)iTriangle)->fWeight;
						afB.SetValue((lsize)iTriangle * 2, (((iAxis == 0) ? fCos : fSin) * fWeight) - (afPinned.GetValue((lsize)iTriangle * 2) * fPinned));
						afB.SetValue(((lsize)iTriangle * 2) + 1, (((iAxis == 0) ? -fSin : fCos) * fWeight) - (afPinned.GetValue(((lsize)iTriangle * 2) + 1) * fPinned));
					}
				}
				cSolver.Solve(afX.GetData() + ((lsize)iAxis * (lsize)(iNumUVs - 1)), afB.GetData());
			}
			for (i = 1; i < iNumUVs; i++)
			{
				pasUVs[i].Init((float32)afX.GetValue((lsize)(i - 1)), (float32)afX.GetValue((lsize)(iNumUVs - 1 + i - 1)));
			}
		}
		afTurns.Kill();
		afX.Kill();
		afB.Kill();
	}

	cSolver.Kill();
	cMatrix.Kill();
	afPinned.Kill();
	abUsed.Kill();
	asTriangles.Kill();
	return bFlat;
}

