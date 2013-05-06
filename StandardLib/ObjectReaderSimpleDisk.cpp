#include "BaseLib/DiskFile.h"
#include "ObjectReaderSimpleDisk.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderSimpleDisk::Init(char* szDirectory)
{
	CObjectReaderSimple::Init(&mcFile);
	mszFullDirectory.Init(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderSimpleDisk::Kill(void)
{
	mszFullDirectory.Kill();
	CObjectReaderSimple::Kill();
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
		return NULL;
	}

	//Write file type identifier.
	bResult = mcFile.ReadData(szExtension, 4);
	if ((!bResult) || (strcmp(szExtension, OBJECT_FILE_EXTENSION) != 0))
	{
		return NULL;
	}

	bResult = mcFile.ReadInt(&iFileType);
	if ((!bResult) || (iFileType != BASIC_OBJECT_FILE))
	{
		return NULL;
	}

	pcSerialised = ReadSerialised(&mcFile);
	if (!pcSerialised)
	{
		return NULL;
	}

	mcFile.Close();
	mcFile.Kill();
	return pcSerialised;
}
