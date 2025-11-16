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
	CEmbeddedObject*	pcObject;

	pcObject = pObject.Object();
	return Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Add(CEmbeddedObject* pcObject)
{
	bool				bAdded;

	bAdded = mcArray.Add(pcObject);
	if (bAdded)
	{
		if (pcObject)
		{
			if (IsEmbeddingContainerAllocatedInObjects())
			{
				pcObject->AddHeapFrom(this, true);
			}
			else
			{
				pcObject->AddStackFrom(this);
			}
		}
	}
	return bAdded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::AddAll(CArrayCommonObject* pcArray)
{
	size 			i;
	size 			uiNumElements;
	CBaseObject*	pcObject;
	bool			bAdded;

	uiNumElements = pcArray->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcObject = pcArray->UnsafeGet(i);
		bAdded = Add(pcObject);
		if (!bAdded)
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
bool CArrayCommonObject::Set(size iIndex, CPointer& pObject)
{
	CBaseObject*	pcPointedTo;
	bool			bResult;

	if (iIndex >= mcArray.UnsafeNumElements())
	{
		return false;
	}

	pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(iIndex);
	bResult = mcArray.Set(iIndex, pObject.Object());

	if (bResult)
	{
		if (pcPointedTo)
		{
			if (IsEmbeddingContainerAllocatedInObjects())
			{
				bResult = pcPointedTo->RemoveHeapFromTryFree(this, true);
			}
			else
			{
				bResult = pcPointedTo->RemoveStackFromTryFree(this, true);
			}
		}
	}

	if (bResult)
	{
		pObject.AddHeapFrom(this);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Remove(CPointer& pObject)
{
	CEmbeddedObject*	pcObject;

	if (pObject.IsNotNull())
	{
		pcObject = pObject.Object();
		return Remove(pcObject);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Remove(CEmbeddedObject* pcObject)
{
	//This method is only publically called from tests.
	if (pcObject)
	{
		if (mcArray.Remove(pcObject))
		{
			if (IsEmbeddingContainerAllocatedInObjects())
			{
				return pcObject->RemoveHeapFromTryFree(this, true);
			}
			else
			{
				return pcObject->RemoveStackFromTryFree(this, true);
			}
		}
		return gcLogger.Error2(__METHOD__, " Could not remove Object {", ObjectToString(pcObject), "} from Array {", ObjectToString(this), "}.", NULL);
	}
	return gcLogger.Error2(__METHOD__, " Could not remove Object {NULL} from Array {", ObjectToString(this), "}.", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::RemoveAll(void)
{
	size 			i;
	CBaseObject*	pcObject;
	size 			uiNumElements;
	bool			bRemoved;

	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcObject = UnsafeGet(i);
		if (pcObject)
		{
			if (IsEmbeddingContainerAllocatedInObjects())
			{
				bRemoved = pcObject->RemoveHeapFromTryFree(this, false);
			}
			else
			{
				bRemoved = pcObject->RemoveStackFromTryFree(this, true);
			}
			if (!bRemoved)
			{
				return false;
			}
		}
	}

	mcArray.ReInit();

#ifdef _DEBUG
	mpcObjectsThisIn->ValidateObjectsConsistency();
#endif // _DEBUG

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::RemoveAllPointerTosDontFree(void)
{
	CBaseObject* pcPointedTo;
	size 			i;
	size 			uiNumElements;

	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			if (IsEmbeddingContainerAllocatedInObjects())
			{
				RemoveToFromDontFree(pcPointedTo);
			}
			else
			{
				//I'm not sure if that can ever be called.
				gcLogger.Error2(__METHOD__, " is not yet implemented for Stack-Froms.", NULL);
			}
		}
	}
	mcArray.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::RemoveAllPointerTosTryFree(void)
{
	//Called by KillInternal.
	//You could factor out a common method between this and RemoveAll.

	CBaseObject*	pcPointedTo;
	size 			i;
	size 			uiNumElements;
	bool			bRemoved;

	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			if (IsEmbeddingContainerAllocatedInObjects())
			{
				bRemoved = pcPointedTo->RemoveHeapFromTryFree(this, false);
			}
			else
			{
				bRemoved = pcPointedTo->RemoveStackFromTryFree(this, true);
			}
			mcArray.UnsafeSet(i, NULL);
		}
	}
	mcArray.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Clear(void)
{
	return RemoveAll();
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
	CEmbeddedObject**		ppcPointedTo;
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
void CArrayCommonObject::UnsafePointTo(CEmbeddedObject* pcNew, CEmbeddedObject* pcOld)
{
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::InsertAt(size iIndex, CPointer& pObject)
{
	CEmbeddedObject* pcObject;

	pcObject = pObject.Object();
	return InsertAt(iIndex, pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::InsertAt(size iIndex, CEmbeddedObject* pcObject)
{
	bool				bAdded;

	bAdded = mcArray.Insert(iIndex, pcObject);
	if (bAdded)
	{
		if (pcObject)
		{
			if (IsEmbeddingContainerAllocatedInObjects())
			{
				pcObject->AddHeapFrom(this, true);
			}
			else
			{
				pcObject->AddStackFrom(this);
			}
		}
	}
	return bAdded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::RemoveAt(size iIndex)
{
	CEmbeddedObject*	pcObject;
	bool				bResult;

	if (iIndex < mcArray.UnsafeNumElements())
	{
		pcObject = UnsafeGet(iIndex);
		bResult = mcArray.Remove(iIndex);

		if (bResult && pcObject)
		{
			if (IsEmbeddingContainerAllocatedInObjects())
			{
				bResult = pcObject->RemoveHeapFromTryFree(this, true);
			}
			else
			{
				bResult = pcObject->RemoveStackFromTryFree(this, true);
			}
		}
		return bResult;
	}
	return false;
}

