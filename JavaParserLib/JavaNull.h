#ifndef __JAVA_NULL_H__
#define __JAVA_NULL_H__
#include "BaseLib/Chars.h"
#include "JavaLiteral.h"


class CJavaNull : public CJavaLiteral
{
CONSTRUCTABLE(CJavaNull);
public:
	void 	Init(void);
	void 	Kill(void);

	void	Print(CChars* pszDest);
	BOOL	IsNull(void);
};


#endif // !__JAVA_NULL_H__

