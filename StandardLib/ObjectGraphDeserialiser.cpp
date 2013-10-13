#include "BaseLib/Log.h"
#include "BaseLib/ErrorTypes.h"
#include "Objects.h"
#include "ObjectDeserialiser.h"
#include "ObjectGraphDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::Init(CObjectReader* pcReader, BOOL bNamedHollows, CObjectAllocator* pcAllocator, CDependentReadObjects* pcDependentReadObjects, CNamedIndexedObjects* pcMemory)
{
	CDependentObjectAdder::Init(pcDependentReadObjects);
	mpcReader = pcReader;
	mpcAllocator = pcAllocator;
	mpcMemory = pcMemory;
	mbNamedHollows = bNamedHollows;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::Kill(void)
{
	mpcMemory = NULL;
	mpcAllocator = NULL;
	mpcDependentObjects = NULL;
	CDependentObjectAdder::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectGraphDeserialiser::Read(char* szObjectName)
{
	BOOL				bResult;
	CBaseObject*		pvObject;
	CSerialisedObject*	pcSerialised;
	
	bResult = mpcReader->Begin();
	if (!bResult)
	{
		return NULL;
	}

	pcSerialised = mpcReader->Read(szObjectName);
	pvObject = ReadSerialsed(pcSerialised);
	if (pvObject == NULL)
	{
		return NULL;
	}

	bResult = ReadDependentObjects();
	if (!bResult)
	{
		return NULL;
	}

	bResult = mpcReader->End();
	if (!bResult)
	{
		return NULL;
	}

	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectGraphDeserialiser::ReadDependentObjects(void)
{
	BOOL					bResult;
	CDependentReadObject*	pcDependent;

	for (;;)
	{
		pcDependent = mpcDependentObjects->GetUnread();
		if (pcDependent)
		{
			if (mbNamedHollows)
			{
				if (pcDependent->IsIndexed())
				{
					bResult = ReadUnread(pcDependent);
					if (!bResult)
					{
						return FALSE;
					}
				}
				else
				{
					MarkRead(pcDependent->GetOldIndex());
				}
			}
			else
			{
				bResult = ReadUnread(pcDependent);
				if (!bResult)
				{
					return FALSE;
				}
			}
		}
		else
		{
			break;
		}
	}

	bResult = AddContainingPointersAndCreateHollowObjects();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectGraphDeserialiser::ReadUnread(CDependentReadObject* pcDependent)
{
	CSerialisedObject*			pcSerialised;
	char*						szObjectName;
	CObjectDeserialiser			cDeserialiser;
	CBaseObject*				pvObject;

	pcSerialised = NULL;
	if (pcDependent->IsNamed())
	{
		szObjectName = pcDependent->GetName();
		pcSerialised = mpcReader->Read(szObjectName);
		if (!pcSerialised)
		{
			return FALSE;
		}
	}
	else
	{
		pcSerialised = mpcReader->Read(pcDependent->GetOldIndex());
		if (!pcSerialised)
		{
			return FALSE;
		}
	}

	if (pcDependent->GetOldIndex() != pcSerialised->GetIndex())
	{
		gcLogger.Error2(__METHOD__, " pcDependent->GetOldIndex [", IndexToString(pcDependent->GetOldIndex()), "] != pcSerialised->GetIndex [", IndexToString(pcSerialised->GetIndex()), "]", NULL);
		return FALSE;
	}

	pvObject = ReadSerialsed(pcSerialised);
	return pvObject != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectGraphDeserialiser::ReadSerialsed(CSerialisedObject* pcSerialised)
{
	CObjectDeserialiser		cDeserialiser;
	CBaseObject*			pvObject;
	OIndex					oiNew;
	OIndex					oiOld;

	oiOld = pcSerialised->GetIndex();
	cDeserialiser.Init(this);
	pvObject = cDeserialiser.Load(pcSerialised);

	cDeserialiser.Kill();
	free(pcSerialised);

	if (pvObject)
	{
		MarkRead(oiOld);

		oiNew = pvObject->GetOI();
		mpcDependentObjects->AddIndexRemap(oiNew, oiOld);
	}

	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::MarkRead(OIndex oi)
{
	mpcDependentObjects->Mark(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectGraphDeserialiser::AddContainingPointersAndCreateHollowObjects(void)
{
	CDependentReadPointer*	pcDependentReadPointer;
	int						i;
	int						iNum;

	iNum = mpcDependentObjects->NumPointers();
	for (i = 0; i < iNum; i++)
	{
		pcDependentReadPointer = mpcDependentObjects->GetPointer(i);
		if (!AddContainingPointersAndCreateHollowObject(pcDependentReadPointer))
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectGraphDeserialiser::AddContainingPointersAndCreateHollowObject(CDependentReadPointer* pcDependentReadPointer)
{
	OIndex					oiNew;
	CBaseObject*			pcBaseObject;
	CDependentReadObject*	pcDependentReadObject;
	CBaseObject*			pvObject;

	oiNew = GetNewIndexFromOld(pcDependentReadPointer->moiPointedTo);
	pcBaseObject = mpcMemory->Get(oiNew);

	if (!pcBaseObject)
	{
		pcDependentReadObject = mpcDependentObjects->GetObject(pcDependentReadPointer->moiPointedTo);
		if (pcDependentReadObject->mcType == OBJECT_POINTER_NAMED)
		{
			pvObject = mpcAllocator->AddHollow(pcDependentReadObject->mszObjectName.Text(), pcDependentReadPointer->miNumEmbedded);
			mpcDependentObjects->AddIndexRemap(pvObject->GetOI(), pcDependentReadPointer->moiPointedTo);
			pcBaseObject = pvObject;
		}
		else if (pcDependentReadObject->mcType == OBJECT_POINTER_ID)
		{
			return FALSE;
		}
	}

	AddContainingPointer(pcBaseObject, pcDependentReadPointer->mppcPointedFrom, pcDependentReadPointer->mpcContaining);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CObjectGraphDeserialiser::GetNewIndexFromOld(OIndex oiOld)
{
	return mpcDependentObjects->GetNewIndexFromOld(oiOld);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectGraphDeserialiser::AllocateObject(CObjectHeader* pcHeader)
{
	if (pcHeader->mcType == OBJECT_POINTER_NULL)
	{
		return NULL;
	}
	else if (pcHeader->mcType == OBJECT_POINTER_ID)
	{
		return mpcAllocator->Add(pcHeader->mszClassName.Text());
	}
	else if (pcHeader->mcType == OBJECT_POINTER_NAMED)
	{
		return mpcAllocator->Add(pcHeader->mszClassName.Text(), pcHeader->mszObjectName.Text());
	}
	else
	{
		gcLogger.Error("Cant allocate object for unknown header type.");
		return NULL;
	}
}

