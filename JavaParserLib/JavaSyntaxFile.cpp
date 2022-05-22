#include "JavaSyntaxFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxFile::Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	CJavaSyntax::Init(pcTree, pcParent);

	mszFilename.Init();

	mpcPackage = NULL;;
	mapcImports.Init();

	mpcClass = NULL;
	mapcPackageClasses.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxFile::Kill(void)
{
	mpcPackage = NULL;;
	mapcImports.Kill();

	mpcClass = NULL;
	mapcPackageClasses.Kill();

	mszFilename.Kill();

	CJavaSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxFile::SetFileName(char* szFileName)
{
	mszFilename.Set(szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxFile::GetType(void)
{
	return "File";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxFile::Print(CChars* pszDest, int iDepth)
{
	int						i;
	CJavaSyntaxImport*		pcSyntax;
	CJavaSyntaxClassCommon*	pcClassCommon;

	CJavaSyntax::Print(pszDest, iDepth);
	pszDest->Append(&mszFilename);
	pszDest->AppendNewLine();

	if (mpcPackage)
	{
		mpcPackage->Print(pszDest, iDepth + 1);
	}

	for (i = 0; i < mapcImports.NumElements(); i++)
	{
		pcSyntax = mapcImports.GetPtr(i);
		pcSyntax->Print(pszDest, iDepth + 1);
	}

	if (mpcClass)
	{
		mpcClass->Print(pszDest, iDepth + 1);
	}

	for (i = 0; i < mapcPackageClasses.NumElements(); i++)
	{
		pcClassCommon = mapcPackageClasses.GetPtr(i);
		pcClassCommon->Print(pszDest, iDepth + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxFile::IsFile(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxFile::SetPackage(CJavaSyntaxPackage* pcPackage)
{
	if (mpcPackage == NULL)
	{
		mpcPackage = pcPackage;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxFile::AddImport(CJavaSyntaxImport* pcImport)
{
	mapcImports.Add(pcImport);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxFile::SetClass(CJavaSyntaxClassCommon* pcClassCommon)
{
	if (mpcClass == NULL)
	{
		mpcClass = pcClassCommon;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxFile::HasClass(void)
{
	return mpcClass != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxFile::AddPackageClass(CJavaSyntaxClassCommon* pcClassCommon)
{
	mapcPackageClasses.Add(pcClassCommon);
}

