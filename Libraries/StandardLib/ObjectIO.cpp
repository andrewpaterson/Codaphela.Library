#include "BaseLib/Chars.h"
#include "BaseLib/FileUtil.h"
#include "SerialisedObject.h"
#include "ObjectFileGeneral.h"
#include "ObjectIO.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CObjectIO::Unnamed(OIndex oi, CChars* pszDestName)
{
	pszDestName->Init(OBJECT_UNNAMED_FILE"/");
	pszDestName->AppendHexHiLo(&oi, sizeof(OIndex));
	return pszDestName->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CObjectIO::Unnamed(CSerialisedObject* pcSerialised, CChars* pszDestName)
{
	return Unnamed(pcSerialised->GetIndex(), pszDestName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CObjectIO::Filename(const char* szSerialisedName, const char* szWorkingDirectory, CChars* pszDestDirectory, CChars* pszDestFullFilename)
{
	CFileUtil	cFileUtil;
	CChars		szShortFilename;
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

	szShortFilename.Init();
	pszDirectoryTemp->Init();
	cFileUtil.SplitPath(szSerialisedName, &szShortFilename, pszDirectoryTemp);
	szShortFilename.Append(".");
	szShortFilename.Append(OBJECT_FILE_EXTENSION);

	pszDestFullFilename->Init(szWorkingDirectory);
	cFileUtil.AppendToPath(pszDestFullFilename, pszDirectoryTemp->Text());
	pszDirectoryTemp->Set(pszDestFullFilename);

	cFileUtil.AppendToPath(pszDestFullFilename, szShortFilename.Text());
	szShortFilename.Kill();

	if (!pszDestDirectory)
	{
		szDirectory.Kill();
	}

	return pszDestFullFilename->Text();
}

