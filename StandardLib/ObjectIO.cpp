#include "BaseLib/Chars.h"
#include "BaseLib/FileUtil.h"
#include "SerialisedObject.h"
#include "ObjectFileGeneral.h"
#include "ObjectIO.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CObjectIO::Unnamed(CSerialisedObject* pcSerialised, CChars* pszDestName)
{
	OIndex	oi;

	pszDestName->Init(OBJECT_UNNAMED_FILE"/");
	oi = pcSerialised->GetIndex();
	pszDestName->AppendHexHiLo(&oi, sizeof(OIndex));
	return pszDestName->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CObjectIO::FileName(char* szSerialisedName, char* szWorkingDirectory, CChars* pszDestDirectory, CChars* pszDestFullFileName)
{
	CFileUtil	cFileUtil;
	CChars		szShortFileName;
	CChars		szDirectory;
	CChars*		pszDirectoryTemp;

	if (pszDestDirectory)
	{
		pszDirectoryTemp = pszDestDirectory;
	}
	else
	{
		pszDirectoryTemp = &szDirectory;
	}

	szShortFileName.Init();
	pszDirectoryTemp->Init();
	cFileUtil.SplitPath(szSerialisedName, &szShortFileName, pszDirectoryTemp);
	szShortFileName.Append(".");
	szShortFileName.Append(OBJECT_FILE_EXTENSION);

	pszDestFullFileName->Init(szWorkingDirectory);
	cFileUtil.AppendToPath(pszDestFullFileName, pszDirectoryTemp->Text());
	pszDirectoryTemp->Set(pszDestFullFileName);

	cFileUtil.AppendToPath(pszDestFullFileName, szShortFileName.Text());
	szShortFileName.Kill();

	if (!pszDestDirectory)
	{
		szDirectory.Kill();
	}

	return pszDestFullFileName->Text();
}

