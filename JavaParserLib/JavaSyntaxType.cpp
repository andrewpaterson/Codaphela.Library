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
	mapcGenerics.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxType::Kill(void)
{
	mpcName = NULL;
	mapcGenerics.Kill();
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
	int						i;
	CJavaSyntaxGeneric*		pcGenric;

	CJavaSyntax::Print(pszDest, iDepth);
	mpcName->Print(pszDest);
	pszDest->AppendNewLine();

	for (i = 0; i < mapcGenerics.NumElements(); i++)
	{
		pcGenric = mapcGenerics.GetPtr(i);
		pcGenric->Print(pszDest, iDepth + 1);
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
void CJavaSyntaxType::AddGeneric(CJavaSyntaxGeneric* pcGeneric) { mapcGenerics.Add(pcGeneric); }
void CJavaSyntaxType::SetName(CJavaTokenIdentifier* pcName) { mpcName = pcName; }

