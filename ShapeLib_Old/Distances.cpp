#include "Distances.h"
#include "DirectXLib\D3DVECTORHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float PointDistanceFromShape(CShapeImm* pcShape, D3DVECTOR* pcPoint)
{
	int		iType;
	void*	pvShape;
	float	fDistance;
	float	fTemp;

	fDistance = 1e30f;
	pvShape = pcShape->GetFirst(&iType);
	while (pvShape)
	{
		switch(iType) 
		{
		case ST_Sphere:
			fTemp =  PointDistanceFromSphere((CSphereImm*)pvShape, pcPoint);
			break;
		case ST_Plane:
			fTemp =  PointDistanceFromPlane((CPlaneImm*)pvShape, pcPoint);
			break;
		//case ST_Capsule:
		//	fTemp =  PointDistanceFromCapsule((CPlaneImm*)pvShape, pcPoint);
		//	break;
		//case ST_ConvexHull:
		//	fTemp =  PointDistanceFromConvexHull((CPlaneImm*)pvShape, pcPoint);
		//	break;
		}
		if (fTemp < fDistance)
		{
			fDistance = fTemp;
		}
		pvShape = pcShape->GetNext(pvShape, &iType);
	}
	return fDistance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float PointDirectedDistanceFromShape(CShapeImm* pcShape, D3DVECTOR* psPoint, D3DVECTOR* psDirection)
{
	int			iType;
	void*		pvShape;
	float		fDistance;
	float		fTemp;
	D3DVECTOR	sTemp;

	fDistance = 1e30f;
	pvShape = pcShape->GetFirst(&iType);
	while (pvShape)
	{
		switch(iType) 
		{
		case ST_Sphere:
			fTemp =  PointDirectedDistanceFromSphere((CSphereImm*)pvShape, psPoint, &sTemp);
			break;
		case ST_Plane:
			fTemp =  PointDirectedDistanceFromPlane((CPlaneImm*)pvShape, psPoint, &sTemp);
			break;
		}
		if (fTemp < fDistance)
		{
			fDistance = fTemp;
			D3DXVec3Assign((D3DXVECTOR3*)psDirection, (D3DXVECTOR3*)&sTemp);
		}
		pvShape = pcShape->GetNext(pvShape, &iType);
	}
	return fDistance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float PointDistanceFromSphere(CSphereImm* pcSphere, D3DVECTOR* point)
{
	D3DXVECTOR3		temp;
	float			f;

	D3DXVec3Subtract(&temp, (D3DXVECTOR3*)&pcSphere->msPosition, (D3DXVECTOR3*)point);
	f = D3DXVec3Length(&temp) - pcSphere->mfRadius;
	return f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float PointDistanceFromPoint(D3DVECTOR* point1, D3DVECTOR* point2)
{
	D3DXVECTOR3		temp;

	D3DXVec3Subtract(&temp, (D3DXVECTOR3*)point1, (D3DXVECTOR3*)point2);
	return D3DXVec3Length(&temp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float PointDistanceSquaredFromPoint(D3DVECTOR* point1, D3DVECTOR* point2)
{
	D3DXVECTOR3		temp;

	D3DXVec3Subtract(&temp, (D3DXVECTOR3*)point1, (D3DXVECTOR3*)point2);
	return D3DXVec3Dot(&temp, &temp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float PointDistanceFromLineInfinite(CLineInfinite* pcLine, D3DVECTOR* point)
{
	D3DXVECTOR3		v1;
	D3DXVECTOR3		v2;

	D3DXVec3Subtract(&v1, (D3DXVECTOR3*)point, (D3DXVECTOR3*)&pcLine->msStart);
	D3DXVec3Cross(&v2, &v1, (D3DXVECTOR3*)&pcLine->msDirection);
	return D3DXVec3Length(&v2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float PointDistanceAlongLineInfinite(CLineInfinite* pcLine, D3DVECTOR* point)
{
	D3DXVECTOR3		temp;

	D3DXVec3Subtract(&temp, (D3DXVECTOR3*)point, (D3DXVECTOR3*)&pcLine->msStart);
	return D3DXVec3Dot(&temp, (D3DXVECTOR3*)&pcLine->msDirection);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float PointDistanceFromLineSegment(CLineSegmentImm* pcLine, D3DVECTOR* point)
{
	D3DXVECTOR3		temp;
	D3DXVECTOR3		v2;
	float			d;

	//Perpendicular distance along pcLine
	D3DXVec3Subtract(&temp, (D3DXVECTOR3*)point, (D3DXVECTOR3*)&pcLine->msStart);
	d = D3DXVec3Dot(&temp, (D3DXVECTOR3*)&pcLine->msDirection);

	//If d lies between 0 and length then shortest distance to point from pcLine is perpendicular to it.
	if ((d >= 0) && (d <= pcLine->mfLength))
	{
		D3DXVec3Cross(&v2, &temp, (D3DXVECTOR3*)&pcLine->msDirection);
		return D3DXVec3Length(&v2);
	}

	//If d is less than 0 then shortest distance is from start to point.
	else if (d < 0)
	{
		return D3DXVec3Length(&temp);
	}

	//If d is greater than length then shortest distance is from msEnd to point.
	else
	{
		D3DXVec3Subtract(&temp, (D3DXVECTOR3*)point, (D3DXVECTOR3*)&pcLine->msEnd);
		return D3DXVec3Length(&temp);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float PointDistanceSquaredFromLineSegment(CLineSegmentImm* pcLine, D3DVECTOR* point)
{
	D3DXVECTOR3		temp;
	D3DXVECTOR3		v2;
	float			d;

	//Perpendicular distance along pcLine
	D3DXVec3Subtract(&temp, (D3DXVECTOR3*)point, (D3DXVECTOR3*)&pcLine->msStart);
	d = D3DXVec3Dot(&temp, (D3DXVECTOR3*)&pcLine->msDirection);

	//If d lies between 0 and length then shortest distance to point from pcLine is perpendicular to it.
	if ((d >= 0) && (d <= pcLine->mfLength))
	{
		D3DXVec3Cross(&v2, &temp, (D3DXVECTOR3*)&pcLine->msDirection);
		return D3DXVec3Dot(&v2, &v2);
	}

	//If d is less than 0 then shortest distance is from start to point.
	else if (d < 0)
	{
		return D3DXVec3Dot(&temp, &temp);
	}

	//If d is greater than length then shortest distance is from msEnd to point.
	else
	{
		D3DXVec3Subtract(&temp, (D3DXVECTOR3*)point, (D3DXVECTOR3*)&pcLine->msEnd);
		return D3DXVec3Dot(&temp, &temp);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float PointDistanceFromPlane(CPlaneImm* pcPlane, D3DVECTOR* psPoint)
{
	return D3DXVec3Dot((D3DXVECTOR3*)&pcPlane->sNormal, (D3DXVECTOR3*)psPoint);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float PointDirectedDistanceFromPlane(CPlaneImm* pcPlane, D3DVECTOR* psPoint, D3DVECTOR* psDirection)
{
	D3DXVECTOR3		temp;

	D3DXVec3Assign((D3DXVECTOR3*)psDirection, (D3DXVECTOR3*)&pcPlane->sNormal);
	D3DXVec3Subtract(&temp, (D3DXVECTOR3*)psPoint, (D3DXVECTOR3*)&pcPlane->sPosition);
	return D3DXVec3Dot((D3DXVECTOR3*)&pcPlane->sNormal, &temp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float PointDirectedDistanceFromSphere(CSphereImm* pcSphere, D3DVECTOR* psPoint, D3DVECTOR* psDirection)
{
	D3DXVECTOR3		temp;
	float			f;
	float			fInv;

	D3DXVec3Subtract(&temp, (D3DXVECTOR3*)psPoint, (D3DXVECTOR3*)&pcSphere->msPosition);
	f = D3DXVec3Length(&temp) - pcSphere->mfRadius;
	if (f > 0.0f)
	{
		fInv = 1.0f / f;
		D3DXVec3Assign((D3DXVECTOR3*)psDirection, temp.x*fInv, temp.y*fInv, temp.z*fInv);
	}
	else
	{
		D3DXVec3Assign((D3DXVECTOR3*)psDirection, 0.0f, 0.0f, 0.0f);
	}
	return f;
}

