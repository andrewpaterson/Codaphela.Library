#include "JavaSyntaxType.h"
#include "JavaSyntaxGeneric.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxGeneric::Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	CJavaSyntax::Init(pcTree, pcParent);
	mpcGenericType = NULL;
	mbWildCard = FALSE;
	mpcExtends = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxGeneric::Kill(void)
{
	mpcGenericType = NULL;
	mbWildCard = FALSE;
	mpcExtends = NULL;
	CJavaSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxGeneric::GetType(void)
{
	return "Generic";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxGeneric::IsGeneric(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxGeneric::SetGenericType(CJavaSyntaxType* pcGenericType) { mpcGenericType = pcGenericType; }
void CJavaSyntaxGeneric::SetExtends(CJavaSyntaxType* pcType) { mpcExtends = pcType; }
void CJavaSyntaxGeneric::SetWildCard(BOOL bWildCard) { mbWildCard = bWildCard; }

