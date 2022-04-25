#ifndef __JAVA_CHARACTER_H__
#define __JAVA_CHARACTER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/PrimitiveTypes.h"
#include "JavaLiteral.h"


class CJavaCharacter : public CJavaLiteral
{
CONSTRUCTABLE(CJavaCharacter);
protected:
	char16	mc;

public:
	void 	Init(char c);
	void 	Init(char16 c);
	void 	Kill(void);

	void	Print(CChars* pszDest);
	BOOL	IsCharacter(void);
};


#endif // !__JAVA_CHARACTER_H__

