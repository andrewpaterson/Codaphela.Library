#include "JavaSyntaxType.h"
#include "JavaSyntaxExtent.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxExtent::Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	CJavaSyntaxTypeCommon::Init(pcTree, pcParent);

	mpcName = NULL;
	mbWildCard = false;
	mpcExtends = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxExtent::Kill(void)
{
	mpcName = NULL;
	mbWildCard = false;
	mpcExtends = NULL;

	CJavaSyntaxTypeCommon::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxExtent::GetType(void)
{
	return "Extent";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxExtent::TypePrint(CChars* pszDest, int iDepth)
{
	CJavaSyntaxTypeCommon::TypePrint(pszDest, iDepth);
	if (mbWildCard)
	{
		pszDest->Append('?');
	}
	if (mpcName) 
	{
		mpcName->Print(pszDest);
	}
	pszDest->AppendNewLine();

	if (mpcExtends)
	{
		mpcExtends->TypePrint(pszDest, iDepth + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxExtent::PrettyPrint(CChars* pszDest, int iBlockDepth)
{
	CJavaSyntaxTypeCommon::PrettyPrint(pszDest);
	if (mbWildCard)
	{
		pszDest->Append('?');
	}

	if (mpcExtends)
	{
		pszDest->Append(" extends ");
		mpcExtends->PrettyPrint(pszDest, iBlockDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaSyntaxExtent::IsExtent(void) { return true; }
void CJavaSyntaxExtent::SetExtends(CJavaSyntaxType* pcExtends) { mpcExtends = pcExtends; }
void CJavaSyntaxExtent::SetWildCard(bool bWildCard) { mbWildCard = bWildCard; }
bool CJavaSyntaxExtent::IsWildCard(void) { return mbWildCard; }

