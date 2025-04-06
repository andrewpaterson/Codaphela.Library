#ifndef __GERBER_APERTURE_MACRO_CENTER_LINE_H__
#define __GERBER_APERTURE_MACRO_CENTER_LINE_H__
#include "BaseLib/Chars.h"
#include "GerberApertureMacro.h"
#include "GerberExpression.h"
#include "GerberExpressionPosition.h"


class CGerberApertureMacroCenterLine : public CGerberApertureMacro
{
protected:
	CGerberExpression			mcExposure;
	CGerberExpression			mcWidth;
	CGerberExpression			mcHeight;
	CGerberExpressionPosition	mcCenter;
	CGerberExpression			mcRotation;

public:
	void				Init(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError);
	void				Kill(void);

	bool				IsCenterLine(void);

	CGerberExpression*	GetExposure(void);
	CGerberExpression*	GetWidth(void);
	CGerberExpression*	GetHeight(void);
	CGerberExpression*	GetCenterX(void);
	CGerberExpression*	GetCenterY(void);
	CGerberExpression*	GetRotation(void);
};


#endif // __GERBER_APERTURE_MACRO_CENTER_LINE_H__

