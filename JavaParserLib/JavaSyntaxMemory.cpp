#include "BaseLib/Numbers.h"
#include "JavaSyntaxMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxMemory::Init(void)
{
	mcStack.Init(4 KB);
	mapcSyntaxs.Init();
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

	iNumSyntaxs = mapcSyntaxs.NumElements();
	for (i = 0; i < iNumSyntaxs; i++)
	{
		pcSyntax = mapcSyntaxs.GetPtr(i);
		pcSyntax->Kill();
	}

	mapcSyntaxs.Kill();
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
		mapcSyntaxs.Add(pcSyntax);

		new(pcSyntax) CJavaSyntaxFile;
		pcSyntax->Init(pcTree);
	}

	return pcSyntax;
}

