#ifndef __GERBER_EXPRESSION_H__
#define __GERBER_EXPRESSION_H__
#include "BaseLib/Calculator.h"


class CGerberExpression
{
public:
	CCalculator			mcCalculator;
	CCalcExpression*	mpcExpression;

public:
	void			Init(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError);
	void			Kill(void);

	CCalculator*	GetCalculator(void);
	void			SetCalcExpression(CCalcExpression* pcExpression);
	CNumber*		GetConstNumber(void);
};


#endif // __GERBER_EXPRESSION_H__

