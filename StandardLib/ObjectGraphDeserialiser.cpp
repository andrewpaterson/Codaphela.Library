#include "BaseLib/ErrorTypes.h"
#include "Objects.h"
#include "DependentObjectDeserialiser.h"
#include "ObjectGraphDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::Init(CObjectReader* pcReader)
{
	mpcReader = pcReader;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::Kill(void)
{

}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectGraphDeserialiser::Read(char* szObjectName)
{
	CDependentObjectDeserialiser	cDeserialiser;
	CSerialisedObject*				pcSerialised;
	BOOL							bResult;
	CPointerObject					pObject;

	bResult = mpcReader->Begin();
	if (!bResult)
	{
		return ONull;
	}

	pcSerialised = mpcReader->Read(szObjectName);
	if (!pcSerialised)
	{
		return ONull;
	}

	cDeserialiser.Init(this, pcSerialised);
	pObject = cDeserialiser.Load();
	if (pObject.IsNull())
	{
		cDeserialiser.Kill();
		free(pcSerialised);
		return ONull;
	}
	cDeserialiser.Kill();
	free(pcSerialised);
	return pObject;

	bResult = mpcReader->End();
	if (!bResult)
	{
		return ONull;
	}
}

