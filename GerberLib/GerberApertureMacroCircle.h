#ifndef __GERBER_APERTURE_MACRO_CIRCLE_H__
#define __GERBER_APERTURE_MACRO_CIRCLE_H__
#include "BaseLib/Chars.h"
#include "GerberApertureMacro.h"
#include "GerberExpression.h"
#include "GerberExpressionPosition.h"


class CGerberApertureMacroCircle : public CGerberApertureMacro
{
protected:
	CGerberExpression			mcExposure;  //Off == 0, On == 1
	CGerberExpression			mcDiameter;  //>= 0
	CGerberExpressionPosition	mcCenter;
	CGerberExpression			mcRotation;

public:
	void				Init(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError);
	void				Kill(void);

	bool				IsCircle(void);

	CGerberExpression*	GetExposure(void);
	CGerberExpression*	GetDiameter(void);
	CGerberExpression*	GetCenterX(void);
	CGerberExpression*	GetCenterY(void);
	CGerberExpression*	GetRotation(void);
};


#endif // __GERBER_APERTURE_MACRO_CIRCLE_H__

