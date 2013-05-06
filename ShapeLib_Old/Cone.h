#ifndef __CONE_H__
#define __CONE_H__
#include <d3dx9.h>

//This must be rewritten to use a LineSegment instead of all the other crap.


class CConeImm
{
public:
	D3DVECTOR	msPosition;  //Position of tip of cone.
	D3DVECTOR	msDirection;  //Unit direction to base of cone.
	float		mfLength;  //Height of cone.
	float		mfRadius;  //Width of cone at one unit.

	void Set(D3DVECTOR* psPos, D3DVECTOR* psDir, float fLen, float fRad);
	void Copy(CConeImm* source);
};


#endif __CONE_H__

