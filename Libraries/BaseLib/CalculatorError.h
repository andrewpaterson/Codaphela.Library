#ifndef __CALCULATOR_ERROR_H__
#define __CALCULATOR_ERROR_H__
#include "Chars.h"


class CCalculatorError
{
protected:
	CChars		mszError;
	bool		mbUseUserError;

public:
	void		Init(bool bUseUserError);
	void		Kill(void);

	void		SetError(const char* szError);
	bool		HasError(void) const;
	const char*	GetError(void) const;

	bool		UseUserError(void);
};


#endif // __CALCULATOR_ERROR_H__

