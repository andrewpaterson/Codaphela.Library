#include "JavaSyntaxType.h"
#include "JavaSyntaxGeneric.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxGeneric::Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	CJavaSyntax::Init(pcTree, pcParent);
	mapcGenerics.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxGeneric::Kill(void)
{
	mapcGenerics.Kill();
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
void CJavaSyntaxGeneric::TypePrint(CChars* pszDest, int iDepth)
{
	size					i;
	CJavaSyntaxTypeCommon*	pcType;
	size					uiNumElements;

	CJavaSyntax::TypePrint(pszDest, iDepth);
	pszDest->AppendNewLine();

	uiNumElements = mapcGenerics.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcType = mapcGenerics.GetPtr(i);
		pcType->TypePrint(pszDest, iDepth + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxGeneric::PrettyPrint(CChars* pszDest, int iBlockDepth)
{
	size					i;
	CJavaSyntaxTypeCommon*	pcType;
	size					uiNumElements;

	pszDest->Append('<');

	uiNumElements = mapcGenerics.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		if (i != 0)
		{
			pszDest->Append(", ");
		}
		pcType = mapcGenerics.GetPtr(i);
		pcType->PrettyPrint(pszDest);
	}
	pszDest->Append('>');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaSyntaxGeneric::IsGeneric(void) { return true; }
void CJavaSyntaxGeneric::AddType(CJavaSyntaxTypeCommon* pcType) { mapcGenerics.Add(pcType); }
CTypeCommonArray* CJavaSyntaxGeneric::GetGenerics(void) { return &mapcGenerics; }

