#ifndef __GERBER_APERTURE_MACRO_VARIABLE_DEFINITION_H__
#define __GERBER_APERTURE_MACRO_VARIABLE_DEFINITION_H__
#include "GerberApertureMacro.h"
#include "GerberExpression.h"


//$n=expression
class CGerberApertureMacroVariableDefinition : public CGerberApertureMacro
{
protected:
	uint32				muiVariable;
	CGerberExpression	mcExpression;

public:
	void				Init(uint32 uiVariable, CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError);
	void				Kill(void);

	bool				IsVariableDefinition(void);

	bool				Evaluate(CGerberMacroVariables* pcVariables, CGerberEvaluatedPrimitives* pcPrimitives);

	uint32				GetVariable(void);
	CGerberExpression*	GetExpression(void);
};


#endif // __GERBER_APERTURE_MACRO_VARIABLE_DEFINITION_H__

