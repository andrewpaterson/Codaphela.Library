#ifndef __JAVA_TOKEN_MEMORY_H__
#define __JAVA_TOKEN_MEMORY_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/TextPosition.h"
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
	void					Init(void);
	void					Kill(void);

	CJavaTokenComment*		CreateComment(STextPosition* psPosition, char* szComment, int iLength);
	CJavaTokenIdentifier*	CreateIdentifier(STextPosition* psPosition, char* szIdentifier, int iLength);
	CJavaTokenAnnotation*	CreateAnnotation(STextPosition* psPosition, char* szAnnotation, int iLength);
	CJavaTokenKeyword*		CreateKeyword(STextPosition* psPosition, CJavaTokenKeywordDefinition* pcKeyword);
	CJavaTokenOperator*		CreateOperator(STextPosition* psPosition, CJavaTokenOperatorDefinition* pcOperator);
	CJavaTokenSeparator*	CreateSeparator(STextPosition* psPosition, CJavaTokenSeparatorDefinition* pcSeparator);
	CJavaTokenScope*		CreateScope(STextPosition* psPosition, CJavaTokenScopeDefinition* pcGeneric);
	CCJavaTokenAmbiguous*	CreateAmbiguous(STextPosition* psPosition, CCJavaTokenAmbiguousDefinition* pcAmbiguous);
	CJavaTokenNull*			CreateNull(STextPosition* psPosition);
	CJavaTokenBoolean*		CreateBoolean(STextPosition* psPosition, bool bValue);
	CJavaTokenCharacter*	CreateCharacter(STextPosition* psPosition, char c);
	CJavaTokenCharacter*	CreateCharacter(STextPosition* psPosition, char16 iChar);
	CJavaTokenInteger*		CreateInteger(STextPosition* psPosition, int32 iValue);
	CJavaTokenInteger*		CreateInteger(STextPosition* psPosition, int64 iValue);
	CJavaTokenFloat*		CreateFloat(STextPosition* psPosition, float32 fValue);
	CJavaTokenFloat*		CreateFloat(STextPosition* psPosition, float64 fValue);
	CJavaTokenString*		CreateString(STextPosition* psPosition, char* szString, int iLength);
	CJavaTokenString*		CreateString(STextPosition* psPosition, char16* szString, int iLength);
};


#endif // !__JAVA_TOKEN_MEMORY_H__

	