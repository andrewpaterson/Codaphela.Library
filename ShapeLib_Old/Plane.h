#ifndef __PLANE_H__
#define __PLANE_H__
#include <d3dx9.h>
#include "CoreLib\ArrayTemplate.h"


//Remember that a triangle is can be cast to a plane
class CPlaneImm
{
public:
	D3DVECTOR	sNormal;		
	float		d;			//d of plane.
	D3DVECTOR	sPosition;	//For a triangle this is the first of three points.

	//Set from three points
	void Set(D3DVECTOR* v1, D3DVECTOR* v2, D3DVECTOR* v3);
	void Set(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);
	void Set(D3DVECTOR* psPoint, D3DVECTOR* psNormal);
	void Set(void);
	void Copy(CPlaneImm* psSource);
};


typedef CArrayTemplate<CPlaneImm> CPlaneArray;


#endif __PLANE_H__

