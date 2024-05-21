#include "BaseLib/DiskFile.h"
#include "FileObjectReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileObjectReader::Init(CFileBasic* pcFile)
{
	mpcFile = pcFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileObjectReader::Kill(void)
{
	mpcFile = NULL;
	CExternalObjectReader::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CFileObjectReader::Read(OIndex oi)
{
	CChars				szUnnamed;
	CSerialisedObject*	pcSerialised;

	szUnnamed.Init(OBJECT_UNNAMED_FILE"/");
	szUnnamed.AppendHexHiLo(&oi, sizeof(OIndex));

	pcSerialised = Read(szUnnamed.Text());

	szUnnamed.Kill();
	return pcSerialised;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CFileObjectReader::Read(char* szObjectName)
{
	CSerialisedObject*	pcSerialised;
	CChars				szDirectory;
	CChars				szFilename;
	char				szExtension[4];
	bool				bResult;
	int					iFileType;

	szFilename.Kill();
	szDirectory.Kill();

	bResult = mpcFile->Open(EFM_Read);
	if (!bResult)
	{
		return NULL;
	}
	
	//Write file type identifier.
	bResult = mpcFile->ReadData(szExtension, 4);
	if ((!bResult) || (strcmp(szExtension, OBJECT_FILE_EXTENSION) != 0))
	{
		return NULL;
	}

	bResult = mpcFile->ReadInt32(&iFileType);
	if ((!bResult) || (iFileType != BASIC_OBJECT_FILE))
	{
		return NULL;
	}

	pcSerialised = ReadSerialised(mpcFile);
	if (!pcSerialised)
	{
		return NULL;
	}

	mpcFile->Close();
	return pcSerialised;
}


