#ifndef __CALCULATOR_ERROR_H__
#define __CALCULATOR_ERROR_H__
#include "Chars.h"


class CCalculatorError
{
protected:
	CChars		mszError;
	bool		mbUseUserError;

public:
	void	Init(bool bUseUserError);
	void	Kill(void);

	void	SetError(char* szError);
	bool	HasError(void);
	char*	GetError(void);

	bool	UseUserError(void);
};


#endif // __CALCULATOR_ERROR_H__

