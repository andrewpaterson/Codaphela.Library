#include "BaseLib/Logger.h"
#include "BaseLib/DiskFile.h"
#include "ObjectReaderSimpleDisk.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderSimpleDisk::Init(char* szDirectory)
{
	CFileObjectReader::Init(&mcFile);
	mszFullDirectory.Init(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderSimpleDisk::Kill(void)
{
	mszFullDirectory.Kill();
	CFileObjectReader::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CObjectReaderSimpleDisk::Read(char* szObjectName)
{
	CSerialisedObject*	pcSerialised;
	CChars				szDirectory;
	CChars				szFileName;
	char				szExtension[4];
	BOOL				bResult;
	int					iFileType;

	FileName(szObjectName, mszFullDirectory.Text(), &szDirectory, &szFileName);

	mcFile.Init(DiskFile(szFileName.Text()));
	szFileName.Kill();
	szDirectory.Kill();

	bResult = mcFile.Open(EFM_Read);
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not open file [", mcFile.GetFileName(), "] for reading.", NULL);
		mcFile.Kill();
		return NULL;
	}

	//Read file type identifier.
	bResult = mcFile.ReadData(szExtension, 4);
	if ((!bResult) || (strcmp(szExtension, OBJECT_FILE_EXTENSION) != 0))
	{
		mcFile.Close();
		mcFile.Kill();
		return NULL;
	}

	bResult = mcFile.ReadInt(&iFileType);
	if ((!bResult) || (iFileType != BASIC_OBJECT_FILE))
	{
		mcFile.Close();
		mcFile.Kill();
		return NULL;
	}

	pcSerialised = ReadSerialised(&mcFile);
	if (!pcSerialised)
	{
		mcFile.Close();
		mcFile.Kill();
		return NULL;
	}

	mcFile.Close();
	mcFile.Kill();
	return pcSerialised;
}

