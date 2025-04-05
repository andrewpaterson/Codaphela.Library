#ifndef __GERBER_APERTURE_MACRO_CIRCLE_H__
#define __GERBER_APERTURE_MACRO_CIRCLE_H__
#include "BaseLib/Chars.h"
#include "GerberApertureMacro.h"
#include "GerberExpression.h"


class CGerberApertureMacroCircle : public CGerberApertureMacro
{
protected:
	CGerberExpression	mcExposure;  //Off == 0, On == 1
	CGerberExpression	mcDiameter;  //? 0
	CGerberExpression	mcCenterX;
	CGerberExpression	mcCenterY;
	CGerberExpression	mcRotation;

public:
	void				Init(void);
	void				Kill(void);

	CGerberExpression*	GetExposure(void);
	CGerberExpression*	GetDiameter(void);
	CGerberExpression*	GetCenterX(void);
	CGerberExpression*	GetCenterY(void);
	CGerberExpression*	GetRotation(void);
};


#endif // __GERBER_APERTURE_MACRO_CIRCLE_H__

