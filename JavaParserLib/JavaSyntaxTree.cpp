#include "JavaSyntaxTree.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxTree::Init(void)
{
	mpcRoot = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxTree::Kill(void)
{
	mpcRoot = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxTree::SetRoot(CJavaSyntaxFile* pcRoot)
{
	mpcRoot = pcRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxTree::Print(CChars* pszDest)
{
	mpcRoot->Print(pszDest, 0);
}

