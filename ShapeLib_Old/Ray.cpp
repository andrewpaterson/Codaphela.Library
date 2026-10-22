#include "Ray.h"
#include <stdio.h>


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRayImm::SetFromPositions(D3DVECTOR* s, D3DVECTOR* e)
{
	msStart = (*s);
	D3DXVec3Subtract((D3DXVECTOR3*)&msDirection, (D3DXVECTOR3*)e, (D3DXVECTOR3*)s);
	D3DXVec3Normalize((D3DXVECTOR3*)&msDirection, (D3DXVECTOR3*)&msDirection);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRayImm::SetFromDirection(D3DVECTOR* sStartOfRay, D3DVECTOR* sDirection)
{
	msStart = (*sStartOfRay);
	msDirection = (*sDirection);
	D3DXVec3Normalize((D3DXVECTOR3*)&msDirection, (D3DXVECTOR3*)&msDirection);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRayImm::SetFromDirection(float xs, float ys, float zs, float xd, float yd, float zd)
{
	(D3DXVECTOR3)msStart = D3DXVECTOR3(xs, ys, zs);
	(D3DXVECTOR3)msDirection = D3DXVECTOR3(xd, yd, zd);
	D3DXVec3Normalize((D3DXVECTOR3*)&msDirection, (D3DXVECTOR3*)&msDirection);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRayImm::Dump(void)
{
	char sz[1024];

	sprintf(sz, "(%.2f, %.2f, %.2f) -> (%.2f, %.2f, %.2f)\n", msStart.x, msStart.y, msStart.z, msDirection.x, msDirection.y, msDirection.z);
	OutputDebugString(sz);
}

