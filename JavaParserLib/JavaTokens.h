#ifndef __JAVA_TOKENS_H__
#define __JAVA_TOKENS_H__
#include "BaseLib/Constructable.h"
#include "JavaTokenMemory.h"


class CJavaTokens
{
CONSTRUCTABLE(CJavaTokens);
protected:
	CJavaTokenMemory	mcMemory;

public:
	void				Init(void);
	void				Kill(void);

	CJavaComment*		AddComment(char* szComment, int iLength);
	CJavaKeyword*		AddKeyword(EJavaKeyword eKeyword);
	CJavaIdentifier*	AddIdentifier(char* szIdentifier, int iLength);
	CJavaOperator*		AddOperator(EJavaOperatorType eType, EJavaOperator eOperator);
	CJavaSeparator*		AddSeparator(EJavaSeparator eSeparator);
	CJavaAmbiguous*		AddAmbiguous(EJavaAmbiguous eAmbiguous);
	CJavaGeneric*		AddGeneric(EJavaGeneric eGeneric);
};


#endif // !__JAVA_TOKENS_H__

