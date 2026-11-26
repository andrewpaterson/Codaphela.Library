#ifndef __JAVA_BOOLEAN_H__
#define __JAVA_BOOLEAN_H__
#include "BaseLib/Chars.h"
#include "JavaTokenLiteral.h"


class CJavaTokenBoolean : public CJavaTokenLiteral
{
NAMED_CLASS(CJavaTokenBoolean);
protected:
	bool	mbValue;

public:
	void	Init(STextPosition* psPosition, bool bValue);
	void	Kill(void);

	char*	GetType(void);
	bool	Is(bool b);

	void	Print(CChars* pszDest);
	bool	IsBoolean(void);
};


#endif // !__JAVA_BOOLEAN_H__

