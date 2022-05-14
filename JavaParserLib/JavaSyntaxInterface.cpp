#include "JavaSyntaxInterface.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxInterface::Init(CJavaSyntaxTree* pcTree)
{
	CJavaSyntaxTopLevel::Init(pcTree);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxInterface::Kill(void)
{
	CJavaSyntaxTopLevel::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxInterface::GetType(void)
{
	return "Interface";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxInterface::IsInterface(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxInterface::SetAbstract(BOOL bAbstract) { mbAbstract = bAbstract; }
void CJavaSyntaxInterface::SetFinal(BOOL bFinal) { mbFinal = bFinal; }
void CJavaSyntaxInterface::SetSyntaxType(CJavaSyntaxType* pcType) { mpcType = pcType; }
