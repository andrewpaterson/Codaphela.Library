#include "JavaSyntax.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntax::Init(CJavaSyntaxTree* pcTree)
{
	mpcTree = pcTree;
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
BOOL CJavaSyntax::IsFile(void) { return FALSE; }
BOOL CJavaSyntax::IsImport(void) { return FALSE; }
BOOL CJavaSyntax::IsPackage(void) { return FALSE; }
BOOL CJavaSyntax::IsTopLevel(void) { return FALSE; }

