#include "JavaSyntaxValueExpression.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxValueExpression::Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	CJavaSyntaxExpressionCommon::Init(pcTree, pcParent);
	mapcExpressions.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxValueExpression::Kill(void)
{
	mapcExpressions.Kill();
	CJavaSyntaxExpressionCommon::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxValueExpression::GetType(void)
{
	return "Value";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxValueExpression::TypePrint(CChars* pszDest, int iDepth)
{
	CJavaSyntaxExpressionCommon::TypePrint(pszDest, iDepth);
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxValueExpression::PrettyPrint(CChars* pszDest, int iBlockDepth)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxValueExpression::IsValueExpression(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxValueExpression::IsCompoundStatement(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxValueExpression::AddExpression(CJavaSyntaxExpressionCommon* pcExpression)
{
	mapcExpressions.Add(pcExpression);
}

