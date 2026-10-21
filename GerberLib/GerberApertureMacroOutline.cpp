#include "BaseLib/Logger.h"
#include "GerberMacroVariables.h"
#include "GerberEvaluatedPrimitive.h"
#include "GerberApertureMacroOutline.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberApertureMacroOutline::Init(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError)
{
	CGerberApertureMacro::Init(GAMP_Outline);

	mcExposure.Init(pcSymbols, bSkipWhitespace, bUseUserError);
	mcPositions.Init();
	mcRotation.Init(pcSymbols, bSkipWhitespace, bUseUserError);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberApertureMacroOutline::Kill(void)
{
	mcRotation.Kill();
	mcPositions.Kill();
	mcExposure.Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroOutline::GetExposure(void)
{
	return &mcExposure;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroOutline::GetRotation(void)
{
	return &mcRotation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpressionPosition* CGerberApertureMacroOutline::Add(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError)
{
	CGerberExpressionPosition*	pcPosition;

	pcPosition = mcPositions.Add();
	pcPosition->Init(pcSymbols, bSkipWhitespace, bUseUserError);
	return pcPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberApertureMacroOutline::IsOutline(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGerberApertureMacroOutline::NumPositions(void)
{
	return mcPositions.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpressionPosition* CGerberApertureMacroOutline::GetPosition(size uiIndex)
{
	return mcPositions.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberApertureMacroOutline::Evaluate(CGerberMacroVariables* pcVariables, CGerberEvaluatedPrimitives* pcPrimitives)
{
	CGerberEvaluatedPrimitive*	pcPrimitive;
	CGerberExpressionPosition*	pcPosition;
	size						i;
	size						uiNumPositions;

	//The start point and one point per vertex; at least 3 vertices.  4.5.1.6
	uiNumPositions = mcPositions.NumElements();
	if (uiNumPositions < 4)
	{
		gcLogger.Error2(__METHOD__, " Aperture macro outline must have at least 3 vertices.", NULL);
		return false;
	}

	pcPrimitive = pcPrimitives->Add(GAMP_Outline);
	if (!EvaluateExposure(&mcExposure, pcVariables, pcPrimitive))
	{
		return false;
	}
	pcPrimitive->AddValue()->Init((int32)(uiNumPositions - 1));

	for (i = 0; i < uiNumPositions; i++)
	{
		pcPosition = mcPositions.Get(i);
		if (!EvaluateExpression(pcPosition->GetX(), pcVariables, pcPrimitive) ||
			!EvaluateExpression(pcPosition->GetY(), pcVariables, pcPrimitive))
		{
			return false;
		}
	}
	return EvaluateExpression(&mcRotation, pcVariables, pcPrimitive);
}

