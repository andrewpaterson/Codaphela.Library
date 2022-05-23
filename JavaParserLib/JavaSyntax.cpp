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
void CJavaSyntax::Print(CChars* pszDest, int iDepth)
{
	pszDest->Append(' ', iDepth * 2);
	pszDest->Append(GetType());
	pszDest->Append(": ");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntax::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz, 0);
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

BOOL CJavaSyntax::IsBlock(void) { return FALSE; }
BOOL CJavaSyntax::IsExpression(void) { return FALSE; }
BOOL CJavaSyntax::IsOriginalFor(void) { return FALSE; }
BOOL CJavaSyntax::IsIterativeFor(void) { return FALSE; }
BOOL CJavaSyntax::IsDeclaration(void) { return FALSE; }
BOOL CJavaSyntax::IsAssignment(void) { return FALSE; }

BOOL CJavaSyntax::IsStatement(void) { return FALSE; }
BOOL CJavaSyntax::IsClassCommon(void) { return FALSE; }
BOOL CJavaSyntax::IsTypeCommon(void) { return FALSE; }
BOOL CJavaSyntax::IsVariableCommon(void) { return FALSE; }

BOOL CJavaSyntax::IsError(void) { return FALSE; }
BOOL CJavaSyntax::IsMismatch(void) { return FALSE; }

