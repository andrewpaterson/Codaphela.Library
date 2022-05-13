#ifndef __JAVA_TOKEN_MEMORY_H__
#define __JAVA_TOKEN_MEMORY_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaComment.h"
#include "JavaKeyword.h"
#include "JavaIdentifier.h"
#include "JavaAnnotation.h"
#include "JavaOperator.h"
#include "JavaSeparator.h"
#include "JavaScope.h"
#include "JavaAmbiguous.h"
#include "JavaNull.h"
#include "JavaBoolean.h"
#include "JavaCharacter.h"
#include "JavaInteger.h"
#include "JavaFloat.h"
#include "JavaString.h"


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
	CJavaAnnotation*	CreateAnnotation(char* szAnnotation, int iLength);
	CJavaKeyword*		CreateKeyword(CJavaKeywordDefinition* pcKeyword);
	CJavaOperator*		CreateOperator(CJavaOperatorDefinition* pcOperator);
	CJavaSeparator*		CreateSeparator(CJavaSeparatorDefinition* pcSeparator);
	CJavaScope*			CreateScope(CJavaScopeDefinition* pcGeneric);
	CJavaAmbiguous*		CreateAmbiguous(CJavaAmbiguousDefinition* pcAmbiguous);
	CJavaNull*			CreateNull(void);
	CJavaBoolean*		CreateBoolean(BOOL bValue);
	CJavaCharacter*		CreateCharacter(char c);
	CJavaCharacter*		CreateCharacter(char16 iChar);
	CJavaInteger*		CreateInteger(int32 iValue);
	CJavaInteger*		CreateInteger(int64 iValue);
	CJavaFloat*			CreateFloat(float32 fValue);
	CJavaFloat*			CreateFloat(float64 fValue);
	CJavaString*		CreateString(char* szString, int iLength);
	CJavaString*		CreateString(char16* szString, int iLength);
};


#endif // !__JAVA_TOKEN_MEMORY_H__

	