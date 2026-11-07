#include "BaseLib/Logger.h"
#include "BaseLib/DiskFile.h"
#include "ObjectReaderSimpleDisk.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderSimpleDisk::Init(const char* szDirectory)
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
CSerialisedObject* CObjectReaderSimpleDisk::Read(const char* szObjectName)
{
	CSerialisedObject*	pcSerialised;
	CChars				szDirectory;
	CChars				szFilename;

	Filename(szObjectName, mszFullDirectory.Text(), &szDirectory, &szFilename);
	mcFile.Init(DiskFile(szFilename.Text()));
	szFilename.Kill();
	szDirectory.Kill();

	if (!mcFile.Open(EFM_Read))
	{
		gcLogger.Error2(__METHOD__, " Could not open file [", mcFile.GetFilename(), "] for reading.", NULL);
		mcFile.Kill();
		return NULL;
	}

	pcSerialised = ReadOpenedFile(&mcFile);
	mcFile.Close();
	mcFile.Kill();
	return pcSerialised;
}

