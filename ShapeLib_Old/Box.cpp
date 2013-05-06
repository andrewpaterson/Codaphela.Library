#include "Box.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBoxImm::Init(void)
{
	CConvexHullImm::Init();
	D3DXVec3Assign((D3DXVECTOR3*)&msMinimum, 0.0f, 0.0f, 0.0f);
	D3DXVec3Assign((D3DXVECTOR3*)&msMaximum, 0.0f, 0.0f, 0.0f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBoxImm::Kill(void)
{
	CConvexHullImm::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBoxImm::Set(D3DVECTOR* psMinimum, D3DVECTOR* psMaximum)
{
	msMinimum = *psMinimum;
	msMaximum = *psMaximum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBoxImm::SetFromPoints(D3DXVECTOR3* psPoints, int iStride, int iNumPoints)
{
	int				i;

	D3DXVec3Assign((D3DXVECTOR3*)&msMinimum, LARGE_NUMBER, LARGE_NUMBER, LARGE_NUMBER);
	D3DXVec3Assign((D3DXVECTOR3*)&msMaximum, -LARGE_NUMBER, -LARGE_NUMBER, -LARGE_NUMBER);
	for (i = 0; i < iNumPoints; i++)
	{
		psPoints = (D3DXVECTOR3*)((size_t)(((int)((size_t)((void*)psPoints)) + iStride)));

		//Greater than...
		if (psPoints->x > msMaximum.x)
			msMaximum.x = psPoints->x;
		if (psPoints->y > msMaximum.y)
			msMaximum.y = psPoints->y;
		if (psPoints->z > msMaximum.z)
			msMaximum.z = psPoints->z;

		//Less than...
		if (psPoints->x < msMinimum.x)
			msMinimum.x = psPoints->x;
		if (psPoints->y < msMinimum.y)
			msMinimum.y = psPoints->y;
		if (psPoints->z < msMinimum.z)
			msMinimum.z = psPoints->z;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBoxImm::Save(CChunkFile* pcFile)
{
	CConvexHullImm::Save(pcFile);
	pcFile->WriteData(&msMinimum, sizeof(D3DVECTOR));
	pcFile->WriteData(&msMaximum, sizeof(D3DVECTOR));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBoxImm::Load(CChunkFile* pcFile)
{
	CConvexHullImm::Load(pcFile);
	pcFile->ReadData(&msMinimum, sizeof(D3DVECTOR));
	pcFile->ReadData(&msMaximum, sizeof(D3DVECTOR));
}

