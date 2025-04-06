#ifndef __GERBER_EXPRESSION_POSITION_H__
#define __GERBER_EXPRESSION_POSITION_H__
#include "BaseLib/ArrayTemplate.h"
#include "GerberExpression.h"


class CGerberExpressionPosition
{
protected:
	CGerberExpression	mX;
	CGerberExpression	mY;

public:
	void				Init(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError);
	void				Kill(void);

	CGerberExpression*	GetX(void);
	CGerberExpression*	GetY(void);
};


typedef CArrayTemplate<CGerberExpressionPosition>	__CArrayGerberExpressionPositions;


class CArrayGerberExpressionPositions : public __CArrayGerberExpressionPositions
{
public:
	void Kill(void);
};


#endif // __GERBER_EXPRESSION_POSITION_H__


