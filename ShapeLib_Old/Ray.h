#ifndef __RAY_H__
#define __RAY_H__
#include <d3dx9.h>


class CRayImm
{
public:
	D3DVECTOR	msStart;  //Starting point of line
	D3DVECTOR	msDirection;  //unit direction from start to end

	void SetFromPositions(D3DVECTOR* s, D3DVECTOR* e);
	void SetFromDirection(D3DVECTOR* sStartOfRay, D3DVECTOR* sDirection);
	void SetFromDirection(float xs, float ys, float zs, float xd, float yd, float zd);
	void Copy(CRayImm* source);
	void Dump(void);
 };


#endif __RAY_H__