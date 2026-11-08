/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "BaseLib/PointerFunctions.h"
#include "BaseObject.h"
#include "Objects.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::ValidateEmpty(void)
{
	OIndex	iNumIndexed;

	iNumIndexed = mcMemory.NumIndexed();
	if (iNumIndexed != 0)
	{
		CChars				sz;

		sz.Init("\n");
		sz.Append("Memory not empty.  ");
		sz.Append(iNumIndexed);
		sz.Append(" objects are still indexed.\n");
		PrintMemoryUseIteration(&sz);
		gcLogger.Error(sz.Text());
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Validates every object in memory.  Memory is walked once, into an array
// the passes then use: the walk costs more than most of the checks.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::ValidateObjectsConsistency(void)
{
	//If this method is called from an Object - rather than a test case - then it should be wrapped with a #ifdef _DEBUG
	//This is because it is still useful to have ValidateObjectsConsistency called in RELEASE from tests.

	CArrayBaseObjectPtr		apcObjects;

	//Objects being Removed are flagged FREED but are still in memory while their Free() runs.  If Free() 
	//causes other objects to be killed (e.g. RemoveFreeObject) the graph is not yet consistent.
	if (mbValidateConsistency && !mcRemovalContext.IsBusy())
	{
		apcObjects.Init();
		GetObjectsInMemory(&apcObjects);

		ValidateSceneGraph();
		ValidateIndexedObjects(&apcObjects);
		ClearValidationFlags(&apcObjects);
		ValidateObjectInternals(&apcObjects);

		apcObjects.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::GetObjectsInMemory(CArrayBaseObjectPtr* papcObjects)
{
	SIndexesIterator	sIter;
	CBaseObject*		pcBaseObject;

	pcBaseObject = mcMemory.StartIteration(&sIter);
	while (pcBaseObject)
	{
		papcObjects->Add(pcBaseObject);
		pcBaseObject = mcMemory.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Validates the objects the scene graph didn't reach.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::ValidateIndexedObjects(CArrayBaseObjectPtr* papcObjects)
{
	size			i;
	size			uiNumObjects;
	CBaseObject*	pcBaseObject;

	uiNumObjects = papcObjects->NumElements();
	for (i = 0; i < uiNumObjects; i++)
	{
		pcBaseObject = papcObjects->GetPtr(i);
		if (!(pcBaseObject->muiFlags & OBJECT_FLAGS_TESTED_FOR_SANITY))
		{
			pcBaseObject->SetFlag(OBJECT_FLAGS_TESTED_FOR_SANITY, true);

			pcBaseObject->ValidateConsistency();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::ClearValidationFlags(CArrayBaseObjectPtr* papcObjects)
{
	size	i;
	size	uiNumObjects;

	uiNumObjects = papcObjects->NumElements();
	for (i = 0; i < uiNumObjects; i++)
	{
		papcObjects->GetPtr(i)->SetFlag(OBJECT_FLAGS_TESTED_FOR_SANITY, false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::ValidateObjectInternals(CArrayBaseObjectPtr* papcObjects)
{
	size			i;
	size			uiNumObjects;
	CBaseObject*	pcObject;
	CArrayVoidPtr	apvStackPointers;

	//Sorted once, so finding those inside an object is a search, not a walk of them all.
	apvStackPointers.Init();
	GetStackPointerAddresses(&apvStackPointers);
	apvStackPointers.QuickSort(&ComparePtrPtr);

	uiNumObjects = papcObjects->NumElements();
	for (i = 0; i < uiNumObjects; i++)
	{
		pcObject = papcObjects->GetPtr(i);
		pcObject->ValidateNoStackPointerInside(&apvStackPointers);
		pcObject->ValidateInternalConsistency();
	}
	apvStackPointers.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The address of each stack pointer (a Ptr or a collection on the stack).
// A collection pointing to several objects is there once for each.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::GetStackPointerAddresses(CArrayVoidPtr* papvAddresses)
{
	CStackPointer*		pcStackPointer;
	SStackPointer*		psStackPointer;
	SFreeListIterator	sIter;

	pcStackPointer = GetStackPointers()->StartIteration(&sIter);
	while (pcStackPointer)
	{
		psStackPointer = pcStackPointer->Get();
		if ((psStackPointer->eType == SPT_Pointer) || (psStackPointer->eType == SPT_Collection))
		{
			papvAddresses->Add(psStackPointer->u.pvPointer);
		}
		pcStackPointer = GetStackPointers()->Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Validates the objects reachable from the root, depth first.  It keeps
// its own stack: a long chain overflowed the program's.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::ValidateSceneGraph(void)
{
	CRoot*					pcRoot;
	CArrayBaseObjectPtr		apcStack;
	CBaseObject*			pcBaseObject;

	pcRoot = (CRoot*)GetFromMemory(ROOT_NAME);
	if (pcRoot != NULL)
	{
		apcStack.Init();
		apcStack.Add(pcRoot);
		while (apcStack.NumElements() > 0)
		{
			pcBaseObject = apcStack.Pop();
			ValidateSceneGraphObject(pcBaseObject, &apcStack);
		}
		apcStack.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Validates the object (unless already validated) and adds what it points
// to to papcStack, reversed so they are validated in order.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::ValidateSceneGraphObject(CBaseObject* pcBaseObject, CArrayBaseObjectPtr* papcStack)
{
	CArrayEmbeddedObjectPtr		apcTos;
	size						i;
	CEmbeddedObject*			pcToObject;
	size						uiNumTos;

	if (pcBaseObject->IsEmbedded())
	{
		pcBaseObject->FailNotExpectedToBeEmbedded(__METHOD__);
	}

	if (!pcBaseObject->TestedForSanity())
	{
		pcBaseObject->SetFlag(OBJECT_FLAGS_TESTED_FOR_SANITY, true);

		pcBaseObject->ValidateConsistency();

		apcTos.Init();
		pcBaseObject->GetPointerTos(&apcTos);
		uiNumTos = apcTos.NumElements();
		for (i = uiNumTos; i > 0; i--)
		{
			pcToObject = *apcTos.Get(i - 1);
			papcStack->Add(pcToObject->GetEmbeddingContainer());
		}

		apcTos.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::ValidateNoEmbedded(CArrayBaseObjectPtr* papcObjectPts)
{
	size			i;
	CBaseObject*	pcKilled;
	size			iNumElements;
	CBaseObject*	pcContainer;

	iNumElements = papcObjectPts->NumElements();
	if (iNumElements == 0)
	{
		return true;
	}

	for (i = 0; i < iNumElements; i++)
	{
		pcKilled = *papcObjectPts->Get(i);
		if (pcKilled->IsEmbedded())
		{
			pcContainer = pcKilled->GetEmbeddingContainer();
			gcLogger.Error2(__METHOD__, " Object of class [", pcKilled->ClassName(), "] is marked for killing but is embedded in object with index [", IndexToString(pcContainer->GetIndex()),"] of class [", pcContainer->ClassName(), "].", NULL);
			return false;
		}
		else if (!pcKilled->IsAllocatedInObjects())
		{
			gcLogger.Error2(__METHOD__, " Object of class [", pcKilled->ClassName(), "] is marked for killing but is not allocated in Objects [0x", PointerToString(this),"].", NULL);
			return false;
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::EnableValidation(void)
{
	mbValidateConsistency = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DisableValidation(void)
{
	mbValidateConsistency = false;
}

