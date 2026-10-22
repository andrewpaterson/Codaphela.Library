#ifndef __BOX_H__
#define __BOX_H__
#include <d3dx9.h>
#include "ConvexHull.h"
#include "DirectXLib\D3DVectorHelper.h"
#include "CoreLib\Numbers.h"


class CBoxImm : public CConvexHullImm
{
public:
	D3DVECTOR	msMinimum;
	D3DVECTOR	msMaximum;

	void Init(void);
	void Kill(void);
	void Set(D3DVECTOR* psMinimum, D3DVECTOR* psMaximum);
	void SetFromPoints(D3DXVECTOR3* psPoints, int iStride, int iNumPoints);

	void Save(CChunkFile* pcFile);
	void Load(CChunkFile* pcFile);
};


#endif __BOX_H__

