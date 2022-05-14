#ifndef __JAVA_AMBIGUOUS_H__
#define __JAVA_AMBIGUOUS_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


enum ECJavaTokenAmbiguous
{
	JA_AngleBracketLeft,
	JA_AngleBracketRight,
	JA_QuestionMark,
	JA_Asterisk,

	JA_Unknown = -1
};


class CCJavaTokenAmbiguousDefinition;
class CCJavaTokenAmbiguous : public CJavaToken
{
CONSTRUCTABLE(CCJavaTokenAmbiguous);
protected:
	CCJavaTokenAmbiguousDefinition*	mpcDefinition;

public:
	void 	Init(CCJavaTokenAmbiguousDefinition* pcDefinition);
	void 	Kill(void);

	BOOL	IsAmbiguous(void);
	void	Print(CChars* pszDest);
	char*	GetType(void);

	BOOL	Is(ECJavaTokenAmbiguous eAmbiguous);
};


class CCJavaTokenAmbiguousDefinition
{
protected:
	ECJavaTokenAmbiguous	meAmbiguous;
	CChars			mszName;

public:
	void 			Init(ECJavaTokenAmbiguous eAmbiguous, char* szName);
	void 			Kill(void);

	char*			GetName(void);
	ECJavaTokenAmbiguous	Get(void);
};


#endif // !__JAVA_AMBIGUOUS_H__

