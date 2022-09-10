#ifndef __JAVA_IDENTIFIER_H__
#define __JAVA_IDENTIFIER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplateEmbeddedPtr.h"
#include "JavaToken.h"


class CJavaTokenIdentifier : public CJavaToken
{
CONSTRUCTABLE(CJavaTokenIdentifier);
protected:
	char*	mszIdentifier;
	int		miLength;

public:
	void 	Init(STextPosition* psPosition, char* szIdentifier, int iLength);
	void 	Kill(void);

	void	Print(CChars* pszDest);
	char*	GetType(void);

	bool	IsIdentifier(void);
	bool	Is(char* szIdentifier);
	char*	GetIdentifer(void);
};


typedef CArrayTemplateEmbeddedPtr<CJavaTokenIdentifier, 6> CIdentifierArray;


#endif // !__JAVA_IDENTIFIER_H__

