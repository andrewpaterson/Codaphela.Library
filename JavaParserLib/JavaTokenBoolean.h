#ifndef __JAVA_BOOLEAN_H__
#define __JAVA_BOOLEAN_H__
#include "BaseLib/Chars.h"
#include "JavaTokenLiteral.h"


class CJavaTokenBoolean : public CJavaTokenLiteral
{
CONSTRUCTABLE(CJavaTokenBoolean);
protected:
	BOOL	mbValue;

public:
	void 	Init(BOOL bValue);
	void 	Kill(void);

	char*	GetType(void);
	BOOL	Is(BOOL b);

	void	Print(CChars* pszDest);
	BOOL	IsBoolean(void);
};


#endif // !__JAVA_BOOLEAN_H__

