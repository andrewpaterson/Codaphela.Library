#include "GerberApertureMacroThermal.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberApertureMacroThermal::Init(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError)
{
	CGerberApertureMacro::Init(GAMP_Vector_Line);
	mcCenter.Init(pcSymbols, bSkipWhitespace, bUseUserError);
	mcOuterDiameter.Init(pcSymbols, bSkipWhitespace, bUseUserError);
	mcInnerDiameter.Init(pcSymbols, bSkipWhitespace, bUseUserError);
	mcGapThickness.Init(pcSymbols, bSkipWhitespace, bUseUserError);
	mcRotation.Init(pcSymbols, bSkipWhitespace, bUseUserError);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberApertureMacroThermal::Kill(void)
{
	mcCenter.Kill();
	mcOuterDiameter.Kill();
	mcInnerDiameter.Kill();
	mcGapThickness.Kill();
	mcRotation.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroThermal::GetCenterX(void)
{
	return mcCenter.GetX();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroThermal::GetCenterY(void)
{
	return mcCenter.GetY();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroThermal::GetOuterDiameter(void)
{
	return &mcOuterDiameter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroThermal::GetInnerDiameter(void)
{
	return &mcInnerDiameter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroThermal::GetGapThickness(void)
{
	return &mcGapThickness;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroThermal::GetRotation(void)
{
	return &mcRotation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberApertureMacroThermal::IsThermal(void)
{
	return true;
}

