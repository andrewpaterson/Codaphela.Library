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

	mapcClasses.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxFile::Kill(void)
{
	mpcPackage = NULL;;
	mapcImports.Kill();

	mapcClasses.Kill();

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
void CJavaSyntaxFile::TypePrint(CChars* pszDest, int iDepth)
{
	int						i;
	CJavaSyntaxImport*		pcSyntax;
	CJavaSyntaxClassCommon*	pcClassCommon;

	CJavaSyntax::TypePrint(pszDest, iDepth);
	pszDest->Append(&mszFilename);
	pszDest->AppendNewLine();

	if (mpcPackage)
	{
		mpcPackage->TypePrint(pszDest, iDepth + 1);
	}

	for (i = 0; i < mapcImports.NumElements(); i++)
	{
		pcSyntax = mapcImports.GetPtr(i);
		pcSyntax->TypePrint(pszDest, iDepth + 1);
	}

	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClassCommon = mapcClasses.GetPtr(i);
		pcClassCommon->TypePrint(pszDest, iDepth + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxFile::PrettyPrint(CChars* pszDest, int iBlockDepth)
{
	int							i;
	CJavaSyntaxImport*			pcSyntax;
	CJavaSyntaxClassCommon*		pcClassCommon;

	if (mpcPackage)
	{
		mpcPackage->PrettyPrint(pszDest);
		pszDest->AppendNewLine();
		pszDest->AppendNewLine();
	}

	for (i = 0; i < mapcImports.NumElements(); i++)
	{
		pcSyntax = mapcImports.GetPtr(i);
		pcSyntax->PrettyPrint(pszDest);
		pszDest->AppendNewLine();
	}

	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClassCommon = mapcClasses.GetPtr(i);
		pcClassCommon->PrettyPrint(pszDest, iBlockDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaSyntaxFile::IsFile(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaSyntaxFile::SetPackage(CJavaSyntaxPackage* pcPackage)
{
	if (mpcPackage == NULL)
	{
		mpcPackage = pcPackage;
		return true;
	}
	else
	{
		return false;
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
void CJavaSyntaxFile::AddClass(CJavaSyntaxClassCommon* pcClassCommon)
{
	mapcClasses.Add(pcClassCommon);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSyntaxPackage* CJavaSyntaxFile::GetPackage(void)
{
	return mpcPackage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImportArray* CJavaSyntaxFile::GetImports(void)
{
	return &mapcImports;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClassCommonArray* CJavaSyntaxFile::GetClasses(void)
{
	return &mapcClasses;
}

