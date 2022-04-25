#ifndef __JAVA_BOOLEAN_H__
#define __JAVA_BOOLEAN_H__
#include "BaseLib/Chars.h"
#include "JavaLiteral.h"


class CJavaBoolean : public CJavaLiteral
{
CONSTRUCTABLE(CJavaBoolean);
protected:
	BOOL	mbValue;

public:
	void 	Init(BOOL bValue);
	void 	Kill(void);

	void	Print(CChars* pszDest);
	BOOL	IsBoolean(void);
};


#endif // !__JAVA_BOOLEAN_H__
