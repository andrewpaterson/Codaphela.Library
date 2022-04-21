#include "BaseLib/Numbers.h"
#include "JavaTokenMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenMemory::Init(void)
{
	mcStack.Init(4 KB);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenMemory::Kill(void)
{
	//CJavaToken*			pcToken;
	//int					iNumTokens;
	//int					i;

	//for (i = 0; i < iNumTokens; i++)
	//{
	//	pcToken = mapcTokens.Get(i);
	//	pcToken->Kill();
	//}

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
		szDest = (char*)RemapSinglePointer(pcToken, sizeof(CJavaComment));
		memcpy(szDest, szComment, iLength);
		szDest[iLength] = '\0';
	}

	return pcToken;
}

