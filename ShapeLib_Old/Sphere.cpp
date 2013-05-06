#include "Sphere.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphereImm::Set(D3DVECTOR* pos, float rad)
{
	msPosition = (*pos);
	mfRadius = rad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphereImm::Set(float x, float y, float z, float rad)
{
	msPosition.x = x;
	msPosition.y = y;
	msPosition.z = z;
	mfRadius = rad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphereImm::Copy(CSphereImm* source)
{
	memcpy(this, source, sizeof(CSphereImm));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphereImm::SetFromPointsUsingAveragePosition(D3DXVECTOR3* psPoints, int iStride, int iNumPoints)
{
	int				i;
	D3DXVECTOR3*	psPBase;
	D3DXVECTOR3		cPoint;
	float			fMaxLen;
	float			fLen;

	psPBase = psPoints;
	msPosition.x = msPosition.y = msPosition.z = 0.0f;
	for (i = 0; i < iNumPoints; i++)
	{
		(D3DXVECTOR3)msPosition += (*psPoints);
		psPoints = (D3DXVECTOR3*)((size_t)(((int)((size_t)((void*)psPoints)) + iStride)));
	}

	(D3DXVECTOR3)msPosition /= (float)iNumPoints;

	fMaxLen = 0.0f;
	psPoints = psPBase;
	for (i = 0; i < iNumPoints; i++)
	{
		cPoint = (D3DXVECTOR3)msPosition - (D3DXVECTOR3)(*psPoints);
		fLen = D3DXVec3LengthSq(&cPoint);
		if (fLen > fMaxLen)
		{
			fMaxLen = fLen;
		}
		psPoints = (D3DXVECTOR3*)((size_t)(((int)((size_t)((void*)psPoints)) + iStride)));
	}
	mfRadius = sqrtf(fMaxLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphereImm::SetFromPointsUsingBestFit(D3DXVECTOR3* psPBase, int iStride, int iNumPoints)
{
	int				i, j;
	D3DXVECTOR3*	psPoints1;
	D3DXVECTOR3*	psPoints2;
	D3DXVECTOR3*	psEndPoint1;
	D3DXVECTOR3*	psEndPoint2;
	D3DXVECTOR3		cPoint;
	float			fMaxLen;
	float			fLen;

	fMaxLen = 0.0f;
	psPoints1 = psPBase;
	for (i = 0; i < iNumPoints; i++)
	{
		psPoints2 = psPBase;
		for (j = 0; j < iNumPoints; j++)
		{
			if (i != j)
			{
				cPoint = *psPoints1 - *psPoints2;
				fLen = D3DXVec3LengthSq(&cPoint);
				if (fLen > fMaxLen)
				{
					psEndPoint1 = psPoints1;
					psEndPoint2 = psPoints2;
					fMaxLen = fLen;
				}
			}
			psPoints2 = (D3DXVECTOR3*)((size_t)(((int)((size_t)((void*)psPoints2)) + iStride)));
		}
		psPoints1 = (D3DXVECTOR3*)((size_t)(((int)((size_t)((void*)psPoints1)) + iStride)));
	}
	mfRadius = sqrtf(fMaxLen) / 2.0f;
	msPosition = (*psEndPoint1 + *psEndPoint2) / 2.0f;
}

