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
CJavaTokenKeyword* CJavaTokenMemory::CreateKeyword(STextPosition* psPosition, CJavaTokenKeywordDefinition* pcKeyword)
{
	CJavaTokenKeyword*	pcToken;

	pcToken = (CJavaTokenKeyword*)mcStack.Add(sizeof(CJavaTokenKeyword));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenKeyword;
		pcToken->Init(psPosition, pcKeyword);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenIdentifier* CJavaTokenMemory::CreateIdentifier(STextPosition* psPosition, char* szIdentifier, int iLength)
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
		pcToken->Init(psPosition, szDest, iLength);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenAnnotation* CJavaTokenMemory::CreateAnnotation(STextPosition* psPosition, char* szAnnotation, int iLength)
{
	CJavaTokenAnnotation*	pcToken;
	char*					szDest;

	pcToken = (CJavaTokenAnnotation*)mcStack.Add(sizeof(CJavaTokenAnnotation) + iLength + 1);
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenAnnotation;
		szDest = (char*)RemapSinglePointer(pcToken, sizeof(CJavaTokenAnnotation));
		memcpy(szDest, szAnnotation, iLength);
		szDest[iLength] = '\0';
		pcToken->Init(psPosition, szDest, iLength);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenOperator* CJavaTokenMemory::CreateOperator(STextPosition* psPosition, CJavaTokenOperatorDefinition* pcOperator)
{
	CJavaTokenOperator* pcToken;

	pcToken = (CJavaTokenOperator*)mcStack.Add(sizeof(CJavaTokenOperator));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenOperator;
		pcToken->Init(psPosition, pcOperator);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenSeparator* CJavaTokenMemory::CreateSeparator(STextPosition* psPosition, CJavaTokenSeparatorDefinition* pcSeparator)
{
	CJavaTokenSeparator* pcToken;

	pcToken = (CJavaTokenSeparator*)mcStack.Add(sizeof(CJavaTokenSeparator));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenSeparator;
		pcToken->Init(psPosition, pcSeparator);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenScope* CJavaTokenMemory::CreateScope(STextPosition* psPosition, CJavaTokenScopeDefinition* pcGeneric)
{
	CJavaTokenScope* pcToken;

	pcToken = (CJavaTokenScope*)mcStack.Add(sizeof(CJavaTokenScope));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenScope;
		pcToken->Init(psPosition, pcGeneric);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCJavaTokenAmbiguous* CJavaTokenMemory::CreateAmbiguous(STextPosition* psPosition, CCJavaTokenAmbiguousDefinition* pcAmbiguous)
{
	CCJavaTokenAmbiguous* pcToken;

	pcToken = (CCJavaTokenAmbiguous*)mcStack.Add(sizeof(CCJavaTokenAmbiguous));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CCJavaTokenAmbiguous;
		pcToken->Init(psPosition, pcAmbiguous);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenNull* CJavaTokenMemory::CreateNull(STextPosition* psPosition)
{
	CJavaTokenNull*	pcToken;

	pcToken = (CJavaTokenNull*)mcStack.Add(sizeof(CJavaTokenNull));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenNull;
		pcToken->Init(psPosition);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenBoolean* CJavaTokenMemory::CreateBoolean(STextPosition* psPosition, bool bValue)
{
	CJavaTokenBoolean* pcToken;

	pcToken = (CJavaTokenBoolean*)mcStack.Add(sizeof(CJavaTokenBoolean));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenBoolean;
		pcToken->Init(psPosition, bValue);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenCharacter* CJavaTokenMemory::CreateCharacter(STextPosition* psPosition, char c)
{
	CJavaTokenCharacter* pcToken;

	pcToken = (CJavaTokenCharacter*)mcStack.Add(sizeof(CJavaTokenCharacter));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenCharacter;
		pcToken->Init(psPosition, c);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenCharacter* CJavaTokenMemory::CreateCharacter(STextPosition* psPosition, char16 c)
{
	CJavaTokenCharacter* pcToken;

	pcToken = (CJavaTokenCharacter*)mcStack.Add(sizeof(CJavaTokenCharacter));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenCharacter;
		pcToken->Init(psPosition, c);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenInteger* CJavaTokenMemory::CreateInteger(STextPosition* psPosition, int32 iValue)
{
	CJavaTokenInteger* pcToken;

	pcToken = (CJavaTokenInteger*)mcStack.Add(sizeof(CJavaTokenInteger));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenInteger;
		pcToken->Init(psPosition, iValue);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenInteger* CJavaTokenMemory::CreateInteger(STextPosition* psPosition, int64 iValue)
{
	CJavaTokenInteger* pcToken;

	pcToken = (CJavaTokenInteger*)mcStack.Add(sizeof(CJavaTokenInteger));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenInteger;
		pcToken->Init(psPosition, iValue);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenFloat* CJavaTokenMemory::CreateFloat(STextPosition* psPosition, float32 fValue)
{
	CJavaTokenFloat* pcToken;

	pcToken = (CJavaTokenFloat*)mcStack.Add(sizeof(CJavaTokenFloat));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenFloat;
		pcToken->Init(psPosition, fValue);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenFloat* CJavaTokenMemory::CreateFloat(STextPosition* psPosition, float64 fValue)
{
	CJavaTokenFloat* pcToken;

	pcToken = (CJavaTokenFloat*)mcStack.Add(sizeof(CJavaTokenFloat));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaTokenFloat;
		pcToken->Init(psPosition, fValue);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenString* CJavaTokenMemory::CreateString(STextPosition* psPosition, char* szString, int iLength)
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
		pcToken->Init(psPosition, szDest, iLength);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenString* CJavaTokenMemory::CreateString(STextPosition* psPosition, char16* szString, int iLength)
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
		pcToken->Init(psPosition, szDest, iLength);
	}

	return pcToken;
}

