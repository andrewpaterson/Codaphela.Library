#include "GerberExpressionPosition.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberExpressionPosition::Init(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError)
{
	mX.Init(pcSymbols, bSkipWhitespace, bUseUserError);
	mY.Init(pcSymbols, bSkipWhitespace, bUseUserError);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberExpressionPosition::Kill(void)
{
	mY.Kill();
	mX.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberExpressionPosition::GetX(void)
{
	return &mX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberExpression* CGerberExpressionPosition::GetY(void)
{
	return &mY;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayGerberExpressionPositions::Kill(void)
{
	size						i;
	size						uiNumElements;
	CGerberExpressionPosition*	pcPosition;

	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcPosition = Get(i);
		pcPosition->Kill();
	}

	__CArrayGerberExpressionPositions::Kill();
}

