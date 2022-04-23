#ifndef __JAVA_AMBIGUOUS_H__
#define __JAVA_AMBIGUOUS_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


enum EJavaAmbiguous
{
	JA_AngleBracketLeft,
	JA_AngleBracketRight,
	JA_QuestionMark,

	JA_Unknown = -1
};


class CJavaAmbiguous : public CJavaToken
{
CONSTRUCTABLE(CJavaAmbiguous);
protected:
	EJavaAmbiguous	meAmbiguous;

public:
	void 	Init(EJavaAmbiguous eAmbiguous);
	void 	Kill(void);
};


class CJavaAmbiguousDefinition
{
protected:
	EJavaAmbiguous	meAmbiguous;
	CChars			mszName;

public:
	void 	Init(EJavaAmbiguous eAmbiguous, char* szName);
	void 	Kill(void);
};


#endif // !__JAVA_AMBIGUOUS_H__

