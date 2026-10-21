#ifndef __GERBER_VECTOR_SHAPES_H__
#define __GERBER_VECTOR_SHAPES_H__
#include "GerberVectorObject.h"
#include "GerberAperture.h"


//Contours for the strokes and apertures, added to the current part of pcObject.  Lengths are in millimetres.
void	AddGerberStroke(CGerberVectorObject* pcObject, SFloat64Vec2* psStart, SFloat64Vec2* psEnd, float64 fRadius);
void	AddGerberArcStroke(CGerberVectorObject* pcObject, SFloat64Vec2* psStart, SFloat64Vec2* psEnd, SFloat64Vec2* psCenter, bool bClockwise, float64 fRadius);
bool	AddGerberApertureShape(CGerberVectorObject* pcObject, CGerberAperture* pcAperture, float64 fUnit);
void	RotateGerberPoint(SFloat64Vec2* psPoint, float64 fDegrees);


#endif // __GERBER_VECTOR_SHAPES_H__

