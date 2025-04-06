#ifndef __GERBER_APERTURE_MACRO_THERMAL_LINE_H__
#define __GERBER_APERTURE_MACRO_THERMAL_LINE_H__
#include "BaseLib/Chars.h"
#include "GerberExpression.h"
#include "GerberExpressionPosition.h"
#include "GerberApertureMacro.h"


class CGerberApertureMacroThermal : public CGerberApertureMacro
{
protected:
	CGerberExpressionPosition	mcCenter;
	CGerberExpression			mcOuterDiameter;  // Outer diameter > inner diameter
	CGerberExpression			mcInnerDiameter;  // >= 0;
	CGerberExpression			mcGapThickness;
	CGerberExpression			mcRotation;

public:
	void				Init(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError);
	void				Kill(void);

	bool				IsThermal(void);

	CGerberExpression*	GetCenterX(void);
	CGerberExpression*	GetCenterY(void);
	CGerberExpression*  GetOuterDiameter(void);
	CGerberExpression*	GetInnerDiameter(void);
	CGerberExpression*	GetGapThickness(void);
	CGerberExpression*	GetRotation(void);
};


#endif // __GERBER_APERTURE_MACRO_THERMAL_LINE_H__

