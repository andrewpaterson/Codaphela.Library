#ifndef __DISTANCES_H__
#define __DISTANCES_H__
#include "Shape.h"


float PointDistanceFromPlane(CPlaneImm* pcPlane, D3DVECTOR* psPoint);
float PointDistanceFromSphere(CSphereImm* pcSphere, D3DVECTOR* point);
float PointDistanceFromPoint(D3DVECTOR* point1, D3DVECTOR* point2);
float PointDistanceSquaredFromPoint(D3DVECTOR* point1, D3DVECTOR* point2);
float PointDistanceFromLineInfinite(CLineInfinite* pcLine, D3DVECTOR* point);
float PointDistanceAlongLineInfinite(CLineInfinite* pcLine, D3DVECTOR* point);
float PointDistanceFromLineSegment(CLineSegmentImm* pcLine, D3DVECTOR* point);
float PointDistanceSquaredFromLineSegment(CLineSegmentImm* pcLine, D3DVECTOR* point);
//float PointDistanceFromTriangle(CTriangle* pcTriangle, D3DVECTOR* psPoint);
float PointDistanceFromShape(CShapeImm* pcShape, D3DVECTOR* psPoint);

//Direction is always normalised.
float PointDirectedDistanceFromPlane(CPlaneImm* pcPlane, D3DVECTOR* psPoint, D3DVECTOR* psDirection);
float PointDirectedDistanceFromSphere(CSphereImm* pcSphere, D3DVECTOR* psPoint, D3DVECTOR* psDirection);
float PointDirectedDistanceFromShape(CShapeImm* pcShape, D3DVECTOR* psPoint, D3DVECTOR* psDirection);

#endif __DISTANCES_H__

