#ifndef __JAVA_AMBIGUOUS_H__
#define __JAVA_AMBIGUOUS_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


enum EJavaAmbiguous
{
	JA_AngleBracketLeft,
	JA_AngleBracketRight,
	JA_QuestionMark,
	JA_Asterisk,

	JA_Unknown = -1
};


class CJavaAmbiguousDefinition;
class CJavaAmbiguous : public CJavaToken
{
CONSTRUCTABLE(CJavaAmbiguous);
protected:
	CJavaAmbiguousDefinition*	mpcDefinition;

public:
	void 	Init(CJavaAmbiguousDefinition* pcDefinition);
	void 	Kill(void);

	void	Print(CChars* pszDest);
	char*	GetType(void);
};


class CJavaAmbiguousDefinition
{
protected:
	EJavaAmbiguous	meAmbiguous;
	CChars			mszName;

public:
	void 	Init(EJavaAmbiguous eAmbiguous, char* szName);
	void 	Kill(void);

	char*	GetName(void);
};


#endif // !__JAVA_AMBIGUOUS_H__

