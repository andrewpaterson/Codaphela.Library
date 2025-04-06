#ifndef __GERBER_APERTURE_MACRO_VECTOR_LINE_H__
#define __GERBER_APERTURE_MACRO_VECTOR_LINE_H__
#include "BaseLib/Chars.h"
#include "GerberExpression.h"
#include "GerberExpressionPosition.h"
#include "GerberApertureMacro.h"


class CGerberApertureMacroVectorLine : public CGerberApertureMacro
{
protected:
	CGerberExpression			mcExposure;
	CGerberExpression			mcWidth;
	CGerberExpressionPosition	mcStart;
	CGerberExpressionPosition	mcEnd;
	CGerberExpression			mcRotation; // Angle, in degrees counterclockwise.  The primitive is rotated around the origin of the macro definition, i.e.the(0, 0) point of macro coordinates (not the center of the line).

public:
	void				Init(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError);
	void				Kill(void);

	bool				IsVectorLine(void);

	CGerberExpression*	GetExposure(void);
	CGerberExpression*	GetWidth(void);
	CGerberExpression*	GetStartX(void);
	CGerberExpression*	GetStartY(void);
	CGerberExpression*	GetEndX(void);
	CGerberExpression*	GetEndY(void);
	CGerberExpression*	GetRotation(void);
};


#endif // __GERBER_APERTURE_MACRO_VECTOR_LINE_H__

