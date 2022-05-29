#include "JavaSyntaxStatement.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxStatement::Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	CJavaSyntax::Init(pcTree, pcParent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxStatement::Kill(void)
{
	CJavaSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxStatement::GetType(void)
{
	return "Statement";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxStatement::TypePrint(CChars* pszDest, int iDepth)
{
	CJavaSyntax::TypePrint(pszDest, iDepth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxStatement::IsStatement(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxStatement::PrintModifiers(CChars* pszDest, CJavaModifiers cModifiers)
{
	cModifiers.Print(pszDest);
	if (!cModifiers.IsNone())
	{
		pszDest->Append(' ');
	}
}

