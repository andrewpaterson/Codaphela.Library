#ifndef __INTERSECTIONS_H__
#define __INTERSECTIONS_H__
#include <stdlib.h>
#include <d3dx9.h>
#include "Distances.h"


BOOL SphereSphereIntersect(CSphereImm* pcSphere1, CSphereImm* pcSphere2);
BOOL SphereTriangleIntersect(CSphereImm* pcSphere, CTriangleImm* pcTriangle);
BOOL SphereConeIntersect(CSphereImm* pcSphere, CConeImm* pcCone);
BOOL SphereRayIntersect(CSphereImm* pcSphere, CRayImm* pcRay);
BOOL PlaneRayIntersect(D3DVECTOR* pcDest, CPlaneImm* pcPlane, CRayImm* pcRay);
BOOL TriangleRayIntersect(CTriangleImm* pcTriangle, CRayImm* pcRay);


#endif __INTERSECTIONS_H__

