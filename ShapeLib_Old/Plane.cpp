#include "Plane.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlaneImm::Set(D3DVECTOR* v1, D3DVECTOR* v2, D3DVECTOR* v3)
{
	D3DXVECTOR3 side1, side2;

	sPosition = (*v1);
	side1 = (D3DXVECTOR3)(*v2) - (D3DXVECTOR3)sPosition;
	side2 = (D3DXVECTOR3)(*v3) - (D3DXVECTOR3)sPosition;
	D3DXVec3Cross((D3DXVECTOR3*)&sNormal, (D3DXVECTOR3*)&side1, (D3DXVECTOR3*)&side2);
	D3DXVec3Normalize((D3DXVECTOR3*)&sNormal, (D3DXVECTOR3*)&sNormal);
	d = -D3DXVec3Dot((D3DXVECTOR3*)&sPosition, (D3DXVECTOR3*)&sNormal);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlaneImm::Set(D3DVECTOR* psPoint, D3DVECTOR* psNormal)
{
	sPosition = (*psPoint);
	sNormal = (*psNormal);
	D3DXVec3Normalize((D3DXVECTOR3*)&sNormal, (D3DXVECTOR3*)&sNormal);
	d = -D3DXVec3Dot((D3DXVECTOR3*)&sPosition, (D3DXVECTOR3*)&sNormal);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlaneImm::Set(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3)
{
	D3DVECTOR side1, side2;

	sPosition = D3DXVECTOR3(x1, y1, z1);
	side1 = D3DXVECTOR3(x2, y2, z2) - (D3DXVECTOR3)sPosition;
	side2 = D3DXVECTOR3(x3, y3, z3) - (D3DXVECTOR3)sPosition;

	D3DXVec3Cross((D3DXVECTOR3*)&sNormal, (D3DXVECTOR3*)&side1, (D3DXVECTOR3*)&side2);
	D3DXVec3Normalize((D3DXVECTOR3*)&sNormal, (D3DXVECTOR3*)&sNormal);
	d = -D3DXVec3Dot((D3DXVECTOR3*)&sPosition, (D3DXVECTOR3*)&sNormal);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlaneImm::Set(void)
{
	d = -D3DXVec3Dot((D3DXVECTOR3*)&sPosition, (D3DXVECTOR3*)&sNormal);
}

