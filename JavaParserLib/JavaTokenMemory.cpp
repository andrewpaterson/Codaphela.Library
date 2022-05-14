#include "BaseLib/Numbers.h"
#include "JavaTokenMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenMemory::Init(void)
{
	mcStack.Init(4 KB);
	mapcTokens.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenMemory::Kill(void)
{
	CJavaToken*		pcToken;
	int				iNumTokens;
	int				i;

	iNumTokens = mapcTokens.NumElements();
	for (i = 0; i < iNumTokens; i++)
	{
		pcToken = mapcTokens.GetPtr(i);
		pcToken->Kill();
	}

	mapcTokens.Kill();
	mcStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenComment* CJavaTokenMemory::CreateComment(char* szComment, int iLength)
{
	CJavaTokenComment*	pcToken;
	char*			szDest;

	pcToken = (CJavaTokenComment*)mcStack.Add(sizeof(CJavaTokenComment) + iLength + 1);
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenComment;
		szDest = (char*)RemapSinglePointer(pcToken, sizeof(CJavaTokenComment));
		memcpy(szDest, szComment, iLength);
		szDest[iLength] = '\0';
		pcToken->Init(szDest, iLength);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenKeyword* CJavaTokenMemory::CreateKeyword(CJavaTokenKeywordDefinition* pcKeyword)
{
	CJavaTokenKeyword*	pcToken;

	pcToken = (CJavaTokenKeyword*)mcStack.Add(sizeof(CJavaTokenKeyword));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenKeyword;
		pcToken->Init(pcKeyword);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenIdentifier* CJavaTokenMemory::CreateIdentifier(char* szIdentifier, int iLength)
{
	CJavaTokenIdentifier*	pcToken;
	char*				szDest;

	pcToken = (CJavaTokenIdentifier*)mcStack.Add(sizeof(CJavaTokenIdentifier) + iLength + 1);
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenIdentifier;
		szDest = (char*)RemapSinglePointer(pcToken, sizeof(CJavaTokenIdentifier));
		memcpy(szDest, szIdentifier, iLength);
		szDest[iLength] = '\0';
		pcToken->Init(szDest, iLength);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenAnnotation* CJavaTokenMemory::CreateAnnotation(char* szAnnotation, int iLength)
{
	CJavaTokenAnnotation*	pcToken;
	char*				szDest;

	pcToken = (CJavaTokenAnnotation*)mcStack.Add(sizeof(CJavaTokenAnnotation) + iLength + 1);
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenAnnotation;
		szDest = (char*)RemapSinglePointer(pcToken, sizeof(CJavaTokenAnnotation));
		memcpy(szDest, szAnnotation, iLength);
		szDest[iLength] = '\0';
		pcToken->Init(szDest, iLength);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenOperator* CJavaTokenMemory::CreateOperator(CJavaTokenOperatorDefinition* pcOperator)
{
	CJavaTokenOperator* pcToken;

	pcToken = (CJavaTokenOperator*)mcStack.Add(sizeof(CJavaTokenOperator));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenOperator;
		pcToken->Init(pcOperator);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenSeparator* CJavaTokenMemory::CreateSeparator(CJavaTokenSeparatorDefinition* pcSeparator)
{
	CJavaTokenSeparator* pcToken;

	pcToken = (CJavaTokenSeparator*)mcStack.Add(sizeof(CJavaTokenSeparator));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenSeparator;
		pcToken->Init(pcSeparator);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenScope* CJavaTokenMemory::CreateScope(CJavaTokenScopeDefinition* pcGeneric)
{
	CJavaTokenScope* pcToken;

	pcToken = (CJavaTokenScope*)mcStack.Add(sizeof(CJavaTokenScope));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenScope;
		pcToken->Init(pcGeneric);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCJavaTokenAmbiguous* CJavaTokenMemory::CreateAmbiguous(CCJavaTokenAmbiguousDefinition* pcAmbiguous)
{
	CCJavaTokenAmbiguous* pcToken;

	pcToken = (CCJavaTokenAmbiguous*)mcStack.Add(sizeof(CCJavaTokenAmbiguous));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CCJavaTokenAmbiguous;
		pcToken->Init(pcAmbiguous);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenNull* CJavaTokenMemory::CreateNull(void)
{
	CJavaTokenNull*	pcToken;

	pcToken = (CJavaTokenNull*)mcStack.Add(sizeof(CJavaTokenNull));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenNull;
		pcToken->Init();
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenBoolean* CJavaTokenMemory::CreateBoolean(BOOL bValue)
{
	CJavaTokenBoolean* pcToken;

	pcToken = (CJavaTokenBoolean*)mcStack.Add(sizeof(CJavaTokenBoolean));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenBoolean;
		pcToken->Init(bValue);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenCharacter* CJavaTokenMemory::CreateCharacter(char c)
{
	CJavaTokenCharacter* pcToken;

	pcToken = (CJavaTokenCharacter*)mcStack.Add(sizeof(CJavaTokenCharacter));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenCharacter;
		pcToken->Init(c);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenCharacter* CJavaTokenMemory::CreateCharacter(char16 c)
{
	CJavaTokenCharacter* pcToken;

	pcToken = (CJavaTokenCharacter*)mcStack.Add(sizeof(CJavaTokenCharacter));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenCharacter;
		pcToken->Init(c);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenInteger* CJavaTokenMemory::CreateInteger(int32 iValue)
{
	CJavaTokenInteger* pcToken;

	pcToken = (CJavaTokenInteger*)mcStack.Add(sizeof(CJavaTokenInteger));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenInteger;
		pcToken->Init(iValue);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenInteger* CJavaTokenMemory::CreateInteger(int64 iValue)
{
	CJavaTokenInteger* pcToken;

	pcToken = (CJavaTokenInteger*)mcStack.Add(sizeof(CJavaTokenInteger));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenInteger;
		pcToken->Init(iValue);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenFloat* CJavaTokenMemory::CreateFloat(float32 fValue)
{
	CJavaTokenFloat* pcToken;

	pcToken = (CJavaTokenFloat*)mcStack.Add(sizeof(CJavaTokenFloat));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenFloat;
		pcToken->Init(fValue);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenFloat* CJavaTokenMemory::CreateFloat(float64 fValue)
{
	CJavaTokenFloat* pcToken;

	pcToken = (CJavaTokenFloat*)mcStack.Add(sizeof(CJavaTokenFloat));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenFloat;
		pcToken->Init(fValue);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenString* CJavaTokenMemory::CreateString(char* szString, int iLength)
{
	CJavaTokenString*	pcToken;
	char*			szDest;

	pcToken = (CJavaTokenString*)mcStack.Add(sizeof(CJavaTokenString) + iLength + 1);
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenString;
		szDest = (char*)RemapSinglePointer(pcToken, sizeof(CJavaTokenString));
		memcpy(szDest, szString, iLength);
		szDest[iLength] = '\0';
		pcToken->Init(szDest, iLength);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenString* CJavaTokenMemory::CreateString(char16* szString, int iLength)
{
	CJavaTokenString*	pcToken;
	char16*			szDest;

	pcToken = (CJavaTokenString*)mcStack.Add(sizeof(CJavaTokenString) + ((iLength + 1) * sizeof(char16)));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenString;
		szDest = (char16*)RemapSinglePointer(pcToken, sizeof(CJavaTokenString));
		memcpy(szDest, szString, iLength * sizeof(char16));
		szDest[iLength] = '\0';
		pcToken->Init(szDest, iLength);
	}

	return pcToken;
}

