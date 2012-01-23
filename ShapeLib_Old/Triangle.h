#ifndef __TRIANGLE_H
#define __TRIANGLE_H
#include "Plane.h"


//Remember that a triangle is can be cast to a plane
class CTriangleImm
{
public:
	D3DVECTOR	sNormal;		
	float		d;			//d of plane.
	D3DVECTOR	asPoints[3];
	
	void Set(void);
};


#endif

