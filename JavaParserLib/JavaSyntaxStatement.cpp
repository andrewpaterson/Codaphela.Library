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
void CJavaSyntaxStatement::Print(CChars* pszDest, int iDepth)
{
	CJavaSyntax::Print(pszDest, iDepth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxStatement::IsStatement(void)
{
	return TRUE;
}

