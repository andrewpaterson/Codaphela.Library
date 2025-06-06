#include "GerberApertureMacroVectorLine.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberApertureMacroVectorLine::Init(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError)
{
	CGerberApertureMacro::Init(GAMP_Vector_Line);
	mcExposure.Init(pcSymbols, bSkipWhitespace, bUseUserError);
	mcWidth.Init(pcSymbols, bSkipWhitespace, bUseUserError);
	mcStart.Init(pcSymbols, bSkipWhitespace, bUseUserError);
	mcEnd.Init(pcSymbols, bSkipWhitespace, bUseUserError);
	mcRotation.Init(pcSymbols, bSkipWhitespace, bUseUserError);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberApertureMacroVectorLine::Kill(void)
{
	mcRotation.Kill();
	mcEnd.Kill();
	mcStart.Kill();
	mcWidth.Kill();
	mcExposure.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroVectorLine::GetExposure(void)
{
	return &mcExposure;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroVectorLine::GetWidth(void)
{
	return &mcWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroVectorLine::GetStartX(void)
{
	return mcStart.GetX();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroVectorLine::GetStartY(void)
{
	return mcStart.GetY();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroVectorLine::GetEndX(void)
{
	return mcEnd.GetX();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroVectorLine::GetEndY(void)
{
	return mcEnd.GetY();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberApertureMacroVectorLine::GetRotation(void)
{
	return &mcRotation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberApertureMacroVectorLine::IsVectorLine(void)
{
	return true;
}

