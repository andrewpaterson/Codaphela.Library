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
	CJavaToken*			pcToken;
	int					iNumTokens;
	int					i;

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


