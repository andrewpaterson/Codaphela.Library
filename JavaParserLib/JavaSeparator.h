#ifndef __JAVA_SEPARATOR_H__
#define __JAVA_SEPARATOR_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


enum EJavaSeparator
{
	JS_Semicolon,
	JS_Comma,
	JS_Dot,
	JS_RoundBracketLeft,
	JS_RoundBracketRight,
	JS_CurlyBracketLeft,
	JS_CurlyBracketRight,
	JS_SquareBracketLeft,
	JS_AquareBracketRight,

	JS_Unknown = -1
};


class CJavaSeparator : public CJavaToken
{
	CONSTRUCTABLE(CJavaSeparator);
protected:
	EJavaSeparator	meSeparator;

public:
	void 	Init(EJavaSeparator eSeparator);
	void 	Kill(void);
};


class CJavaSeparatorDefinition
{
protected:
	EJavaSeparator	meSeparator;
	CChars			mszName;

public:
	void 	Init(EJavaSeparator eSeparator, char* szName);
	void 	Kill(void);
};


#endif // !__JAVA_SEPARATOR_H__

