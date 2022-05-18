#ifndef __JAVA_NULL_H__
#define __JAVA_NULL_H__
#include "BaseLib/Chars.h"
#include "JavaTokenLiteral.h"


class CJavaTokenNull : public CJavaTokenLiteral
{
CONSTRUCTABLE(CJavaTokenNull);
public:
	void 	Init(STextPosition* psPosition);
	void 	Kill(void);

	void	Print(CChars* pszDest);
	BOOL	IsNull(void);
};


#endif // !__JAVA_NULL_H__

