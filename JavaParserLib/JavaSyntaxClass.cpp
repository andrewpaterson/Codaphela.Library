#include "JavaSyntaxClass.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxClass::Init(CJavaSyntaxTree* pcTree)
{
	CJavaSyntaxTopLevel::Init(pcTree);
	mpcType = NULL;
	mbAbstract = FALSE;
	mbFinal = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxClass::Kill(void)
{
	mbAbstract = FALSE;
	mbFinal = FALSE;
	mpcType = NULL;
	CJavaSyntaxTopLevel::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxClass::GetType(void)
{
	return "Class";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxClass::IsClass(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxClass::SetAbstract(BOOL bAbstract) { mbAbstract = bAbstract; }
void CJavaSyntaxClass::SetFinal(BOOL bFinal) { mbFinal = bFinal; }
void CJavaSyntaxClass::SetSyntaxType(CJavaSyntaxType* pcType) { mpcType = pcType; }

