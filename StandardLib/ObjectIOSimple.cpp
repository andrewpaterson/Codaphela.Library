#include "BaseLib/FileUtil.h"
#include "ObjectFileGeneral.h"
#include "ObjectIOSimple.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectIOSimple::FileName(char* szSerialisedName, char* szWorkingDirectory, CChars* pszFullDirectory, CChars* pszFullFileName)
{
	CFileUtil	cFileUtil;
	CChars		szShortFileName;

	szShortFileName.Init();
	pszFullDirectory->Init();
	cFileUtil.SplitPath(szSerialisedName, &szShortFileName, pszFullDirectory);
	szShortFileName.Append(".");
	szShortFileName.Append(OBJECT_FILE_EXTENSION);

	pszFullFileName->Init(szWorkingDirectory);
	pszFullFileName->Append(FILE_SEPARATOR[0]);
	pszFullFileName->Append(pszFullDirectory);
	pszFullDirectory->Set(pszFullFileName);

	pszFullFileName->Append(FILE_SEPARATOR[0]);
	pszFullFileName->Append(szShortFileName);
	szShortFileName.Kill();
}
