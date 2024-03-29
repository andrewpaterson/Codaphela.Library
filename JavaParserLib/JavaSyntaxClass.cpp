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
void CJavaSyntaxClass::TypePrint(CChars* pszDest, int iDepth)
{
	CJavaSyntaxClassCommon::TypePrint(pszDest, iDepth);
	mpcType->TypePrint(pszDest, iDepth + 1);
	mpcBlock->TypePrint(pszDest, iDepth + 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxClass::PrettyPrint(CChars* pszDest, int iBlockDepth)
{
	pszDest->Append('\t', iBlockDepth);
	CJavaSyntaxClassCommon::PrettyPrint(pszDest, iBlockDepth);
	mpcType->PrettyPrint(pszDest);
	pszDest->AppendNewLine();
	mpcBlock->PrettyPrint(pszDest, iBlockDepth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaSyntaxClass::IsClass(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaSyntaxClass::IsCompoundStatement(void)
{
	return true;
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxType* CJavaSyntaxClass::GetSyntaxType(void)
{
	return mpcType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxClassBlock* CJavaSyntaxClass::GetBlock(void)
{
	return mpcBlock;
}


