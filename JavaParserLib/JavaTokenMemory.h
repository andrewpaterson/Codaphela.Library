#ifndef __JAVA_TOKEN_MEMORY_H__
#define __JAVA_TOKEN_MEMORY_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaComment.h"
#include "JavaKeyword.h"
#include "JavaIdentifier.h"
#include "JavaOperator.h"
#include "JavaSeparator.h"
#include "JavaGeneric.h"
#include "JavaAmbiguous.h"


typedef CArrayTemplatePtr<CJavaToken>	CTokenPtrArray;


class CJavaTokenMemory
{
CONSTRUCTABLE(CJavaTokenMemory);
protected:
	CMemoryStackExtended	mcStack;
	CTokenPtrArray			mapcTokens;

public:
	void				Init(void);
	void				Kill(void);

	CJavaComment*		CreateComment(char* szComment, int iLength);
	CJavaIdentifier*	CreateIdentifier(char* szIdentifier, int iLength);
	CJavaKeyword*		CreateKeyword(CJavaKeywordDefinition* pcKeyword);
	CJavaOperator*		CreateOperator(CJavaOperatorDefinition* pcOperator);
	CJavaSeparator*		CreateSeparator(CJavaSeparatorDefinition* pcSeparator);
	CJavaGeneric*		CreateGeneric(CJavaGenericDefinition* pcGeneric);
	CJavaAmbiguous*		CreateAmbiguous(CJavaAmbiguousDefinition* pcAmbiguous);
};


#endif // !__JAVA_TOKEN_MEMORY_H__

	