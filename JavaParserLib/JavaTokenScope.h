#ifndef __JAVA_GENERIC_H__
#define __JAVA_GENERIC_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


enum EJavaTokenScope
{
	JG_AngleBracketLeft,
	JG_AngleBracketRight,
	JG_QuestionMark,
	JG_Asterisk,

	JG_Unknown = -1
};


class CJavaTokenScopeDefinition;
class CJavaTokenScope : public CJavaToken
{
CONSTRUCTABLE(CJavaTokenScope);
protected:
	CJavaTokenScopeDefinition*	mpcScope;

public:
	void 	Init(CJavaTokenScopeDefinition* pcGeneric);
	void 	Kill(void);

	BOOL	IsScope(void);
	void	Print(CChars* pszDest);
	char*	GetType(void);

	BOOL	Is(EJavaTokenScope eGeneric);
};


class CJavaTokenScopeDefinition
{
protected:
	EJavaTokenScope	meScope;
	CChars		mszName;

public:
	void 			Init(EJavaTokenScope eGeneric, char* szName);
	void 			Kill(void);

	char*			GetName(void);
	EJavaTokenScope	Get(void);
};


#endif // !__JAVA_GENERIC_H__

