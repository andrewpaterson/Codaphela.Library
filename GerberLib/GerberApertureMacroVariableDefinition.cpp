#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "GerberMacroVariables.h"
#include "GerberApertureMacroVariableDefinition.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberApertureMacroVariableDefinition::Init(uint32 uiVariable, CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError)
{
	CGerberApertureMacro::Init(GAMP_Variable_Definition);
	muiVariable = uiVariable;
	mcExpression.Init(pcSymbols, bSkipWhitespace, bUseUserError);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberApertureMacroVariableDefinition::Kill(void)
{
	mcExpression.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberApertureMacroVariableDefinition::IsVariableDefinition(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CGerberApertureMacroVariableDefinition::GetVariable(void)
{
	return muiVariable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroVariableDefinition::GetExpression(void)
{
	return &mcExpression;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberApertureMacroVariableDefinition::Evaluate(CGerberMacroVariables* pcVariables, CGerberEvaluatedPrimitives* pcPrimitives)
{
	CNumber	cValue;

	if (!mcExpression.Evaluate(pcVariables, &cValue))
	{
		return false;
	}

	//Macro variables cannot be redefined, including the ones set by the AD command.  4.5.4.3
	if (!pcVariables->Set(muiVariable, &cValue))
	{
		gcLogger.Error2(__METHOD__, " Aperture macro variable $", IntToString(muiVariable), " cannot be redefined.", NULL);
		return false;
	}
	return true;
}

