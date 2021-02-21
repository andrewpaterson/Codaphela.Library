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
	cFileUtil.AppendToPath(pszFullFileName, pszFullDirectory->Text());
	pszFullDirectory->Set(pszFullFileName);

	cFileUtil.AppendToPath(pszFullFileName, szShortFileName.Text());
	szShortFileName.Kill();
}
