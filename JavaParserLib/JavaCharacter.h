#ifndef __JAVA_CHARACTER_H__
#define __JAVA_CHARACTER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/PrimitiveTypes.h"
#include "JavaLiteral.h"


enum EJavaCharacterType
{
	JCT_char8,
	JCT_char16,

	JCT_Unknown = -1
};


class CJavaCharacter : public CJavaLiteral
{
CONSTRUCTABLE(CJavaCharacter);
protected:
	char16				mc;
	EJavaCharacterType	meType;

public:
	void 	Init(char c);
	void 	Init(char16 c);
	void 	Kill(void);

	char*	GetType(void);

	void	Print(CChars* pszDest);
	BOOL	IsCharacter(void);
};


#endif // !__JAVA_CHARACTER_H__

