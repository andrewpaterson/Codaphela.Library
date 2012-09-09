#include "BaseLib/ErrorTypes.h"
#include "Objects.h"
#include "ObjectDeserialiser.h"
#include "ObjectGraphDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::Init(CObjectReader* pcReader, CIndexGenerator* pcIndexGenerator)
{
	mpcReader = pcReader;
	mcDependentObjects.Init(pcIndexGenerator);
	mcIndexRemap.Init(32);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::Kill(void)
{
	mcIndexRemap.Kill();
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
	ForceAddDependent(&cHeader, &pcObjectPtr);

	bFirst = TRUE;
	for (;;)
	{
		pcDependent = mcDependentObjects.GetUnread();
		if (pcDependent)
		{
			bResult = ReadUnread(pcDependent, bFirst);
			if (!bResult)
			{
				return ONull;
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
		return ONull;
	}

	bResult = FixPointers();
	if (!bResult)
	{
		return ONull;
	}

	bResult = FixExisting();
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
BOOL CObjectGraphDeserialiser::ReadUnread(CDependentReadObject* pcDependent, BOOL bFirst)
{
	CSerialisedObject*		pcSerialised;
	char*					szObjectName;
	CObjectDeserialiser		cDeserialiser;
	CPointerObject			pObject;
	OIndex					oiNew;
	OIndex					oiOld;

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
	oiNew = pcDependent->GetNewIndex();
	cDeserialiser.Init(this, pcSerialised);
	pObject = cDeserialiser.Load(oiNew);
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
	CBaseObject*			pcContaining;

	iNum = mcDependentObjects.NumPointers();
	for (i = 0; i < iNum; i++)
	{
		pcReadPointer = mcDependentObjects.GetPointer(i);
		pcBaseObject = gcObjects.GetBaseObject(pcReadPointer->moiPointedTo);
		if (pcBaseObject)
		{
			*pcReadPointer->mppcPointedFrom = pcBaseObject;

			pcContaining = pcReadPointer->mpcContaining;
			if (pcContaining)
			{
				pcBaseObject->AddFrom(pcContaining);
			}
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
BOOL CObjectGraphDeserialiser::FixExisting(void)
{
	CDependentReadObject*	pcReadObject;
	int						i;
	int						iNum;
	CPointerObject			pNewObject;
	CPointerObject			pOldObject;
	OIndex					oiOld;

	iNum = mcDependentObjects.NumObjects();
	for (i = 0; i < iNum; i++)
	{
		pcReadObject = mcDependentObjects.GetObject(i);
		if (pcReadObject->PreExisted())
		{
			oiOld = GetExistingRemap(pcReadObject->GetNewIndex());
			pNewObject = gcObjects.Get(pcReadObject->GetNewIndex());
			pOldObject = gcObjects.Get(oiOld);
	
			pNewObject.RemapFrom(&pOldObject);
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CObjectGraphDeserialiser::GetExistingRemap(OIndex oiNew)
{
	int				i;
	CIndexNewOld*	pcRemap;

	for (i = 0; i < mcIndexRemap.NumElements(); i++)
	{
		pcRemap = mcIndexRemap.Get(i);
		if (pcRemap->moiNew = oiNew)
		{
			return pcRemap->moiOld;
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
void CObjectGraphDeserialiser::ForceAddDependent(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr)
{
	if ((pcHeader->mcType == OBJECT_POINTER_NAMED) || (pcHeader->mcType == OBJECT_POINTER_ID))
	{
		mcDependentObjects.Add(pcHeader, ppcObjectPtr, NULL);
	}
}

