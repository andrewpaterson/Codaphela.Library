#include "JavaSyntaxOperator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxOperator::Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	CJavaSyntaxExpressionCommon::Init(pcTree, pcParent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxOperator::Kill(void)
{
	CJavaSyntaxExpressionCommon::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxOperator::GetType(void)
{
	return "Operator";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxOperator::TypePrint(CChars* pszDest, int iDepth)
{
	CJavaSyntaxExpressionCommon::TypePrint(pszDest, iDepth);
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxOperator::PrettyPrint(CChars* pszDest, int iBlockDepth)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaSyntaxOperator::IsOperator(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaSyntaxOperator::IsCompoundStatement(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxOperator::SetOperator(CJavaTokenOperator* pcTokenOperator)
{
	mpcTokenOperator = pcTokenOperator;
}

