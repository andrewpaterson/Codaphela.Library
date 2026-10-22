#include "Cone.h"


void CConeImm::Set(D3DVECTOR* psPos, D3DVECTOR* psDir, float fLen, float fRad)
{
	msPosition = (*psPos);
	msDirection = (*psDir);
	D3DXVec3Normalize((D3DXVECTOR3*)&msDirection, (D3DXVECTOR3*)&msDirection);
	mfLength = fLen;
	mfRadius = fRad;
}


void CConeImm::Copy(CConeImm* source)
{
	memcpy(this, source, sizeof(CConeImm));
}

