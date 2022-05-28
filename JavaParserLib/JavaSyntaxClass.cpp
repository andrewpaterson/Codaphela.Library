#include "JavaSyntaxClass.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxClass::Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	CJavaSyntaxClassCommon::Init(pcTree, pcParent);
	mpcType = NULL;
	mpcBlock = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxClass::Kill(void)
{
	mpcBlock = NULL;
	mpcType = NULL;
	CJavaSyntaxClassCommon::Kill();
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
void CJavaSyntaxClass::Print(CChars* pszDest, int iDepth)
{
	CJavaSyntaxClassCommon::Print(pszDest, iDepth);
	mpcType->Print(pszDest, iDepth + 1);
	mpcBlock->Print(pszDest, iDepth + 1);
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
void CJavaSyntaxClass::SetSyntaxType(CJavaSyntaxType* pcType)
{
	mpcType = pcType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxClass::SetBlock(CJavaSyntaxClassBlock* pcBlock)
{
	mpcBlock = pcBlock;
}

