#include "JavaSyntaxTopLevel.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxTopLevel::Init(CJavaSyntaxTree* pcTree)
{
	CJavaSyntax::Init(pcTree);
	mpcName = NULL;
	mbPublic = FALSE;
	mbAbstract = FALSE;
	mbFinal = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxTopLevel::Kill(void)
{
	mpcName = NULL;
	mbPublic = FALSE;
	mbAbstract = FALSE;
	mbFinal = FALSE;
	CJavaSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxTopLevel::GetType(void)
{
	return "TopLevel";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxTopLevel::IsTopLevel(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxTopLevel::SetName(CJavaIdentifier* pcName) { mpcName = pcName; }
void CJavaSyntaxTopLevel::SetPublic(BOOL bPublic) { mbPublic = bPublic; }
void CJavaSyntaxTopLevel::SetAbstract(BOOL bAbstract) { mbAbstract = bAbstract; }
void CJavaSyntaxTopLevel::SetFinal(BOOL bFinal) { mbFinal = bFinal; }

