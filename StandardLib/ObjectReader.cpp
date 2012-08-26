#include "ObjectIO.h"
#include "ObjectReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReader::Init(char* szFullDirectory)
{
	mszFullDirectory.Init(szFullDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReader::Kill(void)
{
	mszFullDirectory.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectReader::Begin(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectReader::End(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CObjectReader::ReadSerialised(CFileReader* pcReader)
{
	int					iLength;
	CSerialisedObject*	pcSerialised;
	void*				pvOffset;
	BOOL				bResult;

	if (!pcReader->ReadInt(&iLength))
	{
		return NULL;
	}
	if (iLength <= 0)
	{
		return NULL;
	}

	pcSerialised = (CSerialisedObject*)malloc(iLength);
	if (!pcSerialised)
	{
		return NULL;
	}

	pcSerialised->SetLength(iLength);
	iLength -= sizeof(int);

	pvOffset = RemapSinglePointer(pcSerialised, sizeof(int));
	bResult = pcReader->ReadData(pvOffset, iLength);
	if (!bResult)
	{
		free(pcSerialised);
		return NULL;
	}
	return pcSerialised;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CObjectReader::Read(OIndex oi)
{
	CChars				szUnnamed;
	CSerialisedObject*	pcSerialised;

	szUnnamed.Init(OBJECT_UNNAMED_FILE"/");
	szUnnamed.AppendHexHiLo(&oi, sizeof(OIndex));

	pcSerialised = Read(szUnnamed.Text());

	szUnnamed.Kill();
	return pcSerialised;
}

