#ifndef __JAVA_TOKEN_MEMORY_H__
#define __JAVA_TOKEN_MEMORY_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/MemoryStackExtended.h"
#include "JavaComment.h"
#include "JavaKeyword.h"
#include "JavaIdentifier.h"
#include "JavaOperator.h"
#include "JavaSeparator.h"
#include "JavaGeneric.h"
#include "JavaAmbiguous.h"


class CJavaTokenMemory
{
CONSTRUCTABLE(CJavaTokenMemory);
protected:
	CMemoryStackExtended	mcStack;

public:
	void				Init(void);
	void				Kill(void);

	CJavaComment*		CreateComment(char* szComment, int iLength);
	CJavaKeyword*		CreateKeyword(EJavaKeyword eKeyword);
	CJavaIdentifier*	CreateIdentifier(char* szIdentifier, int iLength);
	CJavaOperator*		CreateOperator(EJavaOperatorType eType, EJavaOperator eOperator);
	CJavaSeparator*		CreateSeparator(EJavaSeparator eSeparator);
	CJavaGeneric*		CreateGeneric(EJavaGeneric eGeneric);
	CJavaAmbiguous*		CreateAmbiguous(EJavaAmbiguous eAmbiguous);
};


#endif // !__JAVA_TOKEN_MEMORY_H__

	