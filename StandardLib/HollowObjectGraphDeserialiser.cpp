#include "Objects.h"
#include "HollowObjectGraphDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObjectGraphDeserialiser::Init(CObjectReader* pcReader, CIndexGenerator* pcIndexGenerator)
{
	CObjectGraphDeserialiser::Init(pcReader, pcIndexGenerator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObjectGraphDeserialiser::Kill(void)
{
	CObjectGraphDeserialiser::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObjectGraphDeserialiser::AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining)
{
	char*	szName;

	if (pcHeader->mcType == OBJECT_POINTER_ID)
	{
		mcDependentObjects.AddIgnoreNamed(pcHeader, ppcObjectPtr, pcContaining);
	}
	else if (pcHeader->mcType == OBJECT_POINTER_NAMED)
	{
		CPointerObject	pObject;

		szName = pcHeader->mszObjectName.Text();
		pObject = gcObjects.Get(szName);
		if (pObject.IsNull())
		{
			pObject = gcObjects.AddHollow(szName);
		}

		FixPointer(pObject.Object(), ppcObjectPtr, pcContaining);
	}
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

