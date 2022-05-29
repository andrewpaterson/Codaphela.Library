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
BOOL CJavaSyntax::IsFile(void) { return FALSE; }
BOOL CJavaSyntax::IsImport(void) { return FALSE; }
BOOL CJavaSyntax::IsPackage(void) { return FALSE; }
BOOL CJavaSyntax::IsEnum(void) { return FALSE; }
BOOL CJavaSyntax::IsInterface(void) { return FALSE; }
BOOL CJavaSyntax::IsClass(void) { return FALSE; }
BOOL CJavaSyntax::IsType(void) { return FALSE; }
BOOL CJavaSyntax::IsExtent(void) { return FALSE; }
BOOL CJavaSyntax::IsGeneric(void) { return FALSE; }

BOOL CJavaSyntax::IsClassBlock(void) { return FALSE; }
BOOL CJavaSyntax::IsExpression(void) { return FALSE; }
BOOL CJavaSyntax::IsOriginalFor(void) { return FALSE; }
BOOL CJavaSyntax::IsIterativeFor(void) { return FALSE; }
BOOL CJavaSyntax::IsVariableDeclaration(void) { return FALSE; }
BOOL CJavaSyntax::IsVariableInitialiser(void) { return FALSE; }
BOOL CJavaSyntax::IsVoidExpression(void) { return FALSE; }
BOOL CJavaSyntax::IsValueExpression(void) { return FALSE; }
BOOL CJavaSyntax::IsArrayValueExpression(void) { return FALSE; }

BOOL CJavaSyntax::IsStatement(void) { return FALSE; }
BOOL CJavaSyntax::IsClassCommon(void) { return FALSE; }
BOOL CJavaSyntax::IsTypeCommon(void) { return FALSE; }
BOOL CJavaSyntax::IsVariableCommon(void) { return FALSE; }
BOOL CJavaSyntax::IsExpressionCommon(void) { return FALSE; }

BOOL CJavaSyntax::IsError(void) { return FALSE; }
BOOL CJavaSyntax::IsMismatch(void) { return FALSE; }

