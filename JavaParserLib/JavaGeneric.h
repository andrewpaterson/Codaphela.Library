#ifndef __JAVA_GENERIC_H__
#define __JAVA_GENERIC_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


enum EJavaGeneric
{
	JG_AngleBracketLeft,
	JG_AngleBracketRight,
	JG_QuestionMark,

	JG_Unknown = -1
};


class CJavaGenericDefinition;
class CJavaGeneric : public CJavaToken
{
CONSTRUCTABLE(CJavaGeneric);
protected:
	CJavaGenericDefinition*	mpcGeneric;

public:
	void 	Init(CJavaGenericDefinition* pcGeneric);
	void 	Kill(void);

	void	Print(CChars* pszDest);
	char*	GetType(void);
};


class CJavaGenericDefinition
{
protected:
	EJavaGeneric	meGeneric;
	CChars			mszName;

public:
	void 	Init(EJavaGeneric eGeneric, char* szName);
	void 	Kill(void);

	char*	GetName(void);
};


#endif // !__JAVA_GENERIC_H__

