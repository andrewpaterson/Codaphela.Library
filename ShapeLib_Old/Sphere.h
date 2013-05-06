#ifndef __SPHERE_H__
#define __SPHERE_H__
#include <d3dx9.h>
#include "CoreLib\ArrayTemplate.h"


class CSphereImm
{
public:
	D3DVECTOR	msPosition;
	float		mfRadius;

	//Set from center of sphere and radius
	void Set(D3DVECTOR* pos, float rad);
	void Set(float x, float y, float z, float rad);
	void SetFromPointsUsingAveragePosition(D3DXVECTOR3* psPoints, int iStride, int iNumPoints);
	void SetFromPointsUsingBestFit(D3DXVECTOR3* psPoints, int iStride, int iNumPoints);
	void BoundingBox(D3DVECTOR* min, D3DVECTOR* max);
	void Copy(CSphereImm* source);
};


#endif __SPHERE_H__

