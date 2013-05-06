#include "LineSegment.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineSegmentImm::Set(void)
{
	D3DXVec3Subtract((D3DXVECTOR3*)&msDirection, (D3DXVECTOR3*)&msEnd, (D3DXVECTOR3*)&msStart);
	mfLength = D3DXVec3Length((D3DXVECTOR3*)&msDirection);
	(D3DXVECTOR3)msDirection /= mfLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineSegmentImm::Set(D3DVECTOR* sStart, D3DVECTOR* sEnd)
{
	msStart = (*sStart);
	msEnd = (*sEnd);
	D3DXVec3Subtract((D3DXVECTOR3*)&msDirection, (D3DXVECTOR3*)&msEnd, (D3DXVECTOR3*)&msStart);
	mfLength = D3DXVec3Length((D3DXVECTOR3*)&msDirection);
	(D3DXVECTOR3)msDirection /= mfLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineSegmentImm::Set(float x1, float y1, float z1, float x2, float y2, float z2)
{
	(D3DXVECTOR3)msStart = D3DXVECTOR3(x1, y1, z1);
	msEnd = D3DXVECTOR3(x2, y2, z2);
	D3DXVec3Subtract((D3DXVECTOR3*)&msDirection, (D3DXVECTOR3*)&msEnd, (D3DXVECTOR3*)&msStart);
	mfLength = D3DXVec3Length((D3DXVECTOR3*)&msDirection);
	(D3DXVECTOR3)msDirection /= mfLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineSegmentImm::Save(CChunkFile* pcFile)
{
	pcFile->WriteData(this, sizeof(CLineSegmentImm));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineSegmentImm::Load(CChunkFile* pcFile)
{
	pcFile->ReadData(this, sizeof(CLineSegmentImm));
}

