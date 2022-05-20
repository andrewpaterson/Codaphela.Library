#include "JavaSyntaxType.h"
#include "JavaSyntaxGeneric.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxGeneric::Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	CJavaSyntax::Init(pcTree, pcParent);
	mpcGenericType = NULL;
	mbWildCard = FALSE;
	mpcExtends = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxGeneric::Kill(void)
{
	mpcGenericType = NULL;
	mbWildCard = FALSE;
	mpcExtends = NULL;
	CJavaSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxGeneric::GetType(void)
{
	return "Generic";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxGeneric::Print(CChars* pszDest, int iDepth)
{
	CJavaSyntax::Print(pszDest, iDepth);
	if (mbWildCard)
	{
		pszDest->Append('?');
		if (mpcGenericType)
		{
			pszDest->Append(" extends");
		}
	}
	pszDest->AppendNewLine();

	if (mpcGenericType)
	{
		mpcGenericType->Print(pszDest, iDepth + 1);
	}
	if (mpcExtends)
	{
		if (!mbWildCard)
		{
			pszDest->Append(' ', (iDepth + 1) * 2);
			pszDest->Append("extends");
		}
		mpcExtends->Print(pszDest, iDepth + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxGeneric::IsGeneric(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxGeneric::SetGenericType(CJavaSyntaxType* pcGenericType) { mpcGenericType = pcGenericType; }
void CJavaSyntaxGeneric::SetExtends(CJavaSyntaxType* pcType) { mpcExtends = pcType; }
void CJavaSyntaxGeneric::SetWildCard(BOOL bWildCard) { mbWildCard = bWildCard; }

