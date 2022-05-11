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
	CJavaSyntaxFile* pcSyntax;

	pcSyntax = (CJavaSyntaxFile*)mcStack.Add(sizeof(CJavaSyntaxFile));
	if (pcSyntax)
	{
		mapcSyntaxes.Add(pcSyntax);

		new(pcSyntax) CJavaSyntaxFile;
		pcSyntax->Init(pcTree);
	}

	return pcSyntax;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxPackage* CJavaSyntaxMemory::CreatePackage(CJavaSyntaxTree* pcTree)
{
	CJavaSyntaxPackage* pcSyntax;

	pcSyntax = (CJavaSyntaxPackage*)mcStack.Add(sizeof(CJavaSyntaxPackage));
	if (pcSyntax)
	{
		mapcSyntaxes.Add(pcSyntax);

		new(pcSyntax) CJavaSyntaxPackage;
		pcSyntax->Init(pcTree);
	}

	return pcSyntax;
}

