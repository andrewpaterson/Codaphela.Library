#ifndef __JAVA_STRING_H__
#define __JAVA_STRING_H__
#include "BaseLib/Chars.h"
#include "BaseLib/PrimitiveTypes.h"
#include "JavaTokenLiteral.h"


enum EJavaTokenStringType
{
	JST_string8,
	JST_string16,

	JST_Unknown = -1
};


class CJavaTokenString : public CJavaTokenLiteral
{
CONSTRUCTABLE(CJavaTokenString);
protected:
	void*					msz;
	EJavaTokenStringType	meType;
	int						miLength;

public:
	void 	Init(STextPosition* psPosition, char* sz, int iLength);
	void 	Init(STextPosition* psPosition, char16* sz, int iLength);
	void 	Kill(void);

	char*	GetType(void);
	BOOL	Is(char* szString);

	void	Print(CChars* pszDest);
	BOOL	IsString(void);
};


#endif // !__JAVA_STRING_H__

