#ifndef __JAVA_GENERIC_H__
#define __JAVA_GENERIC_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


enum EJavaScope
{
	JG_AngleBracketLeft,
	JG_AngleBracketRight,
	JG_QuestionMark,
	JG_Asterisk,

	JG_Unknown = -1
};


class CJavaScopeDefinition;
class CJavaScope : public CJavaToken
{
CONSTRUCTABLE(CJavaScope);
protected:
	CJavaScopeDefinition*	mpcScope;

public:
	void 	Init(CJavaScopeDefinition* pcGeneric);
	void 	Kill(void);

	BOOL	IsScope(void);
	void	Print(CChars* pszDest);
	char*	GetType(void);

	BOOL	Is(EJavaScope eGeneric);
};


class CJavaScopeDefinition
{
protected:
	EJavaScope	meScope;
	CChars		mszName;

public:
	void 			Init(EJavaScope eGeneric, char* szName);
	void 			Kill(void);

	char*			GetName(void);
	EJavaScope	Get(void);
};


#endif // !__JAVA_GENERIC_H__

