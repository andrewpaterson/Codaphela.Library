#ifndef __GERBER_VECTOR_SHAPES_H__
#define __GERBER_VECTOR_SHAPES_H__
#include "SupportLib/VectorShapes.h"
#include "GerberVectorObject.h"
#include "GerberAperture.h"


//Contours for the apertures, added to the current part of pcObject.  Lengths are in millimetres.
bool	AddGerberApertureShape(CGerberVectorObject* pcObject, CGerberAperture* pcAperture, float64 fUnit);


#endif // __GERBER_VECTOR_SHAPES_H__

