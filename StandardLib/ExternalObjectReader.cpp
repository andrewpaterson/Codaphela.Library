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
bool CExternalObjectReader::Begin(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExternalObjectReader::End(void)
{
	return true;
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
	bool				bResult;

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

