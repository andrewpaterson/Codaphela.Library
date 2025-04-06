#ifndef __GERBER_APERTURE_MACRO_POLYGON_H__
#define __GERBER_APERTURE_MACRO_POLYGON_H__
#include "BaseLib/Chars.h"
#include "GerberExpression.h"
#include "GerberExpressionPosition.h"
#include "GerberApertureMacro.h"


class CGerberApertureMacroPolygon : public CGerberApertureMacro
{
protected:
	CGerberExpression			mcExposure;
	CGerberExpression			mcNumberOfVertices;  // 3 <= n <= 12. An integer.The first vertex is on the positive X - axis through the center point when the rotation angle is zero.
	CGerberExpressionPosition	mcCenter;
	CGerberExpression			mcDiameter;  // of the circumscribed circle >= 0.
	CGerberExpression			mcRotation;

public:
	void				Init(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError);
	void				Kill(void);

	bool				IsPolygon(void);

	CGerberExpression*	GetExposure(void);
	CGerberExpression*	GetNumberOfVertices(void);
	CGerberExpression*	GetCenterX(void);
	CGerberExpression*	GetCenterY(void);
	CGerberExpression*	GetDiameter(void);
	CGerberExpression*	GetRotation(void);
};


#endif // __GERBER_APERTURE_MACRO_POLYGON_H__

