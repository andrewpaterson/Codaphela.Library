#include "JavaSyntaxClass.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxClass::Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	CJavaSyntaxClassCommon::Init(pcTree, pcParent);
	mpcType = NULL;
	mbAbstract = FALSE;
	mbFinal = FALSE;
	mbProtected = FALSE;
	mbPrivate = FALSE;
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
	mbProtected = FALSE;
	mbPrivate = FALSE;
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
	CJavaSyntax::Print(pszDest, iDepth);
	if (mbPublic)
	{
		pszDest->Append("public ");
	}
	if (mbPrivate)
	{
		pszDest->Append("private ");
	}
	if (mbProtected)
	{
		pszDest->Append("protected ");
	}
	if (mbFinal)
	{
		pszDest->Append("final ");
	}
	if (mbAbstract)
	{
		pszDest->Append("abstract ");
	}

	pszDest->AppendNewLine();
	mpcType->Print(pszDest, iDepth + 1);
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
BOOL CJavaSyntaxClass::IsPackageModifier(void)
{
	return !mbPublic && !mbPrivate && !mbProtected;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxClass::SetAbstract(BOOL bAbstract) { mbAbstract = bAbstract; }
void CJavaSyntaxClass::SetFinal(BOOL bFinal) { mbFinal = bFinal; }
void CJavaSyntaxClass::SetSyntaxType(CJavaSyntaxType* pcType) { mpcType = pcType; }
BOOL CJavaSyntaxClass::IsAbstract(void) { return mbAbstract; }
BOOL CJavaSyntaxClass::IsFinal(void) { return mbFinal; }
BOOL CJavaSyntaxClass::IsProtected(void) { return mbProtected; }
BOOL CJavaSyntaxClass::IsPrivate(void) { return mbPrivate; }

