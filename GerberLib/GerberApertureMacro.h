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
	GAMP_Variable_Definition = -1,	// $n=expression.  Not a primitive. 4.5.4.3
};


class CGerberMacroVariables;
class CGerberExpression;
class CGerberEvaluatedPrimitive;
class CGerberEvaluatedPrimitives;


class CGerberApertureMacro
{
protected:
	EGerberApertureMacroPrimitive	mePrimitive;

public:
			void	Init(EGerberApertureMacroPrimitive ePrimitive);
	virtual void	Kill(void) = 0;

	virtual bool	IsComment(void);
	virtual bool	IsCircle(void);
	virtual bool	IsCenterLine(void);
	virtual bool	IsOutline(void);
	virtual bool	IsPolygon(void);
	virtual bool	IsVectorLine(void);
	virtual bool	IsThermal(void);
	virtual bool	IsVariableDefinition(void);

	//Adds the evaluated primitive to pcPrimitives or, for a variable definition, sets the variable.
	virtual bool	Evaluate(CGerberMacroVariables* pcVariables, CGerberEvaluatedPrimitives* pcPrimitives);

protected:
			bool	EvaluateExpression(CGerberExpression* pcExpression, CGerberMacroVariables* pcVariables, CGerberEvaluatedPrimitive* pcPrimitive);
			bool	EvaluateExposure(CGerberExpression* pcExpression, CGerberMacroVariables* pcVariables, CGerberEvaluatedPrimitive* pcPrimitive);
};


#endif // __GERBER_APERTURE_MACRO_H__

