#ifndef __SHAPE_TRANSFORMER_H__
#define __SHAPE_TRANSFORMER_H__
#include "Sphere.h"
#include "Cone.h"
#include "Triangle.h"
#include "Plane.h"
#include "LineSegment.h"
#include "LineInfinite.h"
#include "Ray.h"


//The shape transformers allow the shapes to reference externally changing parameters
//(eg: vertex positions, normals etc...) when it would be wasteful to store shared
//data between.  It also means that if any data needs to be regenerated, it can be
//without something or other...


class CSphereTrans
{
public:
	D3DVECTOR*	mpsPosition;
	float		mfRadius;

	void		Transform(CSphereImm* pcSphere);
};


class CConeTrans
{
public:
	D3DVECTOR*	mpsPosition;  //Position of tip of cone.
	D3DVECTOR*	mpsDirection;  //Unit direction to base of cone.
	float		mfLength;  //Height of cone.
	float		mfRadius;  //Width of cone at one unit.

	void		Transform(CConeImm* pcCone);
};


class CTriangleTrans
{
public:
	D3DVECTOR*	mpsNormal;		
	D3DVECTOR*	mapsPoints[3];

	void Transform(CTriangleImm* pcTriangle);
};


class CLineSegmentTrans
{
public:
	D3DVECTOR*	mpsStart;
	D3DVECTOR*	mpsEnd;
	D3DVECTOR*	mpsDirection;
	float		fLength;

	void Transform(CLineSegmentImm* pcLineSegment);
};


#endif __SHAPE_TRANSFORMER_H__

