#ifndef __JAVA_SEPARATOR_H__
#define __JAVA_SEPARATOR_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


enum EJavaTokenSeparator
{
	JS_Semicolon,
	JS_Comma,
	JS_Dot,
	JS_RoundBracketLeft,
	JS_RoundBracketRight,
	JS_CurlyBracketLeft,
	JS_CurlyBracketRight,
	JS_SquareBracketLeft,
	JS_SquareBracketRight,

	JS_Unknown = -1
};


class CJavaTokenSeparatorDefinition;
class CJavaTokenSeparator : public CJavaToken
{
CONSTRUCTABLE(CJavaTokenSeparator);
protected:
	CJavaTokenSeparatorDefinition*	mpcSeparator;

public:
	void 	Init(STextPosition* psPosition, CJavaTokenSeparatorDefinition* pcSeparator);
	void 	Kill(void);

	void	Print(CChars* pszDest);
	char*	GetType(void);

	bool	IsSeparator(void);
	bool	Is(EJavaTokenSeparator eSeparator);
};


class CJavaTokenSeparatorDefinition
{
protected:
	EJavaTokenSeparator	meSeparator;
	CChars			mszName;

public:
	void 			Init(EJavaTokenSeparator eSeparator, char* szName);
	void 			Kill(void);

	EJavaTokenSeparator	Get(void);
	char*			GetName(void);
};


#endif // !__JAVA_SEPARATOR_H__

