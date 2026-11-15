/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "Float32Symmetric3x3.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Symmetric3x3::Init(void)
{
	Init(0, 0, 0, 0, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Symmetric3x3::Init(float32 fXX, float32 fXY, float32 fXZ, float32 fYY, float32 fYZ, float32 fZZ)
{
	xx = fXX;
	xy = fXY;
	xz = fXZ;
	yy = fYY;
	yz = fYZ;
	zz = fZZ;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Symmetric3x3::AddOuter(SFloat32Vec3* psVector)
{
	xx += psVector->x * psVector->x;
	xy += psVector->x * psVector->y;
	xz += psVector->x * psVector->z;
	yy += psVector->y * psVector->y;
	yz += psVector->y * psVector->z;
	zz += psVector->z * psVector->z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Symmetric3x3::Multiply(SFloat32Vec3* psDest, SFloat32Vec3* psVector)
{
	float32	x;
	float32	y;
	float32	z;

	x = psVector->x;
	y = psVector->y;
	z = psVector->z;
	psDest->Init((xx * x) + (xy * y) + (xz * z), (xy * x) + (yy * y) + (yz * z), (xz * x) + (yz * y) + (zz * z));
}


//////////////////////////////////////////////////////////////////////////
//
// Cyclic Jacobi: each sweep turns the matrix by a rotation in the plane
// of each pair of axes, (x, y), (x, z) and (y, z), chosen to make that
// pair's number zero.  A' = R'AR, so its eigenvalues don't change, and
// the rotations multiplied together are its eigenvectors, as columns.
// The rotation's tangent is the smaller root of t^2 + 2 theta t - 1 = 0,
// theta = (Aqq - App) / 2Apq, so it turns by under 45 degrees.  It stops
// when what is off the diagonal is nothing beside what is on it.
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Symmetric3x3::Eigen(float32* pafValues, SFloat32Vec3* pasVectors)
{
	static const int	aaiPairs[3][2] = {{0, 1}, {0, 2}, {1, 2}};
	float32				aafA[3][3];
	float32				aafV[3][3];
	float32				aafB[3][3];
	float32				fTheta;
	float32				fT;
	float32				fC;
	float32				fS;
	float32				fOff;
	float32				fOn;
	float32				fValue;
	SFloat32Vec3		sVector;
	int					iSweep;
	int					iPair;
	int					p;
	int					q;
	int					i;
	int					j;
	int					k;

	aafA[0][0] = xx;
	aafA[0][1] = xy;
	aafA[0][2] = xz;
	aafA[1][0] = xy;
	aafA[1][1] = yy;
	aafA[1][2] = yz;
	aafA[2][0] = xz;
	aafA[2][1] = yz;
	aafA[2][2] = zz;
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			aafV[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}

	for (iSweep = 0; iSweep < FLOAT32_SYMMETRIC_3X3_SWEEPS; iSweep++)
	{
		fOff = (aafA[0][1] * aafA[0][1]) + (aafA[0][2] * aafA[0][2]) + (aafA[1][2] * aafA[1][2]);
		fOn = (aafA[0][0] * aafA[0][0]) + (aafA[1][1] * aafA[1][1]) + (aafA[2][2] * aafA[2][2]);
		if (fOff <= (fOn * 1e-14f))
		{
			break;
		}

		for (iPair = 0; iPair < 3; iPair++)
		{
			p = aaiPairs[iPair][0];
			q = aaiPairs[iPair][1];
			if (aafA[p][q] == 0.0f)
			{
				continue;
			}

			fTheta = (aafA[q][q] - aafA[p][p]) / (2.0f * aafA[p][q]);
			fT = 1.0f / (fabsf(fTheta) + sqrtf((fTheta * fTheta) + 1.0f));
			if (fTheta < 0.0f)
			{
				fT = -fT;
			}
			fC = 1.0f / sqrtf((fT * fT) + 1.0f);
			fS = fT * fC;

			//B = A R, where R is the identity but for Rpp = Rqq = c, Rpq = s and Rqp = -s.
			for (i = 0; i < 3; i++)
			{
				for (j = 0; j < 3; j++)
				{
					aafB[i][j] = aafA[i][j];
				}
				aafB[i][p] = (fC * aafA[i][p]) - (fS * aafA[i][q]);
				aafB[i][q] = (fS * aafA[i][p]) + (fC * aafA[i][q]);
			}

			//A = R' B.
			for (j = 0; j < 3; j++)
			{
				aafA[p][j] = (fC * aafB[p][j]) - (fS * aafB[q][j]);
				aafA[q][j] = (fS * aafB[p][j]) + (fC * aafB[q][j]);
				for (i = 0; i < 3; i++)
				{
					if ((i != p) && (i != q))
					{
						aafA[i][j] = aafB[i][j];
					}
				}
			}
			aafA[p][q] = 0.0f;
			aafA[q][p] = 0.0f;

			//V = V R.
			for (i = 0; i < 3; i++)
			{
				fValue = aafV[i][p];
				aafV[i][p] = (fC * fValue) - (fS * aafV[i][q]);
				aafV[i][q] = (fS * fValue) + (fC * aafV[i][q]);
			}
		}
	}

	for (k = 0; k < 3; k++)
	{
		pafValues[k] = aafA[k][k];
		pasVectors[k].Init(aafV[0][k], aafV[1][k], aafV[2][k]);
	}

	//Largest first.
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 2 - i; j++)
		{
			if (pafValues[j] < pafValues[j + 1])
			{
				fValue = pafValues[j];
				pafValues[j] = pafValues[j + 1];
				pafValues[j + 1] = fValue;
				sVector = pasVectors[j];
				pasVectors[j] = pasVectors[j + 1];
				pasVectors[j + 1] = sVector;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The sum over eigenvectors v of v (v . psVector) / value, leaving out
// those whose value is under fCutOff of the largest: it solves the
// matrix as far as it can be and adds nothing along the rest.
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Symmetric3x3::PseudoInverseMultiply(SFloat32Vec3* psDest, SFloat32Vec3* psVector, float32 fCutOff)
{
	float32			afValues[3];
	SFloat32Vec3	asVectors[3];
	float32			fLargest;
	float32			fAlong;
	int				i;

	Eigen(afValues, asVectors);
	psDest->Init(0, 0, 0);
	fLargest = fabsf(afValues[0]);
	for (i = 1; i < 3; i++)
	{
		if (fabsf(afValues[i]) > fLargest)
		{
			fLargest = fabsf(afValues[i]);
		}
	}
	if (fLargest == 0.0f)
	{
		return;
	}

	for (i = 0; i < 3; i++)
	{
		if (fabsf(afValues[i]) >= (fLargest * fCutOff))
		{
			fAlong = Float32Vec3Dot(&asVectors[i], psVector) / afValues[i];
			psDest->x += asVectors[i].x * fAlong;
			psDest->y += asVectors[i].y * fAlong;
			psDest->z += asVectors[i].z * fAlong;
		}
	}
}

