#include "BaseLib/DiskFile.h"
#include "ObjectReaderSimple.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderSimple::Init(char* szDirectory)
{
	CObjectReader::Init(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderSimple::Kill(void)
{
	CObjectReader::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectReaderSimple::Begin(void)
{
	return CObjectReader::Begin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CObjectReaderSimple::Read(char* szObjectName)
{
	CSerialisedObject*	pcSerialised;
	CChars				szDirectory;
	CChars				szFileName;
	CFileBasic			cFile;
	char				szExtension[4];
	BOOL				bResult;
	int					iFileType;

	FileName(szObjectName, mszFullDirectory.Text(), &szDirectory, &szFileName);

	cFile.Init(DiskFile(szFileName.Text()));
	szFileName.Kill();
	szDirectory.Kill();

	bResult = cFile.Open(EFM_Read);
	if (!bResult)
	{
		return NULL;
	}
	
	//Write file type identifier.
	bResult = cFile.ReadData(szExtension, 4);
	if ((!bResult) || (strcmp(szExtension, OBJECT_FILE_EXTENSION) != 0))
	{
		return NULL;
	}

	bResult = cFile.ReadInt(&iFileType);
	if ((!bResult) || (iFileType != BASIC_OBJECT_FILE))
	{
		return NULL;
	}

	pcSerialised = ReadSerialised(&cFile);
	if (!pcSerialised)
	{
		return NULL;
	}

	cFile.Close();
	cFile.Kill();
	return pcSerialised;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectReaderSimple::End(void)
{
	return CObjectReader::End();
}

