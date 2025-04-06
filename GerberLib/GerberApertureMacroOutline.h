#ifndef __GERBER_APERTURE_MACRO_OUTLINE_H__
#define __GERBER_APERTURE_MACRO_OUTLINE_H__
#include "BaseLib/Chars.h"
#include "GerberExpression.h"
#include "GerberExpressionPosition.h"
#include "GerberApertureMacro.h"


class CGerberApertureMacroOutline : public CGerberApertureMacro
{
protected:
	CGerberExpression					mcExposure;
	CArrayGerberExpressionPositions		mcPositions;
	CGerberExpression					mcRotation;

public:
	void						Init(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError);
	void						Kill(void);

	bool						IsOutline(void);

	CGerberExpressionPosition*	Add(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError);
	CGerberExpression*			GetExposure(void);
	CGerberExpression*			GetRotation(void);
};


#endif // __GERBER_APERTURE_MACRO_OUTLINE_H__

