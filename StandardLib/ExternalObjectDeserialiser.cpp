#include "BaseLib/Logger.h"
#include "BaseLib/ErrorTypes.h"
#include "Objects.h"
#include "ObjectReader.h"
#include "ExternalObjectDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalObjectDeserialiser::Init(CExternalObjectReader* pcReader, bool bNamedHollows, CObjects* pcObjects)
{
	CDependentReadObjects::Init();
	mcIndexRemap.Init();
	miGetIndex = 0;
	mcReadObjects.Init();
	mcPointers.Init();
	mpcReader = pcReader;
	mpcObjects = pcObjects;
	mpcMemory = pcObjects->GetMemory();
	mbNamedHollows = bNamedHollows;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalObjectDeserialiser::Kill(void)
{
	size					i;
	CDependentReadObject*	pcDependent;
	size					uiReadObjects;

	mpcObjects = NULL;
	mpcMemory = NULL;

	mcIndexRemap.Kill();
	mcPointers.Kill();

	uiReadObjects = mcReadObjects.NumElements();
	for (i = 0; i < uiReadObjects; i++)
	{
		pcDependent = mcReadObjects.Get(i);
		pcDependent->Kill();
	}
	mcReadObjects.Kill();

	CDependentReadObjects::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CExternalObjectDeserialiser::Read(char* szObjectName)
{
	bool				bResult;
	CBaseObject*		pvObject;
	CSerialisedObject*	pcSerialised;
	
	bResult = mpcReader->Begin();
	if (!bResult)
	{
		return NULL;
	}

	pcSerialised = mpcReader->Read(szObjectName);
	if (!pcSerialised)
	{
		return NULL;
	}
	pvObject = ReadSerialsed(pcSerialised);
	SafeFree(pcSerialised);

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
bool CExternalObjectDeserialiser::ReadDependentObjects(void)
{
	bool					bResult;
	CDependentReadObject*	pcDependent;
	int						iCount;  //Just for debugging.

	for (iCount = 0;; iCount++)
	{
		pcDependent = GetUnread();
		if (pcDependent)
		{
			if (mbNamedHollows)
			{
				if (pcDependent->IsIndexed())
				{
					bResult = ReadUnread(pcDependent);
					if (!bResult)
					{
						return false;
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
					return false;
				}
			}
		}
		else
		{
			break;
		}
	}

	bResult = AddHeapFromPointersAndCreateHollowObjects();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExternalObjectDeserialiser::ReadUnread(CDependentReadObject* pcDependent)
{
	CSerialisedObject*	pcSerialised;
	char*				szObjectName;
	CObjectReader		cDeserialiser;
	CBaseObject*		pvObject;

	pcSerialised = NULL;
	if (pcDependent->IsNamed())
	{
		szObjectName = pcDependent->GetName();
		pcSerialised = mpcReader->Read(szObjectName);
		if (!pcSerialised)
		{
			return false;
		}
	}
	else
	{
		pcSerialised = mpcReader->Read(pcDependent->GetOldIndex());
		if (!pcSerialised)
		{
			return false;
		}
	}

	if (pcDependent->GetOldIndex() != pcSerialised->GetIndex())
	{
		gcLogger.Error2(__METHOD__, " pcDependent->GetOldIndex [", IndexToString(pcDependent->GetOldIndex()), "] != pcSerialised->GetIndex [", IndexToString(pcSerialised->GetIndex()), "]", NULL);
		return false;
	}

	pvObject = ReadSerialsed(pcSerialised);
	SafeFree(pcSerialised);

	return pvObject != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CExternalObjectDeserialiser::ReadSerialsed(CSerialisedObject* pcSerialised)
{
	CObjectReader	cReader;
	CBaseObject*	pvObject;
	OIndex			oiNew;
	OIndex			oiOld;
	CMemoryFile		cMemoryFile;

	oiOld = pcSerialised->GetIndex();

	cMemoryFile.Init(pcSerialised, pcSerialised->GetLength());
	cMemoryFile.Open(EFM_Read);
	cReader.Init(&cMemoryFile, this);

	pvObject = cReader.Read();

	cMemoryFile.Close();
	cReader.Kill();
	cMemoryFile.Kill();

	if (pvObject)
	{
		MarkRead(oiOld);

		oiNew = pvObject->GetIndex();
		AddIndexRemap(oiNew, oiOld);
	}

	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalObjectDeserialiser::MarkRead(OIndex oi)
{
	Mark(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExternalObjectDeserialiser::Mark(OIndex oi)
{
	CDependentReadObject* pcDependent;

	pcDependent = GetObject(oi);
	if (!pcDependent)
	{
		return false;
	}

	pcDependent->SetRead();
	return true;
}
 
 
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDependentReadObject* CExternalObjectDeserialiser::GetObject(OIndex oi)
{
	CDependentReadObject	cObject;
	size					iIndex;
	bool					bResult;
	CDependentReadObject*	pcDependent;

	cObject.moi = oi;

	bResult = mcReadObjects.FindInSorted(&cObject, &CompareDependentReadObject, &iIndex);
	if (!bResult)
	{
		return NULL;
	}
	pcDependent = mcReadObjects.Get(iIndex);
	return pcDependent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CExternalObjectDeserialiser::NumPointers(void)
{
	return mcPointers.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDependentReadPointer* CExternalObjectDeserialiser::GetPointer(int iIndex)
{
	return mcPointers.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CExternalObjectDeserialiser::NumObjects(void)
{
	return mcReadObjects.NumElements();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CExternalObjectDeserialiser::GetNewIndexFromOld(OIndex oiOld)
{
	size			i;
	CIndexNewOld*	pcRemap;
	size			uiIndices;

	uiIndices = mcIndexRemap.NumElements();
	for (i = 0; i < uiIndices; i++)
	{
		pcRemap = mcIndexRemap.Get(i);
		if (pcRemap->moiOld == oiOld)
		{
			return pcRemap->moiNew;
		}
	}
	return INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalObjectDeserialiser::AddIndexRemap(OIndex oiNew, OIndex oiOld)
{
	CIndexNewOld* pcNewOld;

	pcNewOld = mcIndexRemap.Add();
	pcNewOld->Init(oiNew, oiOld);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayIndexNewOld* CExternalObjectDeserialiser::GetArrayIndexNewOld(void)
{
	return &mcIndexRemap;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExternalObjectDeserialiser::AddHeapFromPointersAndCreateHollowObjects(void)
{
	CDependentReadPointer*	pcDependentReadPointer;
	int						i;
	int						iNum;

	iNum = NumPointers();
	for (i = 0; i < iNum; i++)
	{
		pcDependentReadPointer = GetPointer(i);
		if (!AddHeapFromPointersAndCreateHollowObject(pcDependentReadPointer))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExternalObjectDeserialiser::AddHeapFromPointersAndCreateHollowObject(CDependentReadPointer* pcDependentReadPointer)
{
	OIndex					oiNew;
	CBaseObject*			pcBaseObject;
	CDependentReadObject*	pcDependentReadObject;
	CBaseObject*			pvObject;

	oiNew = GetNewIndexFromOld(pcDependentReadPointer->moiPointedTo);
	pcBaseObject = mpcMemory->Get(oiNew);

	if (!pcBaseObject)
	{
		pcDependentReadObject = GetObject(pcDependentReadPointer->moiPointedTo);
		if (pcDependentReadObject->mcType == OBJECT_POINTER_NAMED)
		{
			//Should CExternalObjectDeserialiser be calling GetNamedObjectInMemoryOrAllocateHollow?  It's more an CInternalObjectDeserialiser thing.
			pvObject = mpcObjects->GetNamedObjectInMemoryOrAllocateHollow(pcDependentReadObject->mszObjectName.Text(), pcDependentReadPointer->miNumEmbedded);
			AddIndexRemap(pvObject->GetIndex(), pcDependentReadPointer->moiPointedTo);
			pcBaseObject = pvObject;
		}
		else if (pcDependentReadObject->mcType == OBJECT_POINTER_ID)
		{
			return false;
		}
	}

	*pcDependentReadPointer->mppcPointedFrom = pcBaseObject;
	AddHeapFrom(pcBaseObject, pcDependentReadPointer->mpcContaining);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CExternalObjectDeserialiser::AllocateForDeserialisation(CObjectHeader* pcHeader)
{
	char*	szName;
	char*	szClassName;

	if (pcHeader->mcType == OBJECT_POINTER_NULL)
	{
		return NULL;
	}
	else
	{
		szClassName = pcHeader->mszClassName.Text();
		if (pcHeader->mcType == OBJECT_POINTER_ID)
		{
			return mpcObjects->AllocateUninitialisedByClassNameAndAddIntoMemory(szClassName);
		}
		else if (pcHeader->mcType == OBJECT_POINTER_NAMED)
		{
			szName = pcHeader->mszObjectName.Text();
			if (!StrEmpty(szName))
			{
				return mpcObjects->GetNamedObjectInMemoryAndReplaceOrAllocateUninitialisedWithSameName(szClassName, szName);
			}
			else
			{
				return mpcObjects->AllocateUninitialisedByClassNameAndAddIntoMemory(szClassName);
			}
		}
	}
	gcLogger.Error("Cant allocate object for unknown header type.");
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExternalObjectDeserialiser::AddDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, uint16 iNumEmbedded, uint16 iEmbeddedIndex)
{
	CDependentReadObject	cDependent;
	bool					bOiExistsInDependents;
	size					iIndex;
	CDependentReadPointer*	pcPointer;
	bool					bNameExistsInDatabase;

	if (!((pcHeader->mcType == OBJECT_POINTER_NAMED) || (pcHeader->mcType == OBJECT_POINTER_ID)))
	{
		return true;
	}

	cDependent.Init(pcHeader);

	bOiExistsInDependents = mcReadObjects.FindInSorted(&cDependent, &CompareDependentReadObject, &iIndex);
	if (!bOiExistsInDependents)
	{
		if (pcHeader->IsNamed())
		{
			bNameExistsInDatabase = mpcObjects->Contains(pcHeader->mszObjectName.Text());
			if (bNameExistsInDatabase)
			{
				cDependent.SetExisting();
			}
		}

		mcReadObjects.InsertAt(&cDependent, iIndex);
	}
	else
	{
		cDependent.Kill();
	}

	pcPointer = mcPointers.Add();
	pcPointer->Init(ppcPtrToBeUpdated, pcObjectContainingPtrToBeUpdated, pcHeader->moi, iNumEmbedded, iEmbeddedIndex);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExternalObjectDeserialiser::AddReverseDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingHeapFrom, uint16 iNumEmbedded, uint16 iEmbeddedIndex, int iDistToRoot)
{
	return true;
}


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
CDependentReadObject* CExternalObjectDeserialiser::GetUnread(void)
{
	size					iOldIndex;
	CDependentReadObject*	psObject;

	if (mcReadObjects.NumElements() == 0)
	{
		return NULL;
	}

	iOldIndex = miGetIndex;
	for (;;)
	{
		if (miGetIndex >= mcReadObjects.NumElements() - 1)
		{
			miGetIndex = 0;
		}
		else
		{
			miGetIndex++;
		}

		psObject = mcReadObjects.Get(miGetIndex);
		if (!psObject->IsRead())
		{
			return psObject;
		}

		if (miGetIndex == iOldIndex)
		{
			return NULL;
		}
	}
}

