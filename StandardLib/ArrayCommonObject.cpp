/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Objects.h"
#include "ObjectSerialiser.h"
#include "ObjectDeserialiser.h"
#include "ArrayCommonObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Init(BOOL bUnique, BOOL bIgnoreNull, BOOL bPreserveOrder, int iChunkSize)
{
	mcArray.Init(FALSE, FALSE, bUnique, bIgnoreNull, bPreserveOrder, iChunkSize);
	mbSubRoot = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Class(void)
{
	CCollection::Class();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Kill(void)
{
	CCollection::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::KillInternalData(void)
{
	CBaseObject::KillInternalData();
	mcArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::RemoveAll(void)
{
	int				i;
	CBaseObject*	pcObject;

	for (i = 0; i < NumElements(); i++)
	{
		pcObject = UnsafeGet(i);
		if (pcObject)
		{
			pcObject->RemoveHeapFrom(this, FALSE);
		}
	}

	mcArray.ReInit();

	mpcObjectsThisIn->ValidateConsistency();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::KillAll(void)
{
	CArrayBaseObjectPtr	apcKilled;
	CBaseObject*		pcPointedTo;
	int					i;
	CBaseObject*		pcContainer;
	int					iNumElements;

	apcKilled.Init(1024);
	iNumElements = mcArray.UnsafeNumElements();

	for (i = 0; i < iNumElements; i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			pcContainer = pcPointedTo->GetEmbeddingContainer();
			pcContainer->RemoveAllStackFroms();
			pcContainer->CollectThoseToBeKilled(&apcKilled);
		}
	}

	for (i = 0; i < iNumElements; i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			pcContainer = pcPointedTo->GetEmbeddingContainer();
			pcContainer->RemoveAllHeapFroms();
		}
	}

	KillCollected(&apcKilled);

	apcKilled.Kill();

	mcArray.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayCommonObject::Remove(CBaseObject* pcObject)
{
	//This is a test method.
	if (pcObject)
	{
		if (mcArray.Remove(pcObject))
		{
			pcObject->RemoveHeapFrom(this);
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::KillData(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayCommonObject::Save(CObjectSerialiser* pcFile)
{
	int				i;
	int				iNumElements;
	CBaseObject*	pcPointedTo;

	ReturnOnFalse(mcArray.SaveArrayHeader(pcFile));

	ReturnOnFalse(pcFile->WriteBool(mbSubRoot));

	iNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		ReturnOnFalse(pcFile->WriteDependent(pcPointedTo));
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayCommonObject::Load(CObjectDeserialiser* pcFile)
{
	int					i;
	int					iFlags;
	int					iNumElements;
	CEmbeddedObject**	pcPointedTo;

	//Note: This function has never been called.
	ReturnOnFalse(mcArray.LoadArrayHeader(pcFile, &iFlags, &iNumElements));

	ReturnOnFalse(pcFile->ReadBool(&mbSubRoot));

	for (i = 0; i < iNumElements; i++)
	{
		pcPointedTo = (CEmbeddedObject**)mcArray.UnsafeGetPointer(i);
		ReturnOnFalse(pcFile->ReadDependent(pcPointedTo, this));
	}

	mcArray.PostLoad(iFlags);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Add(CPointer& pObject)
{
	mcArray.Add(pObject.Object());
	pObject.AddHeapFrom(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::AddAll(CArrayCommonObject* pcArray)
{
	int				i;
	int				iNumElements;
	CBaseObject*	pcObject;

	iNumElements = pcArray->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcObject = pcArray->UnsafeGet(i);
		mcArray.Add(pcObject);
		if (pcObject != NULL)
		{
			pcObject->AddHeapFrom(this);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Set(int iIndex, CPointer& pObject)
{
	CBaseObject*		pcPointedTo;

	if ((iIndex < 0) || (iIndex >= mcArray.UnsafeNumElements()))
	{
		return;
	}

	pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(iIndex);
	mcArray.Set(iIndex, pObject.Object());
	if (pcPointedTo)
	{
		pcPointedTo->RemoveHeapFrom(this);
	}

	pObject.AddHeapFrom(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayCommonObject::Remove(CPointer& pObject)
{
	if (pObject.IsNotNull())
	{
		if (mcArray.Remove(pObject.Object()))
		{
			pObject->RemoveHeapFrom(this);
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayCommonObject::NumElements(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayCommonObject::NonNullElements(void)
{
	return mcArray.UnsafeNonNullElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayCommonObject::NumTos(void)
{
	CBaseObject*	pcPointedTo;
	int				i;
	int				iCount;

	iCount = 0;
	for (i = 0; i < mcArray.UnsafeNumElements(); i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			iCount++;
		}
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayCommonObject::UnsafeNumEmbeddedObjectTos(void)
{
	return NumTos();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::CollectPointedToToBeKilled(CArrayBaseObjectPtr* papcKilled)
{
	CBaseObject*			pcPointedTo;
	int						i;

	for (i = 0; i < mcArray.UnsafeNumElements(); i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		CBaseObject::CollectPointedToToBeKilled(papcKilled, pcPointedTo);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::RemoveAllTos(CArrayEmbeddedBaseObjectPtr* papcFromsChanged)
{
	CBaseObject*			pcPointedTo;
	int						i;

	for (i = 0; i < mcArray.UnsafeNumElements(); i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			RemoveToFrom(pcPointedTo, papcFromsChanged);
		}
	}
	mcArray.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::UpdateEmbeddedObjectTosDistToRoot(CDistToRootEffectedFroms* pcEffectedFroms, int iExpectedDist)
{
	int					i;
	CEmbeddedObject*	pcPointedTo;
	CBaseObject*		pcBaseObject;
	int					iNumElements;

	iNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			pcBaseObject = pcPointedTo->GetEmbeddingContainer();
			if (!pcBaseObject->IsUpdatedToRoot())
			{
				pcBaseObject->ClearDistToRoot();
				pcEffectedFroms->Add(pcBaseObject, iExpectedDist+1);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::ClearEmbeddedObjectTosUpdatedToRootFlag(void)
{
	int					i;
	CEmbeddedObject*	pcPointedTo;
	CBaseObject*		pcBaseObject;
	int					iNumElements;

	iNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			pcBaseObject = pcPointedTo->GetEmbeddingContainer();
			pcBaseObject->ClearTosUpdatedToRootFlag();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::UpdateEmbeddedObjectTosDetached(CDistDetachedFroms* pcDetached, CDistToRootEffectedFroms* pcEffectedFroms)
{
	int					i;
	CEmbeddedObject*	pcPointedTo;
	CBaseObject*		pcBaseObject;
	int					iNumElements;

	iNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			pcBaseObject = pcPointedTo->GetEmbeddingContainer();
			pcBaseObject->UpdateTosDetached(pcDetached, pcEffectedFroms);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::RemoveEmbeddedObjectAllTos(CArrayEmbeddedBaseObjectPtr* papcFromsChanged)
{
	RemoveAllTos(papcFromsChanged);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::RemoveTo(CEmbeddedObject* pcTo)
{
	mcArray.Remove((CUnknown*)pcTo);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::MakeSubRoot(void)
{
	mbSubRoot = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayCommonObject::IsSubRoot(void)
{
	return mbSubRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::SetDistToRootAndSetPointedTosExpectedDistToRoot(int iDistToRoot)
{
	if (miDistToRoot != iDistToRoot)
	{
		miDistToRoot = iDistToRoot;

		SetPointedTosDistToRoot(iDistToRoot);
	}
}

	//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::SetPointedTosDistToRoot(int iDistToRoot)
{
	CBaseObject*			pcPointedTo;
	CBaseObject*			pcContainer;
	int						i;
	int						iNumElements;

	iNumElements = mcArray.UnsafeNumElements();

	if (iDistToRoot >= ROOT_DIST_TO_ROOT)
	{
		for (i = 0; i < iNumElements; i++)
		{
			pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
			if (pcPointedTo)
			{
				pcContainer = pcPointedTo->GetEmbeddingContainer();
				pcContainer->SetExpectedDistToRoot(iDistToRoot+1);
			}
		}
	}
	else if (iDistToRoot == UNATTACHED_DIST_TO_ROOT)
	{
		for (i = 0; i < iNumElements; i++)
		{
			pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
			if (pcPointedTo)
			{
				pcContainer = pcPointedTo->GetEmbeddingContainer();
				pcContainer->SetCalculatedDistToRoot();
			}
		}
	}
	else
	{
		gcLogger.Error2(__METHOD__, "Don't know how to set dist to root to [", IntToString(iDistToRoot), "].", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::GetTos(CArrayEmbeddedObjectPtr* papcTos)
{
	CEmbeddedObject*	pcPointedTo;
	int					i;
	int					iNumElements;

	iNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			papcTos->Add(&pcPointedTo);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::UnsafeGetEmbeddedObjectTos(CArrayEmbeddedObjectPtr* papcTos)
{
	GetTos(papcTos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayCommonObject::ContainsTo(CEmbeddedObject* pcEmbedded)
{
	CEmbeddedObject*	pcPointedTo;
	int					i;

	for (i = 0; i < mcArray.UnsafeNumElements(); i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			if (pcPointedTo == pcEmbedded)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CArrayCommonObject::UnsafeGet(int iIndex)
{
	return (CBaseObject*)mcArray.UnsafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayCommonObject::RemapTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	int					iCount;
	CEmbeddedObject**	ppcPointedTo;
	int					i;
	int					iNumElements;

	iCount = 0;
	iNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < iNumElements; i++)
	{
		ppcPointedTo = (CEmbeddedObject**)mcArray.UnsafeGetPointer(i);
		if (*ppcPointedTo)
		{
			if (*ppcPointedTo == pcOld)
			{
				*ppcPointedTo = pcNew;
				iCount++;
			}
		}
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CArrayCommonObject::GetEmbeddedObject(int iIndex)
{
	if (iIndex == 0)
	{
		return this;
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::ValidateEmbeddedObjectTos(void)
{
	ValidateTos();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::ValidateTos(void)
{
	int					iCount;
	CEmbeddedObject**	ppcPointedTo;
	int					i;

	iCount = 0;
	for (i = 0; i < mcArray.NumElements(); i++)
	{
		ppcPointedTo = (CEmbeddedObject**)mcArray.UnsafeGetPointer(i);
		if (*ppcPointedTo)
		{
			ValidateTo(*ppcPointedTo);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::ValidateConsistency(void)
{
	ValidateEmbeddedConsistency();
	ValidateCanFindRoot();
}

