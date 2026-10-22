#ifndef __GERBER_EVALUATED_PRIMITIVE_H__
#define __GERBER_EVALUATED_PRIMITIVE_H__
#include "GerberNumbers.h"
#include "GerberApertureMacro.h"


//A macro primitive with its expressions evaluated.  The values are in the order of the specification:
//  Circle (1):				Exposure, Diameter, Center X, Center Y, Rotation
//  Vector line (20):		Exposure, Width, Start X, Start Y, End X, End Y, Rotation
//  Center line (21):		Exposure, Width, Height, Center X, Center Y, Rotation
//  Outline (4):			Exposure, Number of vertices n, Start X, Start Y, n subsequent X, Y points, Rotation
//  Polygon (5):			Exposure, Number of vertices, Center X, Center Y, Diameter, Rotation
//  Thermal (7):			Center X, Center Y, Outer diameter, Inner diameter, Gap thickness, Rotation
class CGerberEvaluatedPrimitive
{
protected:
	EGerberApertureMacroPrimitive	mePrimitive;
	CArrayGerberNumber				macValues;

public:
	void							Init(EGerberApertureMacroPrimitive ePrimitive);
	void							Kill(void);

	CNumber*						AddValue(void);

	EGerberApertureMacroPrimitive	GetPrimitive(void);
	bool							IsPrimitive(EGerberApertureMacroPrimitive ePrimitive);
	size							NumValues(void);
	CNumber*						GetValue(size uiIndex);
};


typedef CArrayTemplate<CGerberEvaluatedPrimitive>	CArrayGerberEvaluatedPrimitive;


class CGerberEvaluatedPrimitives
{
protected:
	CArrayGerberEvaluatedPrimitive	macPrimitives;

public:
	void						Init(void);
	void						Kill(void);

	CGerberEvaluatedPrimitive*	Add(EGerberApertureMacroPrimitive ePrimitive);

	size						NumPrimitives(void);
	CGerberEvaluatedPrimitive*	GetPrimitive(size uiIndex);
};


#endif // __GERBER_EVALUATED_PRIMITIVE_H__

