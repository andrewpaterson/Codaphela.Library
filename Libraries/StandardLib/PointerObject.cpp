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
#include "BaseLib/Define.h"
#include "BaseLib/Logger.h"
#include "BaseLib/Numbers.h"
#include "Object.h"
#include "HollowObject.h"
#include "ObjectReader.h"
#include "ObjectRemapFrom.h"
#include "PointerObject.h"
#include "ObjectsLock.h"
#include "ObjectsThreads.h"


bool gbLogPointer = false;

//What a pointer that is let go of is pointed at.
static CEmbeddedObject* const	gpcPointerNoObject = NULL;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LogPointerDebug(CPointer* pvThis, const char* szMethod)
{
	CBaseObject*	pcPointedTo;
	CObject*		pcEmbedding;
	CChars			sz;
	size			uiLength;
	size			uiOriginalLength;

	if (gbLogPointer)
	{
		pcEmbedding = pvThis->Embedding();
		sz.Init();
		sz.Append(szMethod);
		uiOriginalLength = sz.Length();
		sz.RemoveEnd(62);
		uiLength = sz.Length();
		sz.Append(": ");
		if (uiLength < 62)
		{
			sz.Append(' ', 62 - uiLength);
		}
		else if (uiOriginalLength >= 62)
		{
			sz.SetChar(59, '.');
			sz.SetChar(60, '.');
			sz.SetChar(61, '.');
		}
		sz.AppendPointer(pvThis);
		sz.Append(' ');
		if (pcEmbedding != NULL)
		{
			sz.Append("Embedding {");
			pcEmbedding->PrintState(&sz);
			sz.Append("}.");
		}
		sz.Append("Object {");
		pcPointedTo = pvThis->BaseObject();
		if (pcPointedTo)
		{
			pcPointedTo->PrintState(&sz);
		}
		else
		{
			sz.Append("NULL");
		}
		sz.Append("}");
		gcLogger.Debug(sz.Text());
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer::CPointer()
{
	mpcObject = NULL;
	mpcEmbedding = NULL;

	LOG_POINTER_DEBUG();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer::CPointer(CPointer& cPointer)
{
	//Constructor only called if a Pointer is declared on the Stack.
	mpcEmbedding = NULL;
	mpcObject = NULL;

	PointToFromOutside(&cPointer.mpcObject);

	LOG_POINTER_DEBUG();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer::CPointer(const CPointer& cPointer)
{
	//Constructor only called if a Pointer is declared on the Stack.
	mpcEmbedding = NULL;
	mpcObject = NULL;

	PointToFromOutside(&cPointer.mpcObject);

	LOG_POINTER_DEBUG();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer::CPointer(CEmbeddedObject* pcObject)
{
	//Constructor only called if a Pointer is declared on the Stack.
	mpcEmbedding = NULL;
	mpcObject = NULL;

	PointToFromOutside(&pcObject);

	LOG_POINTER_DEBUG();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer::~CPointer()
{
	LOG_POINTER_DEBUG();

	if (mpcObject)
	{
		if (!OBJECTS_LOCKING())
		{
			mpcObject->RemoveStackFromTryFree(this);
		}
		else if (!PointToReading(&gpcPointerNoObject))
		{
			ObjectsLock();
			mpcObject->RemoveStackFromTryFree(this);
			ObjectsUnlock();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::operator = (CEmbeddedObject* pcObject)
{
	PointToFromOutside(&pcObject);

	LOG_POINTER_DEBUG();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::operator = (CPointer& pPointer)
{
	PointToFromOutside(&pPointer.mpcObject);

	LOG_POINTER_DEBUG();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::operator = (const CPointer& pPointer)
{
	PointToFromOutside(&pPointer.mpcObject);

	LOG_POINTER_DEBUG();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CPointer::operator -> ()
{
	return DereferenceArrow();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CPointer::operator & ()
{
	return Dereference();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::operator ! ()
{
	return mpcObject == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::operator == (const CPointer& pPointer)
{
	return mpcObject == pPointer.Object();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::operator != (const CPointer& pPointer)
{
	return mpcObject != pPointer.Object();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::operator == (void* pcPointer)
{
	return mpcObject == pcPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::operator != (void* pcPointer)
{
	return mpcObject != pcPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::UnsafeClearObject(void)
{
	OBJECTS_INSIDE();

	mpcObject = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPointer::NumStackFroms(void) const
{
	OBJECTS_READ();

	if (mpcObject)
	{
		return mpcObject->NumStackFroms();
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPointer::NumHeapFroms(void) const
{
	OBJECTS_READ();

	if (mpcObject)
	{
		return mpcObject->NumHeapFroms();
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPointer::NumEmbedded(void) const
{
	OBJECTS_READ();

	if (mpcObject)
	{
		return mpcObject->NumEmbedded();
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPointer::NumPointerTos(void) const
{
	OBJECTS_READ();

	if (mpcObject)
	{
		return mpcObject->NumPointerTos();
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::SetEmbedding(CObject* pcEmbedding)
{
	mpcEmbedding = pcEmbedding;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::UnsafePointTo(CEmbeddedObject* pcNewObject)
{
	OBJECTS_INSIDE();

	mpcObject = pcNewObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::PointTo(CEmbeddedObject* pcNewObject)
{
	PointToFromOutside(&pcNewObject);
}


//////////////////////////////////////////////////////////////////////////
//
// Points this at *ppcObject, from outside the object system: what the constructors and the assignments do.
// *ppcObject (another pointer's object, where this is to point at what that does) is read once nothing can be
// changing it.
//
// With the locking on it is first tried as a reader, without the lock (PointToReading).
//
//////////////////////////////////////////////////////////////////////////
void CPointer::PointToFromOutside(CEmbeddedObject* const* ppcObject)
{
	if (!OBJECTS_LOCKING())
	{
		PointToInside(*ppcObject);
	}
	else if (!PointToReading(ppcObject))
	{
		ObjectsLock();
		PointToInside(*ppcObject);
		ObjectsUnlock();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer on the stack pointed from one object attached to the root (or none) at another (or none) changes two
// stack froms and nothing else: nothing can be freed by it and no distance changes.  That is most of what is ever
// done with pointers, and a thread that is reading may do it (level 2; ObjectsThreads.h).  False, with nothing
// done, if it isn't that or can't be read for now: it is then done with the lock.
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::PointToReading(CEmbeddedObject* const* ppcObject)
{
	CEmbeddedObject*	pcNewObject;
	CEmbeddedObject*	pcOldObject;
	bool				bDone;

	//A field of an object, or a pointer reached through a NULL one (PointToInside says how).
	if (((char*)this < CORRUPT_MEMORY) || (mpcEmbedding != NULL) || !ObjectsReadBegins())
	{
		return false;
	}

	pcNewObject = *ppcObject;
	pcOldObject = mpcObject;
	bDone = ((pcNewObject == NULL) || pcNewObject->IsAttachedToRoot()) && ((pcOldObject == NULL) || pcOldObject->IsAttachedToRoot());

	//The home thread with objects waiting for it to free takes the lock instead, which frees them.
	bDone = bDone && !(ObjectsReaderIsHome() && ObjectsAreFreesWaiting());
#ifdef OBJECTS_THREAD_CHECKS
	//A Debug build checks that an object in the database is one thread's at a time (ValidateStoredOnOneThread),
	//which looks where a reader may not: such an object's pointers are left for a writer.
	bDone = bDone && ((pcNewObject == NULL) || !pcNewObject->IsStoredOrHollow());
#endif // OBJECTS_THREAD_CHECKS
	if (bDone && (pcNewObject != pcOldObject))
	{
		//The old from goes first: taking it off is what can fail, and with both objects held by the root the order
		//frees nothing.
		if (pcOldObject != NULL)
		{
			bDone = pcOldObject->UnlinkStackFromReading(this);
		}
		if (bDone)
		{
			mpcObject = pcNewObject;
			if (pcNewObject != NULL)
			{
				pcNewObject->AddStackFromReading(this);
			}
		}
	}
	ObjectsReadEnds(bDone);
	return bDone;
}


//////////////////////////////////////////////////////////////////////////
//
// PointTo for what holds the object system's lock already: the constructors and assignments, which read what they
// are to point at under it.
//
//////////////////////////////////////////////////////////////////////////
void CPointer::PointToInside(CEmbeddedObject* pcNewObject)
{
	CEmbeddedObject*	pcOldObject;
	CPointer			cHoldEmbedding;

	//A Ptr field reached through a NULL Ptr (pNull->mpField = p) is near address 0.  DereferenceArrow has already
	//logged the NULL dereference, so the assignment is dropped rather than crashing.  See
	//TestObjectAllocatorAssignmentToNullObject.
	if ((char*)this < CORRUPT_MEMORY)
	{
		return;
	}

	if (mpcObject != pcNewObject)
	{
		pcOldObject = mpcObject;
		mpcObject = pcNewObject;

		if (IsEmbeddingAllocatedInObjects())
		{
			if (pcOldObject)
			{
				//Removing the heap from on the old object can free the embedding (and this pointer) if nothing else holds the embedding.
				//Hold it until this method is done; it is freed when cHoldEmbedding is destroyed.
				if (!IsEmbeddingHeld())
				{
					cHoldEmbedding.PointToInside(mpcEmbedding);
				}

				if (mpcObject)
				{
					mpcObject->AddHeapFrom(mpcEmbedding, false);
					mpcObject->SetDirty(true);
				}
				pcOldObject->SetDirty(true);
				pcOldObject->RemoveHeapFromTryFree(mpcEmbedding);
			}
			else if (mpcObject)
			{
				mpcObject->AddHeapFrom(mpcEmbedding, true);
				mpcObject->SetDirty(true);
			}
			mpcEmbedding->SetDirty(true);
		}
		else
		{
			//Add the new stack from before removing the old one.  The new object may only be reachable through the old
			//object (p = p->mpNext) and would otherwise be freed with it.
			if (mpcObject)
			{
				mpcObject->AddStackFrom(this, false);
			}

			if (pcOldObject)
			{
				pcOldObject->RemoveStackFromTryFree(this);
			}
		}
	}

	if (mpcObject)
	{
		mpcObject->LogicalDirty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsEmbeddingHeld(void) const
{
	//An object attached to the root, or pointed to from the stack, cannot be freed by changing one of its own pointers.
	//Anything else (e.g. after Return()) is only alive because nothing has tried to free it yet.

	CBaseObject*	pcContainer;

	pcContainer = mpcEmbedding->GetEmbeddingContainer();
	if (pcContainer->GetDistToRoot() >= ROOT_DIST_TO_ROOT)
	{
		return true;
	}
	return pcContainer->HasStackFroms();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::PointToNull(void)
{
	PointTo(NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CPointer::Return(void)
{
	OBJECTS_WRITE();

	CEmbeddedObject*	pOldObject;

	//Call this method when you need to return a pointers object but no other 
	//pointers point to the object and this pointers destructor must not destroy the object.

	pOldObject = mpcObject;
	if (pOldObject)
	{
		pOldObject->UnlinkStackFrom(this);
		mpcObject = NULL;
	}
	return pOldObject;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsNotNull(void) const
{
	return mpcObject != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsNull(void) const
{
	return mpcObject == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsNotEqual(CPointer& pPointer)
{
	return mpcObject != pPointer.Object();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsEqual(CPointer& pPointer)
{
	return mpcObject == pPointer.Object();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsEqual(CEmbeddedObject* pcObject)
{
	return mpcObject == pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer* CPointer::This(void)
{
	//The pointer's own address.  operator& gives the address of the object pointed to, so M_Pointer in a Class() and
	//anything else that needs the CPointer itself uses this.
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CPointer::Object(void) const
{
	return mpcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject** CPointer::ObjectPtr(void)
{
	return &mpcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CPointer::BaseObject(void)
{
	if (mpcObject && mpcObject->IsBaseObject())
	{
		return (CBaseObject*)mpcObject;
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
CObject* CPointer::Embedding(void)
{
	return mpcEmbedding;
}


//////////////////////////////////////////////////////////////////////////
//
// The object, loaded if it is a hollow (the pointer then points to the
// loaded object).  NULL for a NULL pointer, or a hollow that can't be
// loaded: the pointer is then left pointing to the hollow, to try again.
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CPointer::Dereference(void)
{
	CEmbeddedObject*	pcLoaded;

	if (mpcObject == NULL)
	{
		return NULL;
	}

	//Written only when it pointed at a hollow that has now been loaded: using a pointer that two threads share (a
	//field of an object they both use) mustn't write to it.
	pcLoaded = mpcObject->Dehollow();
	if ((pcLoaded != NULL) && (pcLoaded != mpcObject))
	{
		mpcObject = pcLoaded;
	}
	return pcLoaded;
}


//////////////////////////////////////////////////////////////////////////
//
// Dereference for operator->: dereferencing a NULL pointer is an error.
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CPointer::DereferenceArrow(void)
{
	if (mpcObject == NULL)
	{
		gcLogger.Error2(__METHOD__, " Attempted to dereference NULL Pointer.", NULL);
		return NULL;
	}
	return Dereference();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPointer::MorphInto(CEmbeddedObject* pcNew)
{
	OBJECTS_WRITE();

	CObjectRemapFrom	cRemapper;
	size				iCount;

	iCount = cRemapper.Remap(mpcObject, pcNew);

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsHollow(void) const
{
	if (mpcObject)
	{
		return mpcObject->IsHollow();
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::Load(CObjectReader* pcFile)
{
	if (mpcObject)
	{
		if (!mpcObject->IsHollow())
		{
			return mpcObject->Load(pcFile);
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPointer::GetDistToRoot(void) const
{
	OBJECTS_READ();

	return mpcObject ? mpcObject->GetDistToRoot() : UNATTACHED_DIST_TO_ROOT;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CPointer::GetIndex(void) const
{
	if (mpcObject)
	{
		return mpcObject->GetIndex();
	}
	else
	{
		return NULL_O_INDEX;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPointer::GetName(void) const
{
	if (mpcObject)
	{
		return mpcObject->GetName();
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
bool CPointer::IsNamed(void) const
{
	if (mpcObject)
	{
		return mpcObject->IsNamed();
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CPointer::ClassName(void)
{
	if (mpcObject)
	{
		return mpcObject->ClassName();
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
CClass* CPointer::GetClass(void) const
{
	if (mpcObject)
	{
		return mpcObject->GetClass();
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
bool CPointer::IsDirty(void) const
{
	OBJECTS_READ();

	if (mpcObject)
	{
		return mpcObject->IsDirty();
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::LogicalDirty(void)
{
	if (mpcObject)
	{
		mpcObject->LogicalDirty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsEmbeddingAllocatedInObjects(void) const
{
	if (mpcEmbedding)
	{
		return mpcEmbedding->IsAllocatedInObjects();
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::Kill(void)
{
	//This method exists so that its object can be killed without invoking -> and potentially loading it first.
	if (mpcObject)
	{
		mpcObject->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::AssignObject(CEmbeddedObject* pcObject)
{
	//A pointer that points at nothing yet is pointed at pcObject as any is, which a reader may do.
	if (!OBJECTS_LOCKING())
	{
		AssignObjectInside(pcObject);
	}
	else if ((mpcObject != NULL) || !PointToReading(&pcObject))
	{
		ObjectsLock();
		AssignObjectInside(pcObject);
		ObjectsUnlock();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// AssignObject for what holds the object system's lock already: what reads a pointer out of the object system's own
// tables (the memory index, a weak reference) and must hold the object before the lock is let go.
//
//////////////////////////////////////////////////////////////////////////
void CPointer::AssignObjectInside(CEmbeddedObject* pcObject)
{
	OBJECTS_INSIDE();

	mpcObject = pcObject;
	if (mpcObject)
	{
		mpcObject->AddStackFrom(this, false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::AddHeapFrom(CBaseObject* pcFrom)
{
	OBJECTS_INSIDE();

	if (mpcObject)
	{
		mpcObject->AddHeapFrom(pcFrom, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::DumpFroms(void)
{
	CBaseObject* pvBaseObject = BaseObject();

	if (pvBaseObject)
	{
		pvBaseObject->DumpFroms();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::DumpPointerTos(void)
{
	CBaseObject* pvBaseObject = BaseObject();

	if (pvBaseObject)
	{
		pvBaseObject->DumpPointerTos();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::Dump(void)
{
	CChars			sz;
	CBaseObject*	pvBaseObject = BaseObject();

	if (pvBaseObject)
	{
		pvBaseObject->Dump();
	}
	else
	{
		sz.Init("NULL\n");
		sz.Dump();
		sz.Kill();
	}
}

