#include "Objects.h"
#include "HollowObjectGraphDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObjectGraphDeserialiser::Init(CObjectReader* pcReader, CIndexGenerator* pcIndexGenerator, CObjectAllocator* pcAllocator)
{
	CObjectGraphDeserialiser::Init(pcReader, pcIndexGenerator, pcAllocator);
	mcExistingHollowRemap.Init(32);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObjectGraphDeserialiser::Kill(void)
{
	mcExistingHollowRemap.Kill();
	CObjectGraphDeserialiser::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CHollowObjectGraphDeserialiser::AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining)
{
	if (pcHeader->mcType == OBJECT_POINTER_ID)
	{
		mcDependentObjects.AddIgnoreNamed(pcHeader, ppcObjectPtr, pcContaining);
	}
	else if (pcHeader->mcType == OBJECT_POINTER_NAMED)
	{
		mcDependentObjects.AddHollow(pcHeader->mszObjectName.Text(), pcHeader->moi, ppcObjectPtr, pcContaining);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CHollowObjectGraphDeserialiser::Read(char* szObjectName)
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
			if (!pcDependent->IsHollow())
			{
				bResult = ReadUnread(pcDependent, bFirst);
			}
			else
			{
				bResult = AddHollow(pcDependent);
			}
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
BOOL CHollowObjectGraphDeserialiser::AddHollow(CDependentReadObject* pcDependent)
{
	char*				szObjectName;
	CPointerObject		pObject;
	OIndex				oiNew;
	OIndex				oiOld;
	CIndexNewOld*		pcNewOld;

	szObjectName = pcDependent->GetName();
	oiOld = pcDependent->moi;
	oiNew = pcDependent->GetNewIndex();
	pObject = gcObjects.AddHollow(szObjectName, oiNew);
	if (pObject.IsNull())
	{
		return FALSE;
	}
	if (oiNew == pObject.GetIndex())
	{
		MarkRead(oiOld);
		return TRUE;
	}
	else
	{
		pcNewOld = mcExistingHollowRemap.Add();
		pcNewOld->Init(oiNew, pObject.GetIndex());
		MarkRead(oiOld);
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CHollowObjectGraphDeserialiser::GetExistingHollowRemap(OIndex oiNew)
{
	int				i;
	CIndexNewOld*	pcRemap;

	for (i = 0; i < mcExistingHollowRemap.NumElements(); i++)
	{
		pcRemap = mcExistingHollowRemap.Get(i);
		if (pcRemap->moiNew == oiNew)
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
BOOL CHollowObjectGraphDeserialiser::FixPointers(void)
{
	CDependentReadPointer*	pcReadPointer;
	int						i;
	int						iNum;
	CBaseObject*			pcBaseObject;
	OIndex					oiTo;

	iNum = mcDependentObjects.NumPointers();
	for (i = 0; i < iNum; i++)
	{
		pcReadPointer = mcDependentObjects.GetPointer(i);

		oiTo = GetExistingHollowRemap(pcReadPointer->moiPointedTo);
		if (oiTo == INVALID_O_INDEX)
		{
			oiTo = pcReadPointer->moiPointedTo;
		}

		pcBaseObject = gcObjects.GetInMemoryObject(oiTo);
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

