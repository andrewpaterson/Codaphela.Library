#include "JavaSyntaxVoidExpression.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxVoidExpression::Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	CJavaSyntaxStatement::Init(pcTree, pcParent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxVoidExpression::Kill(void)
{
	CJavaSyntaxStatement::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxVoidExpression::GetType(void)
{
	return "Void Expression";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxVoidExpression::TypePrint(CChars* pszDest, int iDepth)
{
	CJavaSyntaxStatement::TypePrint(pszDest, iDepth);
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxVoidExpression::IsVoidExpression(void)
{
	return TRUE;
}

