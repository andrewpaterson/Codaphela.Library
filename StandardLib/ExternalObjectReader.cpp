#include "ObjectIO.h"
#include "ExternalObjectReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalObjectReader::Kill(void)
{
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CExternalObjectReader::Begin(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CExternalObjectReader::End(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CExternalObjectReader::ReadSerialised(CFileReader* pcReader)
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

