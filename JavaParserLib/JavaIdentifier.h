#ifndef __JAVA_IDENTIFIER_H__
#define __JAVA_IDENTIFIER_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


class CJavaIdentifier : public CJavaToken
{
CONSTRUCTABLE(CJavaIdentifier);
protected:
	char*	mszIdentifier;
	int		miLength;

public:
	void 	Init(char* szIdentifier, int iLength);
	void 	Kill(void);

	void	Print(CChars* pszDest);
	char*	GetType(void);

	BOOL	IsIdentifier(void);
};


#endif // !__JAVA_IDENTIFIER_H__

