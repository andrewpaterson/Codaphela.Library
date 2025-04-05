#ifndef __GERBER_APERTURE_MACRO_H__
#define __GERBER_APERTURE_MACRO_H__
#include "BaseLib/Chars.h"
#include "GerberCommandEnum.h"


enum EGerberApertureMacroPrimitive
{
	GAMP_Comment = 0,			// A comment string 4.5.1.
	GAMP_Circle = 1,			// Exposure, Diameter, Center X, Center Y[, Rotation]
	GAMP_Vector_Line = 20,		// Exposure, Width, Start X, Start Y, End X, End Y, Rotation 4.5.1.4
	GAMP_Center_Line = 21,		// Exposure, Width, Height, Center X, Center Y, Rotation 4.5.1.5
	GAMP_Outline = 4,			// Exposure, # of vertices, Start X, Start Y, Subsequent points..., Rotation 4.5.1.6
	GAMP_Polygon = 5,			// Exposure, # of vertices, Center X, Center Y, Diameter, Rotation 4.5.1.7
	GAMP_Thermal_Center = 7,	// X, Center Y, Outer diameter, Inner diameter, Gap, Rotation 4.5.1.8
};


class CGerberApertureMacro
{
protected:
	EGerberApertureMacroPrimitive	mePrimitive;

public:
			void Init(EGerberApertureMacroPrimitive ePrimitive);
	virtual void Kill(void) = 0;
};


#endif // __GERBER_APERTURE_MACRO_H__

