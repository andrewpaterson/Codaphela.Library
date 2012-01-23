#include "Intersections.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL SphereSphereIntersect(CSphereImm* pcSphere1, CSphereImm* pcSphere2)
{
	D3DXVECTOR3	dir;
	float		dist;

	D3DXVec3Subtract(&dir, (D3DXVECTOR3*)&pcSphere2->msPosition, (D3DXVECTOR3*)&pcSphere1->msPosition);
	dist = D3DXVec3LengthSq(&dir);

	if (dist > (pcSphere1->mfRadius + pcSphere2->mfRadius) * (pcSphere1->mfRadius + pcSphere2->mfRadius))
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL SphereConeIntersect(CSphereImm* pcSphere, CConeImm* pcCone)
{
	float			fDistAlong;
	float			fDistFrom;
	float			fWidth;
    
	//Rewrite this...  Look it up in: http://mathworld.wolfram.com/Cone-SphereIntersection.html

	fDistAlong = PointDistanceAlongLineInfinite((CLineInfinite*)pcCone, &pcSphere->msPosition);

	//if above the cone then no intersection.
	if (fDistAlong + pcSphere->mfRadius < 0.0f)
	{
		return FALSE;
	}
	//If below the cone then no intersection.
	if (fDistAlong - pcSphere->mfRadius > pcCone->mfLength)
	{
		return FALSE;
	}
	fDistFrom = PointDistanceFromLineInfinite((CLineInfinite*)pcCone, &pcSphere->msPosition);
	fWidth = fDistAlong * pcCone->mfRadius;
    
	if (fDistFrom - pcSphere->mfRadius > fWidth)
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL SphereTriangleIntersect(CSphereImm* psSphere, CTriangleImm *psTriangle)
{
	
	D3DXVECTOR3  	sVectorA;
	D3DXVECTOR3  	sVectorB;
	D3DXVECTOR3  	sVectorC;
	D3DXVECTOR3  	sVectorToSphere;
	D3DXVECTOR3  	sPointOnPlane;
	float        	fDistanceToSphere;
	float        	fS, fT;
	CLineSegmentImm sLineSegment;

	//---------------------------------------------------------------------------------------------------
	// calculate the closest point on the plane to the sphere.
	//---------------------------------------------------------------------------------------------------
	D3DXVec3Subtract(&sVectorA,        (D3DXVECTOR3*)&psTriangle->asPoints[1], (D3DXVECTOR3*)&psTriangle->asPoints[0]);
	D3DXVec3Subtract(&sVectorB,        (D3DXVECTOR3*)&psTriangle->asPoints[2], (D3DXVECTOR3*)&psTriangle->asPoints[0]);
	D3DXVec3Subtract(&sVectorToSphere, (D3DXVECTOR3*)&psSphere->msPosition, (D3DXVECTOR3*)&psTriangle->asPoints[0]);
	fDistanceToSphere = D3DXVec3Dot(&sVectorToSphere, (D3DXVECTOR3*)&psTriangle->sNormal);
	D3DXVec3Scale(&sVectorToSphere, (D3DXVECTOR3*)&psTriangle->sNormal, fDistanceToSphere);
	D3DXVec3Subtract(&sPointOnPlane, (D3DXVECTOR3*)&psSphere->msPosition, &sVectorToSphere);

	//---------------------------------------------------------------------------------------------------
	// sPointOnPlane is now the point on the plane
	//---------------------------------------------------------------------------------------------------
	// check the triangle normal and flatten in it's major axis.
	if ((psTriangle->sNormal.x > psTriangle->sNormal.y) && (psTriangle->sNormal.x > psTriangle->sNormal.z))
	{
		// x is the major axis, so use the y & z components
		fT = ((sPointOnPlane.z * sVectorA.y) - (sPointOnPlane.y * sVectorA.z)) / ((sVectorB.z * sVectorA.y) - (sVectorB.y * sVectorA.z));
		fS = ((sPointOnPlane.y - sVectorB.y) * fT) / sVectorA.y;
	}
	else
	if ((psTriangle->sNormal.y > psTriangle->sNormal.x) && (psTriangle->sNormal.y > psTriangle->sNormal.z))
	{
		// y is the major axis, so use the x & z components
		fT = ((sPointOnPlane.x * sVectorA.z) - (sPointOnPlane.z * sVectorA.x)) / ((sVectorB.x * sVectorA.z) - (sVectorB.z * sVectorA.x));
		fS = ((sPointOnPlane.z - sVectorB.z) * fT) / sVectorA.z;
	}
	else
	if ((psTriangle->sNormal.z > psTriangle->sNormal.x) && (psTriangle->sNormal.z > psTriangle->sNormal.y))
	{
		// z is the major axis, so use the x & y components.
		fT = ((sPointOnPlane.x * sVectorA.y) - (sPointOnPlane.y * sVectorA.x)) / ((sVectorB.x * sVectorA.y) - (sVectorB.y * sVectorA.x));
		fS = ((sPointOnPlane.y - sVectorB.y) * fT) / sVectorA.y;
	}
	//---------------------------------------------------------------------------------------------------
	// (fS & fT) now determine which region the point is in.
	//---------------------------------------------------------------------------------------------------
	if (fS < 0)
	{
		if (fT < 0)
		{
			// region 0
			if (PointDistanceSquaredFromPoint(&psTriangle->asPoints[0], &psSphere->msPosition) <= (psSphere->mfRadius * psSphere->mfRadius))
			{
				return TRUE;
			}
			return FALSE;
		}
		if ((fS + fT) < 1.0f)
		{
			// region 0-1

			//--------------------------------------------------------
			// check how far the sphere is from line segment 0-1
			//--------------------------------------------------------
			D3DXVec3Subtract(&sVectorC, (D3DXVECTOR3*)&psTriangle->asPoints[1], (D3DXVECTOR3*)&psTriangle->asPoints[0]);
			// create the line segment
			memcpy(&sLineSegment.msStart,     (D3DXVECTOR3*)&psTriangle->asPoints[0], sizeof(D3DXVECTOR3));
			memcpy(&sLineSegment.msEnd,       (D3DXVECTOR3*)&psTriangle->asPoints[1], sizeof(D3DXVECTOR3));
			sLineSegment.mfLength = D3DXVec3Length(&sVectorC);
			D3DXVec3Scale((D3DXVECTOR3 *)&sLineSegment.msDirection, &sVectorC, 1.0f / sLineSegment.mfLength);
			
			if (PointDistanceSquaredFromLineSegment(&sLineSegment, &psSphere->msPosition) <= (psSphere->mfRadius * psSphere->mfRadius))
			{
				return TRUE;
			}

			return FALSE;

		}
		else
		{
			// region 1
			if (PointDistanceSquaredFromPoint(&psTriangle->asPoints[1], &psSphere->msPosition) <= (psSphere->mfRadius * psSphere->mfRadius))
			{
				return TRUE;
			}
			return FALSE;
		}
	}
	else
	{
		if (fT < 0)
		{
			if ((fS + fT) < 1.0f)
			{
				// region 0-2
				//--------------------------------------------------------
				// check how far the sphere is from line segment 0-2
				//--------------------------------------------------------
				D3DXVec3Subtract(&sVectorC, (D3DXVECTOR3*)&psTriangle->asPoints[2], (D3DXVECTOR3*)&psTriangle->asPoints[0]);

				// create the line segment
				memcpy(&sLineSegment.msStart,     &psTriangle->asPoints[0], sizeof(D3DXVECTOR3));
				memcpy(&sLineSegment.msEnd,       &psTriangle->asPoints[2], sizeof(D3DXVECTOR3));
				sLineSegment.mfLength = D3DXVec3Length(&sVectorC);
				D3DXVec3Scale((D3DXVECTOR3 *)&sLineSegment.msDirection, &sVectorC, 1.0f / sLineSegment.mfLength);
				
				if (PointDistanceSquaredFromLineSegment(&sLineSegment, &psSphere->msPosition) <= (psSphere->mfRadius * psSphere->mfRadius))
				{
					return TRUE;
				}

				return FALSE;

			}
			else
			{
				// region 2
				if (PointDistanceSquaredFromPoint(&psTriangle->asPoints[2], &psSphere->msPosition) <= (psSphere->mfRadius * psSphere->mfRadius))
				{
					return TRUE;
				}
				return FALSE;
			}
		}
		else
		{
			if ((fS + fT) > 1.0f)
			{
				// region 1-2
				//--------------------------------------------------------
				// check how far the sphere is from line segment 1-2
				//--------------------------------------------------------
				D3DXVec3Subtract(&sVectorC, (D3DXVECTOR3*)&psTriangle->asPoints[2], (D3DXVECTOR3*)&psTriangle->asPoints[1]);

				// create the line segment
				memcpy(&sLineSegment.msStart,     &psTriangle->asPoints[1], sizeof(D3DXVECTOR3));
				memcpy(&sLineSegment.msEnd,       &psTriangle->asPoints[2], sizeof(D3DXVECTOR3));
				sLineSegment.mfLength = D3DXVec3Length(&sVectorC);
				D3DXVec3Scale((D3DXVECTOR3 *)&sLineSegment.msDirection, &sVectorC, 1.0f / sLineSegment.mfLength);
				
				if (PointDistanceSquaredFromLineSegment(&sLineSegment, &psSphere->msPosition) <= (psSphere->mfRadius * psSphere->mfRadius))
				{
					return TRUE;
				}

				return FALSE;
			}
			else
			{
				// region 0-1-2
				//--------------------------------------------------------
				// check how far away the sphere is from the plane.
				//--------------------------------------------------------
				if (fDistanceToSphere <= psSphere->mfRadius)
				{
					return TRUE;
				}
				return FALSE;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL PlaneRayIntersect(D3DVECTOR* pcDest, CPlaneImm* pcPlane, CRayImm* pcRay)
{
	//Let r(t) = p + v*t be the equation of your ray. Let n be the plane normal and q be a point on the plane. 
	//Then any point x on the plane satisfies the equation

	//	n . (q - x) = 0.

	//	So substitute the ray equation r(t) in for x and solve for t:

	//n . (q - r(t)) = 0
	//	n . (q - p - v*t) = 0
	//	n . (q - p) = (n . v) * t
	//	t = [ n . (q - p) ] / (n . v)

	D3DXVECTOR3		qMinusP;
	float			nDotqMinusP;
	float			nDotv;
	float			t;
	D3DXVECTOR3		vScalet;
	
	nDotv = D3DXVec3Dot((D3DXVECTOR3*)&pcPlane->sNormal, (D3DXVECTOR3*)&pcRay->msDirection);
	if (nDotv == 0.0f)
	{
		return FALSE;
	}

	D3DXVec3Subtract(&qMinusP, (D3DXVECTOR3*)&pcPlane->sPosition, (D3DXVECTOR3*)&pcRay->msStart);
	nDotqMinusP = D3DXVec3Dot((D3DXVECTOR3*)&pcPlane->sNormal, &qMinusP);
	t = nDotqMinusP / nDotv;
	D3DXVec3Scale(&vScalet, (D3DXVECTOR3*)&pcRay->msDirection, t);
	D3DXVec3Add((D3DXVECTOR3*)pcDest, (D3DXVECTOR3*)&pcRay->msStart, &vScalet);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL TriangleRayIntersect(CTriangleImm* pcTriangle, CRayImm* pcRay)
{
	//http://softsurfer.com/Archive/algorithm_0105/algorithm_0105.htm

	D3DXVECTOR3	u;
	D3DXVECTOR3	v;
	D3DXVECTOR3	p;
	D3DXVECTOR3	w;
	float		uDotv;
	float		uDotu;
	float		vDotv;
	float		wDotv;
	float		wDotu;
	float		s;
	float		t;
	float		denominator;

	//OI!  You've forgotten to check if the triangle is BEHIND the ray.

	if (PlaneRayIntersect(&p, (CPlaneImm*)pcTriangle, pcRay))
	{
		D3DXVec3Subtract(&w, &p, (D3DXVECTOR3*)&pcTriangle->asPoints[0]);
		D3DXVec3Subtract(&u, (D3DXVECTOR3*)&pcTriangle->asPoints[1], (D3DXVECTOR3*)&pcTriangle->asPoints[0]);
		D3DXVec3Subtract(&v, (D3DXVECTOR3*)&pcTriangle->asPoints[2], (D3DXVECTOR3*)&pcTriangle->asPoints[0]);
	
		uDotv = D3DXVec3Dot(&u, &v);
		uDotu = D3DXVec3Dot(&u, &u);
		vDotv = D3DXVec3Dot(&v, &v);
		wDotv = D3DXVec3Dot(&w, &v);
		wDotu = D3DXVec3Dot(&w, &u);

		denominator = (uDotv * uDotv) - (uDotu * vDotv);
        s = ((uDotv * wDotv) - (vDotv * wDotu)) / denominator;
		t = ((uDotv * wDotu) - (uDotu * wDotv)) / denominator;

		if ((s >= 0) && (t >= 0) && (s + t <= 1))
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL SphereRayIntersect(CSphereImm* pcSphere, CRayImm* pcRay)
{
	D3DXVECTOR3		v1;
	D3DXVECTOR3		v2;
	float			f;
	float			radiusSquare;
	float			sphereAlongRay;

	D3DXVec3Subtract(&v1, (D3DXVECTOR3*)&pcSphere->msPosition, (D3DXVECTOR3*)&pcRay->msStart);
	sphereAlongRay = D3DXVec3Dot(&v1, (D3DXVECTOR3*)&pcRay->msDirection);
	radiusSquare = pcSphere->mfRadius * pcSphere->mfRadius;

	if (sphereAlongRay >= 0)
	{
		//The sphere is infront of the ray...
		D3DXVec3Cross(&v2, &v1, (D3DXVECTOR3*)&pcRay->msDirection);
		f = D3DXVec3LengthSq(&v2);
		if (f < radiusSquare)
		{
			return TRUE;
		}
		return FALSE;
	}
	else
	{
		//The sphere is behind the ray but might still contain the staring point.
		D3DXVec3Subtract(&v2, (D3DXVECTOR3*)&pcRay->msStart, (D3DXVECTOR3*)&pcSphere->msPosition);
		f = D3DXVec3LengthSq(&v2);
		if (f < radiusSquare)
		{
			return TRUE;
		}
		return FALSE;
	}
}

