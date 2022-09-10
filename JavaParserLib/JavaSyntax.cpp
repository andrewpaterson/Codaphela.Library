#include "JavaSyntax.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntax::Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	mpcTree = pcTree;
	mpcParent = pcParent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntax::Kill(void)
{
	mpcTree = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntax::Clear(void)
{
	Kill();
	Init(NULL, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntax::TypePrint(CChars* pszDest, int iDepth)
{
	pszDest->Append(' ', iDepth * 2);
	pszDest->Append(GetType());
	pszDest->Append(": ");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntax::PrintTokenArray(CChars* pszDest, CIdentifierArray* papcTokens)
{
	int						i;
	CJavaTokenIdentifier*	pcIdentifier;

	for (i = 0; i < papcTokens->NumElements(); i++)
	{
		if (i != 0)
		{
			pszDest->Append('.');
		}

		pcIdentifier = papcTokens->GetPtr(i);
		pcIdentifier->Print(pszDest);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntax::Dump(void)
{
	CChars	sz;

	sz.Init();
	PrettyPrint(&sz, 0);
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaSyntax::IsFile(void) { return false; }
bool CJavaSyntax::IsImport(void) { return false; }
bool CJavaSyntax::IsPackage(void) { return false; }
bool CJavaSyntax::IsEnum(void) { return false; }
bool CJavaSyntax::IsInterface(void) { return false; }
bool CJavaSyntax::IsClass(void) { return false; }
bool CJavaSyntax::IsType(void) { return false; }
bool CJavaSyntax::IsExtent(void) { return false; }
bool CJavaSyntax::IsGeneric(void) { return false; }

bool CJavaSyntax::IsClassBlock(void) { return false; }
bool CJavaSyntax::IsOriginalFor(void) { return false; }
bool CJavaSyntax::IsIterativeFor(void) { return false; }
bool CJavaSyntax::IsVariableDeclaration(void) { return false; }
bool CJavaSyntax::IsVariableInitialiser(void) { return false; }
bool CJavaSyntax::IsVoidExpression(void) { return false; }
bool CJavaSyntax::IsValueExpression(void) { return false; }
bool CJavaSyntax::IsArrayValueExpression(void) { return false; }
bool CJavaSyntax::IsMethodCall(void) { return false; }
bool CJavaSyntax::IsParentheses(void) { return false; }
bool CJavaSyntax::IsOperator(void) { return false; }

bool CJavaSyntax::IsStatement(void) { return false; }
bool CJavaSyntax::IsClassCommon(void) { return false; }
bool CJavaSyntax::IsTypeCommon(void) { return false; }
bool CJavaSyntax::IsVariableCommon(void) { return false; }
bool CJavaSyntax::IsExpressionCommon(void) { return false; }
bool CJavaSyntax::IsLiteral(void) { return false; }

bool CJavaSyntax::IsError(void) { return false; }
bool CJavaSyntax::IsMismatch(void) { return false; }

