#ifndef __TEST_FUNCTION_CALLER_H__
#define __TEST_FUNCTION_CALLER_H__
#include "BaseLib/Chars.h"


class CTestFunctionCaller
{
public:
	CChars	msz;

	void Init(void);
	void Kill(void);

	int PrintIt(const char* szMore, int iNumber);
};


#endif // __TEST_FUNCTION_CALLER_H__

