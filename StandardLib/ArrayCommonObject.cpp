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
CArrayCommonObject::~CArrayCommonObject()
{
	Kill();

	//This Destructor code will only be called if the object was allocated on the stack.
	if (muiFlags != OBJECT_FLAGS_CALLED_CONSTRUCTOR)
	{
		if (mpcUnknownsThisIn == NULL)
		{
			if (!HasClass())
			{
				ValidateHasClassFlag(__METHOD__);
			}
		}
		FreePointers();

		ValidateInitCalled();
		ValidateKillCalled();

		muiFlags = 0;
	}
	else
	{
		//Skip all destruction as this object was only constructed to get its ClassName().
		muiFlags = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CArrayCommonObject> CArrayCommonObject::Init(bool bUnique, bool bIgnoreNull, bool bPreserveOrder)
{
	PreInit();
	CCollection::Init();
	mcArray.Init(false, false, bUnique, bIgnoreNull, bPreserveOrder);
	mbSorted = true;
	PostInit();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Class(void)
{
	CCollection::Class();

	U_Unknown(CArrayCommonUnknown, mcArray);
	U_Bool(mbSorted);
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

	ReturnOnFalse(mcArray.LoadArrayHeader(pcFile, &iFlags, &uiNumElements));

	ReturnOnFalse(pcFile->ReadBool(&mbSubRoot));

	for (i = 0; i < uiNumElements; i++)
	{
		pcPointedTo = (CEmbeddedObject**)mcArray.UnsafeGetPointer(i);
		ReturnOnFalse(pcFile->ReadDependent(pcPointedTo, this));
	}

	mbSorted = false;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Sort(void)
{
	mcArray.Sort();
	mbSorted = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::EnsureSorted(void)
{
	if (!mbSorted)
	{
		Sort();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::IsSorted(void)
{
	return mbSorted;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Add(CPointer& pObject)
{
	CEmbeddedObject*	pcObject;

	EnsureSorted();

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

	EnsureSorted();

	bAdded = mcArray.Add(pcObject);
	bAdded = AddObjectFrom(pcObject, bAdded, true);
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

	EnsureSorted();

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
	CBaseObject*		pcPointedTo;
	bool				bResult;
	CEmbeddedObject*	pvObject;

	if (iIndex >= mcArray.UnsafeNumElements())
	{
		return false;
	}

	EnsureSorted();

	pvObject = pObject.Object();
	pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(iIndex);
	bResult = mcArray.Set(iIndex, pvObject);
	bResult = RemoveObjectTryFree(pcPointedTo, bResult, false);
	bResult = AddObjectFrom(pvObject, bResult, true);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CArrayCommonObject::Dereference(CEmbeddedObject** ppcObject)
{
	if (*ppcObject)
	{
		*ppcObject = (*ppcObject)->Dehollow();
		if (mcArray.IsMustSort() && IsSorted())
		{
			mbSorted = false;
		}
		return *ppcObject;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::GrowTo(size iNumElements)
{
	mcArray.GrowTo(iNumElements);
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
		EnsureSorted();

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
	if (pcObject)
	{
		EnsureSorted();

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
bool CArrayCommonObject::RemoveAllPointerTosTryFree(void)
{
	//Called by KillInternal and RemoveAll.

	CBaseObject*	pcPointedTo;
	size 			i;
	size 			uiNumElements;
	bool			bResult;

	bResult = true;
	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcPointedTo = UnsafeGet(i);
		bResult &= RemovePointerToTryFree(pcPointedTo);
	}
	mcArray.ReInit();
	return bResult;  // This can return an error but RemoveAllPointerTosTryFree callers don't handle it.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::RemoveAllPointerTosDontFree(void)
{
	CBaseObject*	pcPointedTo;
	size 			i;
	size 			uiNumElements;
	bool			bResult;

	bResult = true;
	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcPointedTo = UnsafeGet(i);
		bResult &= RemovePointerToDontFree(pcPointedTo);
	}
	mcArray.ReInit();
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

	uiNumElements = NonNullElements();
	if (iCount != NonNullElements())
	{
		gcLogger.Error2(__METHOD__, " NumPointerTos [", SizeToString(iCount), "] should equal NonNullElements [", SizeToString(uiNumElements), "].");
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
			papcTos->Add(pcPointedTo);
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
		if ((pcPointedTo) && (pcPointedTo == pcEmbedded))
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
void CArrayCommonObject::ValidatePointerTos(void)
{
	CEmbeddedObject*	pcPointedTo;
	SSetIterator		sIter;
	bool				bExists;

	bExists = mcArray.StartIteration(&sIter, (CUnknown**)&pcPointedTo);
	while (bExists)
	{
		if (pcPointedTo)
		{
			ValidatePointerTo(pcPointedTo);
		}
		bExists = (CEmbeddedObject*)mcArray.Iterate(&sIter, (CUnknown**)&pcPointedTo);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CArrayCommonObject::StartIterationPointer(SSetIterator* psIter, bool* pbExists)
{
	CBaseObject*	pcObject;
	CPointer		pObject;
	bool			bExists;

	EnsureSorted();

	bExists = mcArray.StartIteration(psIter, (CUnknown**)&pcObject);
	*pbExists = bExists;
	pObject.AssignObject(pcObject);
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CArrayCommonObject::IteratePointer(SSetIterator* psIter, bool* pbExists)
{
	CBaseObject*	pcObject;
	CPointer		pObject;
	bool			bExists;

	bExists = mcArray.Iterate(psIter, (CUnknown**)&pcObject);
	*pbExists = bExists;
	pObject.AssignObject(pcObject);
	return pObject;
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

	EnsureSorted();

	pcObject = pObject.Object();
	return InsertAt(iIndex, pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::InsertAt(size iIndex, CEmbeddedObject* pcObject)
{
	bool				bResult;

	EnsureSorted();

	bResult = mcArray.Insert(iIndex, pcObject);
	bResult = AddObjectFrom(pcObject, bResult, true);
	return bResult;
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
		EnsureSorted();

		pcObject = UnsafeGet(iIndex);
		bResult = mcArray.Remove(iIndex);
		bResult = RemoveObjectTryFree(pcObject, bResult, true);
		return bResult;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::RemoveEnd(size iIndexInclusive)
{
	size				uiNumElements;
	size				i;
	CEmbeddedObject*	pcObject;
	bool				bResult;

	EnsureSorted();

	bResult = true;
	uiNumElements = mcArray.UnsafeNumElements();
	if (iIndexInclusive < uiNumElements)
	{
		for (i = iIndexInclusive; i < uiNumElements; i++)
		{
			pcObject = UnsafeGet(i);
			bResult = RemoveObjectTryFree(pcObject, bResult, false);
			if (!bResult)
			{
				break;
			}
		}
		mcArray.RemoveEnd(iIndexInclusive);
	}
	else
	{
		bResult = false;
	}

#ifdef _DEBUG
	ValidateObjectsConsistency(true);
#endif // _DEBUG
	return bResult;
}

