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
CJavaComment* CJavaTokenMemory::CreateComment(char* szComment, int iLength)
{
	CJavaComment*	pcToken;
	char*			szDest;

	pcToken = (CJavaComment*)mcStack.Add(sizeof(CJavaComment) + iLength + 1);
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaComment;
		szDest = (char*)RemapSinglePointer(pcToken, sizeof(CJavaComment));
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
CJavaKeyword* CJavaTokenMemory::CreateKeyword(CJavaKeywordDefinition* pcKeyword)
{
	CJavaKeyword*	pcToken;

	pcToken = (CJavaKeyword*)mcStack.Add(sizeof(CJavaKeyword));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaKeyword;
		pcToken->Init(pcKeyword);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaIdentifier* CJavaTokenMemory::CreateIdentifier(char* szIdentifier, int iLength)
{
	CJavaIdentifier*	pcToken;
	char*				szDest;

	pcToken = (CJavaIdentifier*)mcStack.Add(sizeof(CJavaIdentifier) + iLength + 1);
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaIdentifier;
		szDest = (char*)RemapSinglePointer(pcToken, sizeof(CJavaIdentifier));
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
CJavaAnnotation* CJavaTokenMemory::CreateAnnotation(char* szAnnotation, int iLength)
{
	CJavaAnnotation*	pcToken;
	char*				szDest;

	pcToken = (CJavaAnnotation*)mcStack.Add(sizeof(CJavaAnnotation) + iLength + 1);
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaAnnotation;
		szDest = (char*)RemapSinglePointer(pcToken, sizeof(CJavaAnnotation));
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
CJavaOperator* CJavaTokenMemory::CreateOperator(CJavaOperatorDefinition* pcOperator)
{
	CJavaOperator* pcToken;

	pcToken = (CJavaOperator*)mcStack.Add(sizeof(CJavaOperator));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaOperator;
		pcToken->Init(pcOperator);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSeparator* CJavaTokenMemory::CreateSeparator(CJavaSeparatorDefinition* pcSeparator)
{
	CJavaSeparator* pcToken;

	pcToken = (CJavaSeparator*)mcStack.Add(sizeof(CJavaSeparator));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaSeparator;
		pcToken->Init(pcSeparator);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaGeneric* CJavaTokenMemory::CreateGeneric(CJavaGenericDefinition* pcGeneric)
{
	CJavaGeneric* pcToken;

	pcToken = (CJavaGeneric*)mcStack.Add(sizeof(CJavaGeneric));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaGeneric;
		pcToken->Init(pcGeneric);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaAmbiguous* CJavaTokenMemory::CreateAmbiguous(CJavaAmbiguousDefinition* pcAmbiguous)
{
	CJavaAmbiguous* pcToken;

	pcToken = (CJavaAmbiguous*)mcStack.Add(sizeof(CJavaAmbiguous));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaAmbiguous;
		pcToken->Init(pcAmbiguous);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaNull* CJavaTokenMemory::CreateNull(void)
{
	CJavaNull*	pcToken;

	pcToken = (CJavaNull*)mcStack.Add(sizeof(CJavaNull));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaNull;
		pcToken->Init();
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaBoolean* CJavaTokenMemory::CreateBoolean(BOOL bValue)
{
	CJavaBoolean* pcToken;

	pcToken = (CJavaBoolean*)mcStack.Add(sizeof(CJavaBoolean));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaBoolean;
		pcToken->Init(bValue);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaCharacter* CJavaTokenMemory::CreateCharacter(char c)
{
	CJavaCharacter* pcToken;

	pcToken = (CJavaCharacter*)mcStack.Add(sizeof(CJavaCharacter));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaCharacter;
		pcToken->Init(c);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaCharacter* CJavaTokenMemory::CreateCharacter(char16 c)
{
	CJavaCharacter* pcToken;

	pcToken = (CJavaCharacter*)mcStack.Add(sizeof(CJavaCharacter));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaCharacter;
		pcToken->Init(c);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaInteger* CJavaTokenMemory::CreateInteger(int32 iValue)
{
	CJavaInteger* pcToken;

	pcToken = (CJavaInteger*)mcStack.Add(sizeof(CJavaInteger));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaInteger;
		pcToken->Init(iValue);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaInteger* CJavaTokenMemory::CreateInteger(int64 iValue)
{
	CJavaInteger* pcToken;

	pcToken = (CJavaInteger*)mcStack.Add(sizeof(CJavaInteger));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaInteger;
		pcToken->Init(iValue);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaFloat* CJavaTokenMemory::CreateFloat(float32 fValue)
{
	CJavaFloat* pcToken;

	pcToken = (CJavaFloat*)mcStack.Add(sizeof(CJavaFloat));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaFloat;
		pcToken->Init(fValue);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaFloat* CJavaTokenMemory::CreateFloat(float64 fValue)
{
	CJavaFloat* pcToken;

	pcToken = (CJavaFloat*)mcStack.Add(sizeof(CJavaFloat));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaFloat;
		pcToken->Init(fValue);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaString* CJavaTokenMemory::CreateString(char* szString, int iLength)
{
	CJavaString*	pcToken;
	char*			szDest;

	pcToken = (CJavaString*)mcStack.Add(sizeof(CJavaString) + iLength + 1);
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaString;
		szDest = (char*)RemapSinglePointer(pcToken, sizeof(CJavaString));
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
CJavaString* CJavaTokenMemory::CreateString(char16* szString, int iLength)
{
	CJavaString*	pcToken;
	char16*			szDest;

	pcToken = (CJavaString*)mcStack.Add(sizeof(CJavaString) + ((iLength + 1) * sizeof(char16)));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaString;
		szDest = (char16*)RemapSinglePointer(pcToken, sizeof(CJavaString));
		memcpy(szDest, szString, iLength * sizeof(char16));
		szDest[iLength] = '\0';
		pcToken->Init(szDest, iLength);
	}

	return pcToken;
}

