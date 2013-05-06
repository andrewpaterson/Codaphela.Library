/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ShapeLib

Codaphela ShapeLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ShapeLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ShapeLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Box.h"
#include "BaseLib/Numbers.h"
#include "ConvexHull.h"
#include "BaseLib/GeometricTypes.h"
#include "BaseLib/FloatHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBox::Init(SFloat3* psCenter, SFloat3* psXAxis, SFloat3* psYAxis, SFloat3* psZAxis)
{
	mpsCenter = psCenter;
	mpsLongAxis = psXAxis;
	mpsMiddleAxis = psYAxis;
	mpsShortAxis = psZAxis;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CBox::Volume(void)
{
	return mfLongLength * mfMiddleLength * mfShortLength;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBox::SetFromPointsAxisAligned(SFloat3* psPoints, int iStride, int iNumPoints)
{
	int					i;
	SFloat3			msMinimum;
	SFloat3			msMaximum;
	SFloat3*			psPoint;

	msMinimum.Init(LARGE_NUMBER, LARGE_NUMBER, LARGE_NUMBER);
	msMaximum.Init(-LARGE_NUMBER, -LARGE_NUMBER, -LARGE_NUMBER);
	for (i = 0; i < iNumPoints; i++)
	{
		psPoint = GetPosition(psPoints, iStride, iNumPoints);

		//Greater than...
		if (psPoint->x > msMaximum.x)
			msMaximum.x = psPoint->x;
		if (psPoint->y > msMaximum.y)
			msMaximum.y = psPoint->y;
		if (psPoint->z > msMaximum.z)
			msMaximum.z = psPoint->z;

		//Less than...
		if (psPoint->x < msMinimum.x)
			msMinimum.x = psPoint->x;
		if (psPoint->y < msMinimum.y)
			msMinimum.y = psPoint->y;
		if (psPoint->z < msMinimum.z)
			msMinimum.z = psPoint->z;
	}

	mfLongLength = msMaximum.x - msMinimum.x;
	mfMiddleLength = msMaximum.y - msMinimum.y;
	mfShortLength = msMaximum.z - msMinimum.z;
	mpsLongAxis->Init(1,0,0);
	mpsMiddleAxis->Init(0,1,0);
	mpsShortAxis->Init(0,0,1);

	if (mfLongLength < mfShortLength)
	{
		Swap(&mfLongLength, &mfShortLength);
		Float3Swap(mpsLongAxis, mpsShortAxis);
	}
	if (mfLongLength < mfMiddleLength)
	{
		Swap(&mfLongLength, &mfMiddleLength);
		Float3Swap(mpsLongAxis, mpsMiddleAxis);
	}
	if (mfMiddleLength < mfShortLength)
	{
		Swap(&mfMiddleLength, &mfShortLength);
		Float3Swap(mpsMiddleAxis, mpsShortAxis);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void FindMinXAndMaxX(SFloat3* psPoints, int iStride, int iNumPoints, float* pfMax, float* pfMin)
{
	int		i;
	float	fXMax;
	float	fXMin;

	fXMax = -LARGE_NUMBER;
	fXMin = LARGE_NUMBER;

	for (i = 0; i < iNumPoints; i++)
	{
		if (psPoints->x > fXMax)
			fXMax = psPoints->x;

		if (psPoints->x < fXMin)
			fXMin = psPoints->x;

		psPoints = (SFloat3*)RemapSinglePointer(psPoints, iStride);
	}

	*pfMax = fXMax;
	*pfMin = fXMin;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void FindMinYAndMaxY(SFloat3* psPoints, int iStride, int iNumPoints, float* pfMay, float* pfMin)
{
	int		i;
	float	fYMay;
	float	fYMin;

	fYMay = -LARGE_NUMBER;
	fYMin = LARGE_NUMBER;

	for (i = 0; i < iNumPoints; i++)
	{
		if (psPoints->y > fYMay)
			fYMay = psPoints->y;

		if (psPoints->y < fYMin)
			fYMin = psPoints->y;

		psPoints = (SFloat3*)RemapSinglePointer(psPoints, iStride);
	}

	*pfMay = fYMay;
	*pfMin = fYMin;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void FindMinZAndMaxZ(SFloat3* psPoints, int iStride, int iNumPoints, float* pfMaz, float* pfMin)
{
	int		i;
	float	fZMaz;
	float	fZMin;

	fZMaz = -LARGE_NUMBER;
	fZMin = LARGE_NUMBER;

	for (i = 0; i < iNumPoints; i++)
	{
		if (psPoints->z > fZMaz)
			fZMaz = psPoints->z;

		if (psPoints->z < fZMin)
			fZMin = psPoints->z;

		psPoints = (SFloat3*)RemapSinglePointer(psPoints, iStride);
	}

	*pfMaz = fZMaz;
	*pfMin = fZMin;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CopyPoints(CArrayBlock* pasPositions, SFloat3* psPoints, int iStride, int iNumPoints)
{
	int		i;

	for (i = 0; i < iNumPoints; i++)
	{
		pasPositions->Add(psPoints);
		psPoints = (SFloat3*)RemapSinglePointer(psPoints, iStride);
	}

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBox::SetFromPointsUsingBestFit(SFloat3* psPoints, int iStride, int iNumPoints, BOOL bPointsExtreme)
{
	CConvexHull				cConvexHull;
	CConvexHullGenerator	cGenerator;
	CArrayBlock			asPositions;
	SFloat3					sVector;
	float					fDeg;
	SFloat4x4				sMatrix;
	SFloat3*				asTransformedPoints;
	float					fMax;
	float					fMin;
	float					fDist;
	float					fMinDist;
	float					fMinAngle;
	SFloat4x4				sMinXMatrix;
	SFloat4x4				sMinXMinYMatrix;
	SFloat4x4				sFinalMatrix;
	float					fXSize;
	float					fYSize;
	float					fZSize;

	if (iNumPoints < 200)
	{
		//Don't bother generating a hull off this if there are only a few points.
		bPointsExtreme = TRUE;
	}

	if (!bPointsExtreme)
	{
		cConvexHull.Init();

		cConvexHull.BeginSetFromPoints(&cGenerator, psPoints, iStride, iNumPoints, NULL);
		cConvexHull.EndSetFromPoints(NULL, 0, &cGenerator);
		
		asPositions.Init(sizeof(SFloat3), cConvexHull.mcPolygons.NumElements()+1);
		cConvexHull.GetVertices(&asPositions, psPoints, iStride);  	//We now have only the extremal points.  Ie: those that lie on the hull.
	}
	else
	{
		asPositions.Init(sizeof(SFloat3), iNumPoints);
		CopyPoints(&asPositions, psPoints, iStride, iNumPoints);
	}

	asTransformedPoints = (SFloat3*)malloc(sizeof(SFloat3) * asPositions.NumElements());


	fMinDist = LARGE_NUMBER;
	fMinAngle = -1.0f;
	for (fDeg = 0.0f; fDeg <= 360.0f; fDeg += 0.125f)
	{
		Float4x4RotationY(&sMatrix, Deg2Rad(fDeg));
		Float3TransformCoords(asTransformedPoints, sizeof(SFloat3), (SFloat3*)asPositions.GetData(), asPositions.ElementSize(), &sMatrix, asPositions.NumElements());

		FindMinXAndMaxX(asTransformedPoints, sizeof(SFloat3), asPositions.NumElements(), &fMax, &fMin);
		fDist = fMax - fMin;

		if (fDist < fMinDist)
		{
			fMinAngle = fDeg;
			fMinDist = fDist;
		}
	}

	Float4x4RotationY(&sMinXMatrix, Deg2Rad(fMinAngle));

	fMinDist = LARGE_NUMBER;
	fMinAngle = -1.0f;
	for (fDeg = 0.0f; fDeg <= 360.0f; fDeg += 0.125f)
	{
		Float4x4RotationX(&sMatrix, Deg2Rad(fDeg));
		Float4x4Multiply(&sMatrix, &sMinXMatrix, &sMatrix);
		Float3TransformCoords(asTransformedPoints, sizeof(SFloat3), (SFloat3*)asPositions.GetData(), asPositions.ElementSize(), &sMatrix, asPositions.NumElements());

		FindMinYAndMaxY(asTransformedPoints, sizeof(SFloat3), asPositions.NumElements(), &fMax, &fMin);
		fDist = fMax - fMin;
		if (fDist < fMinDist)
		{
			fMinAngle = fDeg;
			fMinDist = fDist;
		}
	}

	Float4x4RotationX(&sMatrix, Deg2Rad(fMinAngle));
	Float4x4Multiply(&sMinXMinYMatrix, &sMinXMatrix, &sMatrix);

	fMinDist = LARGE_NUMBER;
	fMinAngle = -1.0f;
	for (fDeg = 0.0f; fDeg <= 360.0f; fDeg += 0.125f)
	{
		Float4x4RotationZ(&sMatrix, Deg2Rad(fDeg));
		Float4x4Multiply(&sMatrix, &sMinXMinYMatrix, &sMatrix);
		Float3TransformCoords(asTransformedPoints, sizeof(SFloat3), (SFloat3*)asPositions.GetData(), asPositions.ElementSize(), &sMatrix, asPositions.NumElements());

		FindMinYAndMaxY(asTransformedPoints, sizeof(SFloat3), asPositions.NumElements(), &fMax, &fMin);
		fDist = fMax - fMin;
		if (fDist < fMinDist)
		{
			fMinAngle = fDeg;
			fMinDist = fDist;
		}
	}

	Float4x4RotationZ(&sMatrix, Deg2Rad(fMinAngle));
	Float4x4Multiply(&sFinalMatrix, &sMinXMinYMatrix, &sMatrix);
	sFinalMatrix.Fix();

	Float3TransformCoords(asTransformedPoints, sizeof(SFloat3), (SFloat3*)asPositions.GetData(), asPositions.ElementSize(), &sFinalMatrix, asPositions.NumElements());
	FindMinXAndMaxX(asTransformedPoints, sizeof(SFloat3), asPositions.NumElements(), &fMax, &fMin);
	fXSize = fMax - fMin;
	FindMinYAndMaxY(asTransformedPoints, sizeof(SFloat3), asPositions.NumElements(), &fMax, &fMin);
	fYSize = fMax - fMin;
	FindMinZAndMaxZ(asTransformedPoints, sizeof(SFloat3), asPositions.NumElements(), &fMax, &fMin);
	fZSize = fMax - fMin;

	*mpsShortAxis = *((SFloat3*)&sFinalMatrix.y);
	mfShortLength = fYSize;

	if (fXSize >= fZSize)
	{
		*mpsLongAxis = *((SFloat3*)&sFinalMatrix.x);
		mfLongLength = fXSize;
		*mpsMiddleAxis = *((SFloat3*)&sFinalMatrix.z);
		mfMiddleLength = fZSize;
	}
	else
	{
		*mpsLongAxis = *((SFloat3*)&sFinalMatrix.z);
		mfLongLength = fZSize;
		*mpsMiddleAxis = *((SFloat3*)&sFinalMatrix.x);
		mfMiddleLength = fXSize;
	}

	asPositions.Kill();

	if (!bPointsExtreme)
	{
		cConvexHull.Kill();
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBox::Find8Box(int* piLongCount, int* piMiddleCount, int* piShortCount)
{
	int		x,y,z;
	float	fVolume;
	float	fMin;

	//2x2x2 case.
	fVolume = PrivateVolumeOfCubesContaining(2,2,2);
	x=2; y=2; z=2;
	fMin = fVolume;

	//4x2x1.
	fVolume = PrivateVolumeOfCubesContaining(4,2,1);
	if (fVolume < fMin)
	{
		x=4; y=2; z=1;
		fMin = fVolume;
	}

	//3x2x1.
	fVolume = PrivateVolumeOfCubesContaining(3,2,1);
	if (fVolume < fMin)
	{
		x=3; y=2; z=1;
		fMin = fVolume;
	}

	//2x2x1.
	fVolume = PrivateVolumeOfCubesContaining(2,2,1);
	if (fVolume < fMin)
	{
		x=2; y=2; z=1;
		fMin = fVolume;
	}

	//2x1x1.
	fVolume = PrivateVolumeOfCubesContaining(2,1,1);
	if (fVolume < fMin)
	{
		x=2; y=1; z=1;
		fMin = fVolume;
	}

	//3x1x1.
	fVolume = PrivateVolumeOfCubesContaining(3,1,1);
	if (fVolume < fMin)
	{
		x=3; y=1; z=1;
		fMin = fVolume;
	}

	//4x1x1.
	fVolume = PrivateVolumeOfCubesContaining(4,1,1);
	if (fVolume < fMin)
	{
		x=4; y=1; z=1;
		fMin = fVolume;
	}

	//5x1x1.
	fVolume = PrivateVolumeOfCubesContaining(5,1,1);
	if (fVolume < fMin)
	{
		x=5; y=1; z=1;
		fMin = fVolume;
	}

	//6x1x1.
	fVolume = PrivateVolumeOfCubesContaining(6,1,1);
	if (fVolume < fMin)
	{
		x=6; y=1; z=1;
		fMin = fVolume;
	}

	//7x1x1.
	fVolume = PrivateVolumeOfCubesContaining(7,1,1);
	if (fVolume < fMin)
	{
		x=7; y=1; z=1;
		fMin = fVolume;
	}

	//8x1x1.
	fVolume = PrivateVolumeOfCubesContaining(8,1,1);
	if (fVolume < fMin)
	{
		x=8; y=1; z=1;
		fMin = fVolume;
	}

	*piLongCount = x;
	*piMiddleCount = y;
	*piShortCount = z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CBox::PrivateVolumeOfCubesContaining(float ix, float iy, float iz)
{
	float fRelativeMiddle;
	float fRelativeLong;
	float fMinY;
	float fMinX;
	float fMinZ;

	fRelativeLong = mfLongLength / mfShortLength;
	fRelativeMiddle = mfMiddleLength / mfShortLength;

	
	fMinX = mfLongLength/ix;
	fMinY = mfMiddleLength/iy;
	fMinZ = mfShortLength/iz;

	//The new volume must be sized up by whichever is greatest of fMinZ, fMinX, fMinY to properly contain this boxes volume.

	//X is greatest.
	if ((fMinX >= fMinY) && (fMinX >= fMinZ))
	{
		return ((fMinX*ix) * (fMinX*iy) * (fMinX*iz));
	}
	//Y is greatest.
	if ((fMinY >= fMinX) && (fMinY >= fMinZ))
	{
		return ((fMinY*ix) * (fMinY*iy) * (fMinY*iz));
	}
	//X is greatest.
	if ((fMinZ >= fMinX) && (fMinZ >= fMinY))
	{
		return ((fMinZ*ix) * (fMinZ*iy) * (fMinZ*iz));
	}
	return 0.0f;
}

