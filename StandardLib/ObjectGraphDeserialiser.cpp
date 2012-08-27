#include "BaseLib/ErrorTypes.h"
#include "Objects.h"
#include "DependentObjectDeserialiser.h"
#include "ObjectGraphDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::Init(CObjectReader* pcReader, CIndexGenerator* pcIndexGenerator)
{
	mpcReader = pcReader;
	mcDependentObjects.Init(pcIndexGenerator);
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

	pObject = pcObjectPtr;
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr)
{
	if ((pcHeader->mcType == OBJECT_POINTER_NAMED) || (pcHeader->mcType == OBJECT_POINTER_ID))
	{
		mcDependentObjects.Add(pcHeader, ppcObjectPtr);
	}
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
	CPointerObject					pObject;
	OIndex							oiNew;
	OIndex							oiOld;

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
	
	iNum = mcDependentObjects.NumPointers();
	for (i = 0; i < iNum; i++)
	{
		pcReadPointer = mcDependentObjects.GetPointer(i);
		pcBaseObject = gcObjects.GetBaseObject(pcReadPointer->moiPointedTo);
		if (pcBaseObject)
		{
			*pcReadPointer->mppcPointedFrom = pcBaseObject;
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}

