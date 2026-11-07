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

	pcSerialised = Read(Unnamed(oi, &szUnnamed));
	szUnnamed.Kill();
	return pcSerialised;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CFileObjectReader::Read(const char* szObjectName)
{
	CSerialisedObject*	pcSerialised;

	//The file holds one object: szObjectName isn't needed to find it.
	if (!mpcFile->Open(EFM_Read))
	{
		return NULL;
	}
	pcSerialised = ReadOpenedFile(mpcFile);
	mpcFile->Close();
	return pcSerialised;
}


//////////////////////////////////////////////////////////////////////////
//
// The serialised object in an opened object file (the file type, then
// the object); NULL if pcFile isn't one.
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CFileObjectReader::ReadOpenedFile(CFileBasic* pcFile)
{
	char	szExtension[4];
	int32	iFileType;

	if (!pcFile->ReadData(szExtension, 4) || (memcmp(szExtension, OBJECT_FILE_EXTENSION, 4) != 0))
	{
		return NULL;
	}
	if (!pcFile->ReadInt32(&iFileType) || (iFileType != BASIC_OBJECT_FILE))
	{
		return NULL;
	}
	return ReadSerialised(pcFile);
}


