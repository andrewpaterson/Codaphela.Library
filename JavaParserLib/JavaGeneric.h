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


class CJavaGeneric : public CJavaToken
{
CONSTRUCTABLE(CJavaGeneric);
protected:
	EJavaGeneric	meGeneric;

public:
	void 	Init(EJavaGeneric eGeneric);
	void 	Kill(void);
};


class CJavaGenericDefinition
{
protected:
	EJavaGeneric	meGeneric;
	CChars			mszName;

public:
	void 	Init(EJavaGeneric eGeneric, char* szName);
	void 	Kill(void);
};


#endif // !__JAVA_GENERIC_H__

