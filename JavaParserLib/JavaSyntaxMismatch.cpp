#include "JavaSyntaxMismatch.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxMismatch::GetType(void)
{
	return "Mismatch";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxMismatch::Print(CChars* pszDest, int iDepth)
{
	CJavaSyntax::Print(pszDest, iDepth);
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxMismatch::IsMismatch(void)
{
	return TRUE;
}

