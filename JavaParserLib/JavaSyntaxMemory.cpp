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
CJavaSyntaxFile* CJavaSyntaxMemory::CreateFile(CJavaSyntaxTree* pcTree)
{
	return Create<CJavaSyntaxFile>(pcTree);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxPackage* CJavaSyntaxMemory::CreatePackage(CJavaSyntaxTree* pcTree)
{
	return Create<CJavaSyntaxPackage>(pcTree);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxImport* CJavaSyntaxMemory::CreateImport(CJavaSyntaxTree* pcTree)
{
	return Create<CJavaSyntaxImport>(pcTree);
}

