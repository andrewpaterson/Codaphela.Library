#ifndef __JAVA_CHARACTER_H__
#define __JAVA_CHARACTER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/PrimitiveTypes.h"
#include "JavaTokenLiteral.h"


enum EJavaTokenCharacterType
{
	JCT_char8,
	JCT_char16,

	JCT_Unknown = -1
};


class CJavaTokenCharacter : public CJavaTokenLiteral
{
CONSTRUCTABLE(CJavaTokenCharacter);
protected:
	char16				mc;
	EJavaTokenCharacterType	meType;

public:
	void 	Init(STextPosition* psPosition, char c);
	void 	Init(STextPosition* psPosition, char16 c);
	void 	Kill(void);

	char*	GetType(void);
	bool	Is(char8 c);
	bool	Is(char16 c);

	void	Print(CChars* pszDest);
	bool	IsCharacter(void);
};


#endif // !__JAVA_CHARACTER_H__

