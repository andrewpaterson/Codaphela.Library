#include "BaseLib/Logger.h"
#include "BaseLib/ErrorTypes.h"
#include "Objects.h"
#include "ObjectDeserialiser.h"
#include "ObjectGraphDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::Init(CObjectReader* pcReader, CIndexGenerator* pcIndexGenerator, CObjectAllocator* pcAllocator)
{
	mpcReader = pcReader;
	mcDependentObjects.Init();
	mcIndexRemap.Init(32);
	mpcAllocator = pcAllocator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::Kill(void)
{
	mpcAllocator = NULL;
	mcIndexRemap.Kill();
	mcDependentObjects.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//CPointerObject CObjectGraphDeserialiser::Read(OIndex oi)
//{
//	CPointerHeader					cHeader;
//	BOOL							bResult;
//	CBaseObject*					pcObjectPtr;
//
//	CPointerObject					pObject;
//
//	pcObjectPtr = NULL;
//	cHeader.Init(oi);
//	AddDependent(&cHeader, &pcObjectPtr);
//
//	bResult = ReadAfterAddDependent();
//	if (!bResult)
//	{
//		return ONull;
//	}
//
//	pObject = pcObjectPtr;
//	return pObject;
//}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectGraphDeserialiser::Read(char* szObjectName)
{
	CPointerHeader					cHeader;
	BOOL							bResult;
	CBaseObject*					pcObjectPtr;

	CPointerObject					pObject;

	pcObjectPtr = NULL;
	cHeader.Init(szObjectName);
	AddDependent(&cHeader, &pcObjectPtr, NULL);

	bResult = ReadAfterAddDependent();
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
BOOL CObjectGraphDeserialiser::ReadAfterAddDependent(void)
{
	BOOL							bResult;
	CPointerObject					pObject;
	CDependentReadObject*			pcDependent;
	BOOL							bFirst;

	bResult = mpcReader->Begin();
	if (!bResult)
	{
		return FALSE;
	}

	bFirst = TRUE;
	for (;;)
	{
		pcDependent = mcDependentObjects.GetUnread();
		if (pcDependent)
		{
			bResult = ReadUnread(pcDependent, bFirst);
			if (!bResult)
			{
				return FALSE;
			}
			bFirst = FALSE;
		}
		else
		{
			break;
		}
	}

	bResult = mpcReader->End();
	if (!bResult)
	{
		return FALSE;
	}

	bResult = FixPointers();
	if (!bResult)
	{
		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectGraphDeserialiser::ReadUnread(CDependentReadObject* pcDependent, BOOL bFirst)
{
	CSerialisedObject*			pcSerialised;
	char*						szObjectName;
	CObjectDeserialiser			cDeserialiser;
	CPointerObject				pObject;
	OIndex						oiOld;
	OIndex						oiNew;

	pcSerialised = NULL;
	if (pcDependent->IsNamed())
	{
		szObjectName = pcDependent->GetName();
		pcSerialised = mpcReader->Read(szObjectName);
		if (!pcSerialised)
		{
			return FALSE;
		}

		if (bFirst)
		{
			mcDependentObjects.SetInitialIndex(pcSerialised->GetIndex());
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

	oiOld = pcDependent->moi;
	cDeserialiser.Init(this);
	pObject = cDeserialiser.Load(pcSerialised);
	if (pObject.IsNull())
	{
		cDeserialiser.Kill();
		free(pcSerialised);
		return FALSE;
	}
	pcDependent = NULL;

	cDeserialiser.Kill();
	free(pcSerialised);
	MarkRead(oiOld);

	oiNew = pObject.GetIndex();
	AddIndexRemap(oiNew, oiOld);

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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectGraphDeserialiser::FixPointers(void)
{
	CDependentReadPointer*	pcReadPointer;
	int						i;
	int						iNum;
	CBaseObject*			pcBaseObject;
	OIndex					oiNew;

	iNum = mcDependentObjects.NumPointers();
	for (i = 0; i < iNum; i++)
	{
		pcReadPointer = mcDependentObjects.GetPointer(i);
		oiNew = GetNewIndexFromOld(pcReadPointer->moiPointedTo);

		pcBaseObject = gcObjects.GetInMemoryObject(oiNew);
		if (pcBaseObject)
		{
			FixPointer(pcBaseObject, pcReadPointer->mppcPointedFrom, pcReadPointer->mpcContaining);
		}
		else
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
OIndex CObjectGraphDeserialiser::GetNewIndexFromOld(OIndex oiOld)
{
	int				i;
	CIndexNewOld*	pcRemap;

	for (i = 0; i < mcIndexRemap.NumElements(); i++)
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
void CObjectGraphDeserialiser::AddIndexRemap(OIndex oiNew, OIndex oiOld)
{
	CIndexNewOld*	pcNewOld;

	pcNewOld = mcIndexRemap.Add();
	pcNewOld->Init(oiNew, oiOld);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectGraphDeserialiser::AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated)
{
	if ((pcHeader->mcType == OBJECT_POINTER_NAMED) || (pcHeader->mcType == OBJECT_POINTER_ID))
	{
		mcDependentObjects.Add(pcHeader, ppcPtrToBeUpdated, pcObjectContainingPtrToBeUpdated);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectGraphDeserialiser::AllocateObject(CObjectHeader* pcHeader)
{
	if (pcHeader->mcType == OBJECT_POINTER_NULL)
	{
		return ONull;
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
		return ONull;
	}
}

