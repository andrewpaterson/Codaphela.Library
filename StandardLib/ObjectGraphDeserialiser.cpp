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
	mcDependentObjects.Init();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::Kill(void)
{
	mcDependentObjects.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectGraphDeserialiser::Read(char* szObjectName)
{
	BOOL							bResult;
	CPointerObject					pObject;
	CDependentReadObject*			pcDependent;
	CPointerHeader					cHeader;
	CBaseObject*					pcObjectPtr;
	BOOL							bFirst;

	bResult = mpcReader->Begin();
	if (!bResult)
	{
		return ONull;
	}

	pcObjectPtr = NULL;
	cHeader.Init(szObjectName);
	mcDependentObjects.Add(&cHeader, &pcObjectPtr);

	bFirst = TRUE;
	for (;;)
	{
		pcDependent = mcDependentObjects.GetUnread();
		if (pcDependent)
		{
			bResult = ReadUnread(pcDependent, bFirst);
			if (!bResult)
			{
				cHeader.Kill();
				return ONull;
			}
			bFirst = FALSE;
		}
		else
		{
			break;
		}
	}
	cHeader.Kill();

	bResult = mpcReader->End();
	if (!bResult)
	{
		return ONull;
	}

	pObject = pcObjectPtr;
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr)
{
	mcDependentObjects.Add(pcHeader, ppcObjectPtr);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectGraphDeserialiser::ReadUnread(CDependentReadObject* pcDependent, BOOL bFirst)
{
	CSerialisedObject*				pcSerialised;
	char*							szObjectName;
	CDependentObjectDeserialiser	cDeserialiser;
	OIndex							oi;
	CPointerObject					pObject;

	pcSerialised = NULL;
	if (pcDependent->IsNamed())
	{
		szObjectName = pcDependent->GetName();
		pcSerialised = mpcReader->Read(szObjectName);

		if (bFirst)
		{
			mcDependentObjects.SetInitialIndex(pcSerialised->GetIndex());
		}
	}
	else
	{
		oi = pcDependent->GetIndex();
		pcSerialised = mpcReader->Read(oi);
	}

	if (!pcSerialised)
	{
		return FALSE;
	}

	cDeserialiser.Init(this, pcSerialised);
	pObject = cDeserialiser.Load();
	if (pObject.IsNull())
	{
		cDeserialiser.Kill();
		free(pcSerialised);
		return FALSE;
	}
	pcDependent = NULL;

	cDeserialiser.Kill();
	free(pcSerialised);
	MarkRead(pObject->GetOI());
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::MarkRead(OIndex oi)
{
	mcDependentObjects.Mark(oi);
}

