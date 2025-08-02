/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "ClassDefines.h"
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

	U_Bool(mbSubRoot);
	U_Unknown(CArrayCommonUnknown, mcArray);
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
	size 			i;
	CBaseObject*	pcObject;
	size 			uiNumElements;

	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
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
	size 			i;
	size 			uiNumElements;
	CBaseObject*	pcPointedTo;

	ReturnOnFalse(mcArray.SaveArrayHeader(pcFile));

	ReturnOnFalse(pcFile->WriteBool(mbSubRoot));

	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
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
	size 				i;
	uint16 				iFlags;
	size 				uiNumElements;
	CEmbeddedObject**	pcPointedTo;

	//Note: This function has never been called.
	ReturnOnFalse(mcArray.LoadArrayHeader(pcFile, &iFlags, &uiNumElements));

	ReturnOnFalse(pcFile->ReadBool(&mbSubRoot));

	for (i = 0; i < uiNumElements; i++)
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
	size 			i;
	size 			uiNumElements;
	CBaseObject*	pcObject;

	uiNumElements = pcArray->NumElements();
	for (i = 0; i < uiNumElements; i++)
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
void CArrayCommonObject::Set(size iIndex, CPointer& pObject)
{
	CBaseObject*		pcPointedTo;

	if (iIndex >= mcArray.UnsafeNumElements())
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
size CArrayCommonObject::NumElements(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonObject::Size(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonObject::Length(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::IsEmpty(void)
{
	return mcArray.IsEmpty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonObject::NonNullElements(void)
{
	return mcArray.UnsafeNonNullElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonObject::NumPointerTos(void)
{
	return BaseNumPointerTos();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonObject::BaseNumPointerTos(void)
{
	CBaseObject*	pcPointedTo;
	size 			i;
	size 			iCount;
	size 			uiNumElements;

	uiNumElements = mcArray.UnsafeNumElements();
	iCount = 0;
	for (i = 0; i < uiNumElements; i++)
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
	CBaseObject*	pcPointedTo;
	size 			i;
	size 			uiNumElements;

	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
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
	size 			i;
	size 			uiNumElements;

	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
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
	size 				i;
	CEmbeddedObject*	pcPointedTo;
	size 				uiNumElements;

	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		AddExpectedDistToRoot(pcPointedTo, iExpectedDist + 1, pcParameters);
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
	CBaseObject*	pcPointedTo;
	CBaseObject*	pcContainer;
	size			i;
	size 			uiNumElements;

	uiNumElements = mcArray.UnsafeNumElements();

#ifdef _DEBUG
	Validate((uiNumElements != 0xCCCCCCCC));
#endif // _DEBUG

	if (iDistToRoot >= ROOT_DIST_TO_ROOT)
	{
		for (i = 0; i < uiNumElements; i++)
		{
			pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
			if (pcPointedTo)
			{
				pcContainer = pcPointedTo->GetEmbeddingContainer();
				pcContainer->SetExpectedDistToRoot(iDistToRoot + 1);
			}
		}
	}
	else if (iDistToRoot == UNATTACHED_DIST_TO_ROOT)
	{
		for (i = 0; i < uiNumElements; i++)
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
	size 				i;
	size 				uiNumElements;

	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
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
	size 				i;
	size 				uiNumElements;
	CBaseObject*		pcContainer;

	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
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
	size 				i;
	size 				uiNumElements;

	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
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
CBaseObject* CArrayCommonObject::UnsafeGet(size iIndex)
{
	return (CBaseObject*)mcArray.UnsafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonObject::RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	size 					iCount;
	CEmbeddedObject**	ppcPointedTo;
	size 					i;
	size 					uiNumElements;

	iCount = 0;
	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
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
CEmbeddedObject* CArrayCommonObject::GetEmbeddedObject(size iIndex)
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
	size 				iCount;
	CEmbeddedObject*	ppcPointedTo;
	size 				uiNumElements;
	SSetIterator		sIter;

	uiNumElements = mcArray.NumElements();
	iCount = 0;
	ppcPointedTo = (CEmbeddedObject*)mcArray.StartIteration(&sIter);
	while (ppcPointedTo)
	{
		ValidatePointerTo(ppcPointedTo);
		ppcPointedTo = (CEmbeddedObject*)mcArray.Iterate(&sIter);
		iCount++;
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
	size 			i;
	size 			uiNumElements;

	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
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
	size 			i;
	size 			uiNumElements;

	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcObject = (CBaseObject*)mcArray.UnsafeGet(i);
		pcObject->Kill();
	}
}

