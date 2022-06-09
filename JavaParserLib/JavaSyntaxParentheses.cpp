#include "JavaSyntaxParentheses.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParentheses::Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	CJavaSyntaxExpressionCommon::Init(pcTree, pcParent);
	mpcExpression = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParentheses::Kill(void)
{
	mpcExpression = NULL;
	CJavaSyntaxExpressionCommon::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxParentheses::GetType(void)
{
	return "Parentheses";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParentheses::TypePrint(CChars* pszDest, int iDepth)
{
	CJavaSyntaxExpressionCommon::TypePrint(pszDest, iDepth);
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParentheses::PrettyPrint(CChars* pszDest, int iBlockDepth)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParentheses::IsParentheses(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxParentheses::IsCompoundStatement(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxParentheses::SetExpression(CJavaSyntaxExpressionCommon* pcExpression)
{
	mpcExpression = pcExpression;
}
