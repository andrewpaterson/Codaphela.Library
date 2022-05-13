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
BOOL CJavaSyntax::IsEnum(void) { return FALSE; }
BOOL CJavaSyntax::IsInterface(void) { return FALSE; }
BOOL CJavaSyntax::IsClass(void) { return FALSE; }
BOOL CJavaSyntax::IsClassGeneric(void) { return FALSE; }
BOOL CJavaSyntax::IsType(void) { return FALSE; }
BOOL CJavaSyntax::IsTypeGeneric(void) { return FALSE; }

BOOL CJavaSyntax::IsError(void) { return FALSE; }
BOOL CJavaSyntax::IsMismatch(void) { return FALSE; }

