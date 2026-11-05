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
	mcNewIndices.Init(true);
	mcReadObjects.Init();
	mcReadObjectPositions.Init(false);
	maoiUnread.Init();
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
	mcNewIndices.Kill();
	mcPointers.Kill();

	uiReadObjects = mcReadObjects.NumElements();
	for (i = 0; i < uiReadObjects; i++)
	{
		pcDependent = mcReadObjects.Get(i);
		pcDependent->Kill();
	}
	mcReadObjects.Kill();
	mcReadObjectPositions.Kill();
	maoiUnread.Kill();

	CDependentReadObjects::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Reads szObjectName and everything it points to.  The reader is ended
// whether or not that worked.
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CExternalObjectDeserialiser::Read(const char* szObjectName)
{
	CBaseObject*	pvObject;

	if (!mpcReader->Begin())
	{
		return NULL;
	}

	pvObject = ReadObjectAndDependents(szObjectName);
	if (!mpcReader->End())
	{
		return NULL;
	}
	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CExternalObjectDeserialiser::ReadObjectAndDependents(const char* szObjectName)
{
	CBaseObject*		pvObject;
	CSerialisedObject*	pcSerialised;

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

	if (!ReadDependentObjects())
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
	OIndex				oiOld;

	pcSerialised = NULL;
	oiOld = pcDependent->GetOldIndex();
	if (pcDependent->IsNamed())
	{
		szObjectName = pcDependent->GetName();
		pcSerialised = mpcReader->Read(szObjectName);
		if (!pcSerialised)
		{
			return gcLogger.Error2(__METHOD__, " Could not read object named [", szObjectName, "] that another object points to.", NULL);
		}
	}
	else
	{
		pcSerialised = mpcReader->Read(oiOld);
		if (!pcSerialised)
		{
			return gcLogger.Error2(__METHOD__, " Could not read object with index [", IndexToString(oiOld), "] that another object points to.", NULL);
		}
	}

	if (oiOld != pcSerialised->GetIndex())
	{
		gcLogger.Error2(__METHOD__, " pcDependent->GetOldIndex [", IndexToString(oiOld), "] != pcSerialised->GetIndex [", IndexToString(pcSerialised->GetIndex()), "]", NULL);
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
	lsize			uiLength;

	oiOld = pcSerialised->GetIndex();

	uiLength = pcSerialised->GetLength();
	cMemoryFile.Init(pcSerialised, uiLength);
	cMemoryFile.Open(EFM_Read);
	cReader.Init(&cMemoryFile, this);

	pvObject = cReader.Read();

	cMemoryFile.Close();
	cReader.Kill();
	cMemoryFile.Kill();

	if (pvObject)
	{
		//Read objects are new to the database (the object reader marks them clean, as if loaded from it).
		pvObject->SetDirty(true);
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
// The object found with (old) index oi, NULL if none was.
//
//////////////////////////////////////////////////////////////////////////
CDependentReadObject* CExternalObjectDeserialiser::GetObject(OIndex oi)
{
	int64*	piPosition;

	piPosition = mcReadObjectPositions.Get(oi);
	if (piPosition == NULL)
	{
		return NULL;
	}
	return mcReadObjects.Get((size)*piPosition);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CExternalObjectDeserialiser::NumPointers(void)
{
	return mcPointers.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDependentReadPointer* CExternalObjectDeserialiser::GetPointer(size iIndex)
{
	return mcPointers.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CExternalObjectDeserialiser::NumObjects(void)
{
	return mcReadObjects.NumElements();
}



//////////////////////////////////////////////////////////////////////////
//
// The index the object read with (old) index oiOld was given,
// INVALID_O_INDEX if it hasn't been read.
//
//////////////////////////////////////////////////////////////////////////
OIndex CExternalObjectDeserialiser::GetNewIndexFromOld(OIndex oiOld)
{
	int64*	poiNew;

	poiNew = mcNewIndices.Get(oiOld);
	if (poiNew == NULL)
	{
		return INVALID_O_INDEX;
	}
	return *poiNew;
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
	mcNewIndices.Put(oiOld, oiNew);
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
	size					i;
	size					iNum;

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
// Points the pointer that was read at the object it pointed to, now it has
// been read (or, for a named object not read, a hollow for it).
//
//////////////////////////////////////////////////////////////////////////
bool CExternalObjectDeserialiser::AddHeapFromPointersAndCreateHollowObject(CDependentReadPointer* pcDependentReadPointer)
{
	OIndex					oiOld;
	OIndex					oiNew;
	CBaseObject*			pcBaseObject;
	CDependentReadObject*	pcDependentReadObject;
	CEmbeddedObject*		pcEmbeddedObject;

	oiOld = pcDependentReadPointer->moiPointedTo;
	oiNew = GetNewIndexFromOld(oiOld);
	pcBaseObject = NULL;
	if (oiNew != INVALID_O_INDEX)
	{
		pcBaseObject = mpcMemory->Get(oiNew);
	}

	if (pcBaseObject == NULL)
	{
		pcDependentReadObject = GetObject(oiOld);
		if (pcDependentReadObject == NULL)
		{
			return gcLogger.Error2(__METHOD__, " Pointer to object with index [", IndexToString(oiOld), "] that was never found.", NULL);
		}
		if (pcDependentReadObject->meType != OBJECT_POINTER_NAMED)
		{
			return gcLogger.Error2(__METHOD__, " Object with index [", IndexToString(oiOld), "] was not read.", NULL);
		}
		pcBaseObject = mpcObjects->GetNamedOrAllocateHollow(pcDependentReadObject->mszObjectName.Text(), pcDependentReadPointer->miNumEmbedded);
		if (pcBaseObject == NULL)
		{
			return gcLogger.Error2(__METHOD__, " Could not get or allocate object named [", pcDependentReadObject->mszObjectName.Text(), "].", NULL);
		}
		AddIndexRemap(pcBaseObject->GetIndex(), oiOld);
	}

	//The pointer may be to one of the object's embedded objects (or, if it is hollow, its embedded hollow).
	pcEmbeddedObject = pcBaseObject->GetEmbeddedObject(pcDependentReadPointer->miEmbeddedIndex);
	if (pcEmbeddedObject == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Object {", ObjectToString(pcBaseObject), "} has no embedded object [", IntToString(pcDependentReadPointer->miEmbeddedIndex), "].", NULL);
	}
	*pcDependentReadPointer->mppcPointedFrom = pcEmbeddedObject;
	AddHeapFrom(pcEmbeddedObject, pcDependentReadPointer->mpcContaining);
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

	if (pcHeader->meType == OBJECT_POINTER_NULL)
	{
		return NULL;
	}
	else
	{
		szClassName = pcHeader->mszClassName.Text();
		if (pcHeader->meType == OBJECT_POINTER_ID)
		{
			return mpcObjects->AllocateUninitialisedByClassNameAndAddIntoMemory(szClassName, "");
		}
		else if (pcHeader->meType == OBJECT_POINTER_NAMED)
		{
			szName = pcHeader->mszObjectName.Text();
			if (!StrEmpty(szName))
			{
				return mpcObjects->AllocateReplacingNamed(szClassName, szName);
			}
			else
			{
				return mpcObjects->AllocateUninitialisedByClassNameAndAddIntoMemory(szClassName, "");
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
bool CExternalObjectDeserialiser::AddDependent(CObjectIdentifier* pcObjectIdentifier, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, uint16 iNumEmbedded, uint16 iEmbeddedIndex)
{
	CDependentReadObject	cDependent;
	CDependentReadPointer*	pcPointer;
	bool					bNameExistsInDatabase;

	if (!((pcObjectIdentifier->meType == OBJECT_POINTER_NAMED) || (pcObjectIdentifier->meType == OBJECT_POINTER_ID)))
	{
		if (pcObjectIdentifier->meType == OBJECT_POINTER_NULL)
		{
			return true;
		}
		else
		{
			return gcLogger.Error2(__METHOD__, " Cannot add depenent with unknown type [", IntToString(pcObjectIdentifier->meType, 16), "].", NULL);
		}
	}

	cDependent.Init(pcObjectIdentifier);

	if (!mcReadObjectPositions.HasKey(pcObjectIdentifier->moi))
	{
		if (pcObjectIdentifier->IsNamed())
		{
			bNameExistsInDatabase = mpcObjects->Contains(pcObjectIdentifier->mszObjectName.Text());
			if (bNameExistsInDatabase)
			{
				cDependent.SetExisting();
			}
		}

		mcReadObjectPositions.Put(pcObjectIdentifier->moi, (int64)mcReadObjects.NumElements());
		PushUnread(pcObjectIdentifier->moi);
		mcReadObjects.Add(&cDependent);
	}
	else
	{
		cDependent.Kill();
	}

	pcPointer = mcPointers.Add();
	pcPointer->Init(ppcPtrToBeUpdated, pcObjectContainingPtrToBeUpdated, pcObjectIdentifier->moi, iNumEmbedded, iEmbeddedIndex);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExternalObjectDeserialiser::AddReverseDependent(CObjectIdentifier* pcObjectIdentifier, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingHeapFrom, uint16 iNumEmbedded, uint16 iEmbeddedIndex)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The unread object with the smallest old index, NULL if all are read: the
// objects read are given new indices in the order of their old ones.
//
//////////////////////////////////////////////////////////////////////////
CDependentReadObject* CExternalObjectDeserialiser::GetUnread(void)
{
	CDependentReadObject*	pcDependent;

	while (maoiUnread.NumElements() > 0)
	{
		pcDependent = GetObject(PopUnread());
		if (!pcDependent->IsRead())
		{
			return pcDependent;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalObjectDeserialiser::PushUnread(OIndex oi)
{
	size	i;
	size	uiParent;
	OIndex*	paoi;

	maoiUnread.Push(&oi);
	paoi = maoiUnread.GetData();
	i = maoiUnread.NumElements() - 1;
	while (i > 0)
	{
		uiParent = (i - 1) / 2;
		if (paoi[uiParent] <= oi)
		{
			break;
		}
		paoi[i] = paoi[uiParent];
		i = uiParent;
	}
	paoi[i] = oi;
}


//////////////////////////////////////////////////////////////////////////
//
// The smallest old index.  There must be one.
//
//////////////////////////////////////////////////////////////////////////
OIndex CExternalObjectDeserialiser::PopUnread(void)
{
	size	i;
	size	uiChild;
	size	uiNum;
	OIndex*	paoi;
	OIndex	oiSmallest;
	OIndex	oiLast;

	oiSmallest = *maoiUnread.Get(0);
	maoiUnread.Pop(&oiLast);
	uiNum = maoiUnread.NumElements();
	paoi = maoiUnread.GetData();  //After the pop: it can move the array.

	//The last goes where the smallest was and moves down past smaller children.
	i = 0;
	for (;;)
	{
		uiChild = i * 2 + 1;
		if (uiChild >= uiNum)
		{
			break;
		}
		if ((uiChild + 1 < uiNum) && (paoi[uiChild + 1] < paoi[uiChild]))
		{
			uiChild++;
		}
		if (oiLast <= paoi[uiChild])
		{
			break;
		}
		paoi[i] = paoi[uiChild];
		i = uiChild;
	}
	if (uiNum > 0)
	{
		paoi[i] = oiLast;
	}
	return oiSmallest;
}


