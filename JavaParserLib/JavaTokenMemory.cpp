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
CJavaKeyword* CJavaTokenMemory::CreateKeyword(EJavaKeyword eKeyword)
{
	CJavaKeyword*	pcToken;

	pcToken = (CJavaKeyword*)mcStack.Add(sizeof(CJavaKeyword));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaKeyword;
		pcToken->Init(eKeyword);
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
CJavaOperator* CJavaTokenMemory::CreateOperator(EJavaOperatorType eType, EJavaOperator eOperator)
{
	CJavaOperator* pcToken;

	pcToken = (CJavaOperator*)mcStack.Add(sizeof(CJavaOperator));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaOperator;
		pcToken->Init(eType, eOperator);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSeparator* CJavaTokenMemory::CreateSeparator(EJavaSeparator eSeparator)
{
	CJavaSeparator* pcToken;

	pcToken = (CJavaSeparator*)mcStack.Add(sizeof(CJavaSeparator));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaSeparator;
		pcToken->Init(eSeparator);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaGeneric* CJavaTokenMemory::CreateGeneric(EJavaGeneric eGeneric)
{
	CJavaGeneric* pcToken;

	pcToken = (CJavaGeneric*)mcStack.Add(sizeof(CJavaGeneric));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaGeneric;
		pcToken->Init(eGeneric);
	}

	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaAmbiguous* CJavaTokenMemory::CreateAmbiguous(EJavaAmbiguous eAmbiguous)
{
	CJavaAmbiguous* pcToken;

	pcToken = (CJavaAmbiguous*)mcStack.Add(sizeof(CJavaAmbiguous));
	if (pcToken)
	{
		mapcTokens.Add(pcToken);

		new(pcToken) CJavaAmbiguous;
		pcToken->Init(eAmbiguous);
	}

	return pcToken;
}


