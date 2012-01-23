#ifndef __SHAPES_H__
#define __SHAPES_H__
#include "Sphere.h"
#include "Cone.h"
#include "ConvexHull.h"
#include "Triangle.h"
#include "Ray.h"
#include "LineSegment.h"
#include "Plane.h"
#include "LineInfinite.h"
#include "Intersections.h"
#include "ShapeTransformer.h"
#include "Shape.h"


typedef CArrayTemplate<CSphereImm>			CArraySphere;
typedef CArrayTemplate<CSphereTrans>		CArraySphereTrans;
typedef CArrayTemplate<CConeImm>			CArrayCone;
typedef CArrayTemplate<CConeTrans>			CArrayConeTrans;
typedef CArrayTemplate<CTriangleImm>		CArrayTriangle;
typedef CArrayTemplate<CTriangleTrans>		CArrayTriangleTrans;
typedef CArrayTemplate<CLineSegmentImm>		CArrayLineSegment;


#endif __SHAPES_H__

