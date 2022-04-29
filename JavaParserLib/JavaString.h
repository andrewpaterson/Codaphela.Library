#ifndef __JAVA_STRING_H__
#define __JAVA_STRING_H__
#include "BaseLib/Chars.h"
#include "BaseLib/PrimitiveTypes.h"
#include "JavaLiteral.h"


enum EJavaStringType
{
	JST_string8,
	JST_string16,

	JST_Unknown = -1
};


class CJavaString : public CJavaLiteral
{
CONSTRUCTABLE(CJavaString);
protected:
	void*				msz;
	EJavaStringType		meType;
	int					miLength;

public:
	void 	Init(char* sz, int iLength);
	void 	Init(char16* sz, int iLength);
	void 	Kill(void);

	char*	GetType(void);

	void	Print(CChars* pszDest);
	BOOL	IsString(void);
};


#endif // !__JAVA_STRING_H__

