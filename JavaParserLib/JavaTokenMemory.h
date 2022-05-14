#ifndef __JAVA_TOKEN_MEMORY_H__
#define __JAVA_TOKEN_MEMORY_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaTokenComment.h"
#include "JavaTokenKeyword.h"
#include "JavaTokenIdentifier.h"
#include "JavaTokenAnnotation.h"
#include "JavaTokenOperator.h"
#include "JavaTokenSeparator.h"
#include "JavaTokenScope.h"
#include "JavaTokenAmbiguous.h"
#include "JavaTokenNull.h"
#include "JavaTokenBoolean.h"
#include "JavaTokenCharacter.h"
#include "JavaTokenInteger.h"
#include "JavaTokenFloat.h"
#include "JavaTokenString.h"


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

	CJavaTokenComment*		CreateComment(char* szComment, int iLength);
	CJavaTokenIdentifier*	CreateIdentifier(char* szIdentifier, int iLength);
	CJavaTokenAnnotation*	CreateAnnotation(char* szAnnotation, int iLength);
	CJavaTokenKeyword*		CreateKeyword(CJavaTokenKeywordDefinition* pcKeyword);
	CJavaTokenOperator*		CreateOperator(CJavaTokenOperatorDefinition* pcOperator);
	CJavaTokenSeparator*		CreateSeparator(CJavaTokenSeparatorDefinition* pcSeparator);
	CJavaTokenScope*			CreateScope(CJavaTokenScopeDefinition* pcGeneric);
	CCJavaTokenAmbiguous*		CreateAmbiguous(CCJavaTokenAmbiguousDefinition* pcAmbiguous);
	CJavaTokenNull*			CreateNull(void);
	CJavaTokenBoolean*		CreateBoolean(BOOL bValue);
	CJavaTokenCharacter*		CreateCharacter(char c);
	CJavaTokenCharacter*		CreateCharacter(char16 iChar);
	CJavaTokenInteger*		CreateInteger(int32 iValue);
	CJavaTokenInteger*		CreateInteger(int64 iValue);
	CJavaTokenFloat*			CreateFloat(float32 fValue);
	CJavaTokenFloat*			CreateFloat(float64 fValue);
	CJavaTokenString*		CreateString(char* szString, int iLength);
	CJavaTokenString*		CreateString(char16* szString, int iLength);
};


#endif // !__JAVA_TOKEN_MEMORY_H__

	