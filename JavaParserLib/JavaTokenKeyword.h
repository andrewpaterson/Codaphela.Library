#ifndef __JAVA_KEYWORD_H__
#define __JAVA_KEYWORD_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


enum EJavaTokenKeyword
{
	JK_abstract,
	JK_assert,
	JK_boolean,	
	JK_break,
	JK_byte,
	JK_case,	
	JK_catch,	
	JK_char,
	JK_class,	
	JK_const,	
	JK_continue,
	JK_default,
	JK_do,
	JK_double,
	JK_else,
	JK_enum,
	JK_extends,
	JK_final,
	JK_finally,
	JK_float,
	JK_for,
	JK_goto,
	JK_if,
	JK_implements,
	JK_import,
	JK_instanceof,
	JK_int,
	JK_interface,
	JK_long,
	JK_native,
	JK_new,
	JK_package,
	JK_private,	
	JK_protected,
	JK_public,
	JK_return,
	JK_short,
	JK_static,	
	JK_strictfp,
	JK_super,
	JK_switch,	
	JK_synchronized,
	JK_this,
	JK_throw,
	JK_throws,	
	JK_transient,
	JK_try,
	JK_void,
	JK_volatile,
	JK_while,
	JK_Unknown = -1
};

class CJavaTokenKeywordDefinition;
class CJavaTokenKeyword : public CJavaToken
{
CONSTRUCTABLE(CJavaTokenKeyword);
protected:
	CJavaTokenKeywordDefinition*	mpcKeyword;

public:
	void 	Init(CJavaTokenKeywordDefinition* pcKeyword);
	void 	Kill(void);

	void	Print(CChars* pszDest);
	char*	GetType(void);
	BOOL	IsKeyword(void);

	BOOL	Is(EJavaTokenKeyword eKeyword);
};


class CJavaTokenKeywordDefinition
{
protected:
	EJavaTokenKeyword	meKeyword;
	CChars			mszName;

public:
	void 			Init(EJavaTokenKeyword eKeyword, char* szName);
	void 			Kill(void);

	char*			GetName(void);
	EJavaTokenKeyword	Get(void);
};


#endif // !__JAVA_KEYWORD_H__

