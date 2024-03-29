#include "JavaSyntaxVariableInitialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxVariableInitialiser::Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	CJavaSyntax::Init(pcTree, pcParent);
	mpcArrayValueExpression = NULL;
	mpcSingleValueExpression = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxVariableInitialiser::Kill(void)
{
	mpcSingleValueExpression = NULL;
	mpcArrayValueExpression = NULL;
	CJavaSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxVariableInitialiser::GetType(void)
{
	return "Initialiser";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxVariableInitialiser::TypePrint(CChars* pszDest, int iDepth)
{
	CJavaSyntax::TypePrint(pszDest, iDepth);
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxVariableInitialiser::PrettyPrint(CChars* pszDest, int iBlockDepth)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaSyntaxVariableInitialiser::IsArrayExpression(void)
{
	return (mpcArrayValueExpression != NULL) && (mpcSingleValueExpression == NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaSyntaxVariableInitialiser::IsSingleExpression(void)
{
	return (mpcArrayValueExpression == NULL) && (mpcSingleValueExpression != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaSyntaxVariableInitialiser::IsVariableInitialiser(void) { return true; }
void CJavaSyntaxVariableInitialiser::SetArrayValueExpression(CJavaSyntaxArrayValueExpression* pcArrayValueExpression) { mpcArrayValueExpression = pcArrayValueExpression; }
void CJavaSyntaxVariableInitialiser::SetSingleValueExpression(CJavaSyntaxValueExpression* pValueExpression) { mpcSingleValueExpression = pValueExpression; }
CJavaSyntaxArrayValueExpression* CJavaSyntaxVariableInitialiser::GetArrayValueExpression(void) { return mpcArrayValueExpression; }
CJavaSyntaxValueExpression* CJavaSyntaxVariableInitialiser::GetSingleValueExpression(void) { return mpcSingleValueExpression; }

