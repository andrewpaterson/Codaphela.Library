#include "JavaSyntaxGeneric.h"
#include "JavaSyntaxType.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxType::Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	CJavaSyntax::Init(pcTree, pcParent);
	mpcName = NULL;
	mpcGeneric = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxType::Kill(void)
{
	mpcName = NULL;
	mpcGeneric = NULL;
	CJavaSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxType::GetType(void)
{
	return "Type";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxType::Print(CChars* pszDest, int iDepth)
{
	CJavaSyntax::Print(pszDest, iDepth);
	if (mpcName)
	{
		mpcName->Print(pszDest);
	}
	pszDest->AppendNewLine();

	if (mpcGeneric)
	{
		mpcGeneric->Print(pszDest, iDepth + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxType::IsType(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxType::SetGeneric(CJavaSyntaxGeneric* pcGeneric) { mpcGeneric = pcGeneric; }


