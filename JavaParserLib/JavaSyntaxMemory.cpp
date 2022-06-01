#include "BaseLib/Numbers.h"
#include "JavaSyntaxMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxMemory::Init(void)
{
	mcStack.Init(4 KB);
	mapcSyntaxes.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxMemory::Kill(void)
{
	CJavaSyntax*	pcSyntax;
	int				iNumSyntaxs;
	int				i;

	iNumSyntaxs = mapcSyntaxes.NumElements();
	for (i = 0; i < iNumSyntaxs; i++)
	{
		pcSyntax = mapcSyntaxes.GetPtr(i);
		pcSyntax->Kill();
	}

	mapcSyntaxes.Kill();
	mcStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxFile*					CJavaSyntaxMemory::CreateFile(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)					{ return Create<CJavaSyntaxFile>(pcTree, pcParent); }
CJavaSyntaxPackage*					CJavaSyntaxMemory::CreatePackage(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)				{ return Create<CJavaSyntaxPackage>(pcTree, pcParent); }
CJavaSyntaxImport*					CJavaSyntaxMemory::CreateImport(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)					{ return Create<CJavaSyntaxImport>(pcTree, pcParent); }
CJavaSyntaxClass*					CJavaSyntaxMemory::CreateClass(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)					{ return Create<CJavaSyntaxClass>(pcTree, pcParent); }
CJavaSyntaxEnum*					CJavaSyntaxMemory::CreateEnum(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)					{ return Create<CJavaSyntaxEnum>(pcTree, pcParent); }
CJavaSyntaxInterface*				CJavaSyntaxMemory::CreateInterface(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)				{ return Create<CJavaSyntaxInterface>(pcTree, pcParent); }
CJavaSyntaxType*					CJavaSyntaxMemory::CreateType(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)					{ return Create<CJavaSyntaxType>(pcTree, pcParent); }
CJavaSyntaxExtent*					CJavaSyntaxMemory::CreateExtent(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)					{ return Create<CJavaSyntaxExtent>(pcTree, pcParent); }
CJavaSyntaxGeneric*					CJavaSyntaxMemory::CreateGeneric(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)				{ return Create<CJavaSyntaxGeneric>(pcTree, pcParent); }
CJavaSyntaxClassBlock*				CJavaSyntaxMemory::CreateClassBlock(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)				{ return Create<CJavaSyntaxClassBlock>(pcTree, pcParent); }
CJavaSyntaxVariableDeclaration*		CJavaSyntaxMemory::CreateVariableDeclaration(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)	{ return Create<CJavaSyntaxVariableDeclaration>(pcTree, pcParent); }
CJavaSyntaxValueExpression*			CJavaSyntaxMemory::CreateValueExpression(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)		{ return Create<CJavaSyntaxValueExpression>(pcTree, pcParent); }
CJavaSyntaxArrayValueExpression*	CJavaSyntaxMemory::CreateArrayValueExpression(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)	{ return Create<CJavaSyntaxArrayValueExpression>(pcTree, pcParent); }
CJavaSyntaxVariableInitialiser*		CJavaSyntaxMemory::CreateVariableInitialiser(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)	{ return Create<CJavaSyntaxVariableInitialiser>(pcTree, pcParent); }
CJavaSyntaxLiteral*					CJavaSyntaxMemory::CreateLiteral(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)				{ return Create<CJavaSyntaxLiteral>(pcTree, pcParent); }

