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
#include "ObjectWriter.h"
#include "ObjectReader.h"
#include "ArrayCommonObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CArrayCommonObject> CArrayCommonObject::Init(bool bUnique, bool bIgnoreNull, bool bPreserveOrder)
{
	mcArray.Init(false, false, bUnique, bIgnoreNull, bPreserveOrder);
	mbSubRoot = false;
	CCollection::Init();
	return this;
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
void CArrayCommonObject::FreePointers(void)
{
	CBaseObject::FreePointers();
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
			pcObject->RemoveHeapFrom(this, false);
		}
	}

	mcArray.ReInit();

#ifdef _DEBUG
	mpcObjectsThisIn->ValidateObjectsConsistency();
#endif // _DEBUG
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Clear(void)
{
	RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Remove(CBaseObject* pcObject)
{
	//This is a test method.
	if (pcObject)
	{
		if (mcArray.Remove(pcObject))
		{
			pcObject->RemoveHeapFrom(this, true);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::UnsafeRemove(CBaseObject* pcObject)
{
	//This is a test method.
	if (pcObject)
	{
		if (mcArray.Remove(pcObject))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Save(CObjectWriter* pcFile)
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
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Load(CObjectReader* pcFile)
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
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Add(CPointer& pObject)
{
	bool	bAdded;

	bAdded = mcArray.Add(pObject.Object());
	if (bAdded)
	{
		pObject.AddHeapFrom(this);
	}
	return bAdded;
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
			pcObject->AddHeapFrom(this, true);
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
		pcPointedTo->RemoveHeapFrom(this, true);
	}

	pObject.AddHeapFrom(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Remove(CPointer& pObject)
{
	if (pObject.IsNotNull())
	{
		if (mcArray.Remove(pObject.Object()))
		{
			pObject->RemoveHeapFrom(this, true);
			return true;
		}
	}
	return false;
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
int CArrayCommonObject::Size(void)
{
	return NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayCommonObject::Length(void)
{
	return NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::IsEmpty(void)
{
	return NumElements() == 0;
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
int CArrayCommonObject::NumPointerTos(void)
{
	return BaseNumPointerTos();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayCommonObject::BaseNumPointerTos(void)
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
void CArrayCommonObject::RemoveAllPointerTosDontKill(void)
{
	CBaseObject* pcPointedTo;
	int						i;

	for (i = 0; i < mcArray.UnsafeNumElements(); i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			RemoveToFrom(pcPointedTo);
		}
	}
	mcArray.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::RemoveAllPointerTos(void)
{
	CBaseObject*	pcPointedTo;
	int				i;

	for (i = 0; i < mcArray.UnsafeNumElements(); i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			pcPointedTo->RemoveHeapFrom(this, true);
			mcArray.UnsafeSet(i, NULL);
		}
	}
	mcArray.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist)
{
	int					i;
	CEmbeddedObject*	pcPointedTo;
	int					iNumElements;

	iNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		AddExpectedDistToRoot(pcPointedTo, iExpectedDist+1, pcParameters);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::RemovePointerTo(CEmbeddedObject* pcTo)
{
	mcArray.Remove((CUnknown*)pcTo);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::MakeSubRoot(void)
{
	mbSubRoot = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::IsSubRoot(void)
{
	return mbSubRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::SetPointerTosExpectedDistToRoot(int iDistToRoot)
{
	SetPointedTosDistToRoot(iDistToRoot);
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

#ifdef _DEBUG
	Validate((iNumElements != 0xCCCCCCCC));
#endif // _DEBUG

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
void CArrayCommonObject::GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos)
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
void CArrayCommonObject::BaseGetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos)
{
	GetPointerTos(papcTos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::CollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters)
{
	CEmbeddedObject*	pcPointedTo;
	int					i;
	int					iNumElements;
	CBaseObject*		pcContainer;

	iNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcPointedTo = (CEmbeddedObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			pcContainer = pcPointedTo->GetEmbeddingContainer();
			pcContainer->CollectAndClearInvalidDistToRootObjects(pcParameters);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::ContainsPointerTo(CEmbeddedObject* pcEmbedded)
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
				return true;
			}
		}
	}
	return false;
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
int CArrayCommonObject::RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
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
CEmbeddedObject* CArrayCommonObject::GetEmbeddedObject(uint16 iIndex)
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
void CArrayCommonObject::BaseValidatePointerTos(void)
{
	ValidatePointerTos();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::ValidatePointerTos(void)
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
			ValidatePointerTo(*ppcPointedTo);
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::TouchAll(void)
{
	CBaseObject*	pcObject;
	CPointer		pObject;
	int				i;
	int				iNum;

	iNum = mcArray.UnsafeNumElements();
	for (i = 0; i < iNum; i++)
	{
		pcObject = (CBaseObject*)mcArray.UnsafeGet(i);
		pObject.AssignObject(pcObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::KillAll(void)
{
	CBaseObject*	pcObject;
	CPointer		pObject;
	int				i;
	int				iNum;

	iNum = mcArray.UnsafeNumElements();
	for (i = 0; i < iNum; i++)
	{
		pcObject = (CBaseObject*)mcArray.UnsafeGet(i);
		pcObject->Kill();
	}
}

