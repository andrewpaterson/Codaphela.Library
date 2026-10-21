#include "BaseLib/Logger.h"
#include "GerberMacroVariables.h"
#include "GerberEvaluatedPrimitive.h"
#include "GerberApertureMacroPolygon.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberApertureMacroPolygon::Init(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError)
{
	CGerberApertureMacro::Init(GAMP_Polygon);
	mcExposure.Init(pcSymbols, bSkipWhitespace, bUseUserError);
	mcNumberOfVertices.Init(pcSymbols, bSkipWhitespace, bUseUserError);
	mcCenter.Init(pcSymbols, bSkipWhitespace, bUseUserError);
	mcDiameter.Init(pcSymbols, bSkipWhitespace, bUseUserError);
	mcRotation.Init(pcSymbols, bSkipWhitespace, bUseUserError);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberApertureMacroPolygon::Kill(void)
{
	mcRotation.Kill();
	mcDiameter.Kill();
	mcCenter.Kill();
	mcNumberOfVertices.Kill();
	mcExposure.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroPolygon::GetExposure(void)
{
	return &mcExposure;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroPolygon::GetNumberOfVertices(void)
{
	return &mcNumberOfVertices;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroPolygon::GetDiameter(void)
{
	return &mcDiameter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroPolygon::GetCenterX(void)
{
	return mcCenter.GetX();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroPolygon::GetCenterY(void)
{
	return mcCenter.GetY();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroPolygon::GetRotation(void)
{
	return &mcRotation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberApertureMacroPolygon::IsPolygon(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberApertureMacroPolygon::Evaluate(CGerberMacroVariables* pcVariables, CGerberEvaluatedPrimitives* pcPrimitives)
{
	CGerberEvaluatedPrimitive*	pcPrimitive;
	CNumber*					pcVertices;
	CNumber						cLimit;

	pcPrimitive = pcPrimitives->Add(GAMP_Polygon);
	if (!EvaluateExposure(&mcExposure, pcVariables, pcPrimitive))
	{
		return false;
	}

	//3 to 12 vertices.  4.5.1.7
	pcVertices = pcPrimitive->AddValue();
	if (!mcNumberOfVertices.Evaluate(pcVariables, pcVertices))
	{
		return false;
	}
	if (!pcVertices->IsInteger() || pcVertices->LessThan(cLimit.Init(3)) || pcVertices->GreaterThan(cLimit.Init(12)))
	{
		gcLogger.Error2(__METHOD__, " Aperture macro polygon must have 3 to 12 vertices.", NULL);
		return false;
	}

	return EvaluateExpression(mcCenter.GetX(), pcVariables, pcPrimitive) &&
		   EvaluateExpression(mcCenter.GetY(), pcVariables, pcPrimitive) &&
		   EvaluateExpression(&mcDiameter, pcVariables, pcPrimitive) &&
		   EvaluateExpression(&mcRotation, pcVariables, pcPrimitive);
}

