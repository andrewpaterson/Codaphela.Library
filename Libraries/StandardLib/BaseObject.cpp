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
#include "BaseLib/Logger.h"
#include "BaseLib/Logger.h"
#include "BaseLib/Float32Vec2.h"
#include "BaseLib/Float32Vec3.h"
#include "BaseLib/Float32Vec4.h"
#include "BaseLib/Float4x4.h"
#include "ObjectWriter.h"
#include "ObjectReader.h"
#include "Objects.h"
#include "DistCalculator.h"
#include "Classes.h"
#include "Primitive.h"
#include "KillOrderCalculator.h"
#include "BaseObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject::CBaseObject()
{
	// The Constructor code is only called if an Object is allocated on the stack.
	mpcObjectsThisIn = NULL;
	mpcClass = NULL;
	miDistToRoot = UNATTACHED_DIST_TO_ROOT;
	muiWalkStamp = 0;
	moi = INVALID_O_INDEX;
	mon._Init();
	muiFlags = OBJECT_FLAGS_CALLED_CONSTRUCTOR;
	muiNumEmbedded = 0;
	muiPreInits = 0;
	muiPostInits = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject::~CBaseObject()
{
	//All the CBaseObject is pushed into its immediate sub-classes (anything : public CBaseObject).  
	//  CCollection
	//  CHollowObject
	//  CObject
	//This is because FreePointers() is virtual and overridden in at least Object.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Allocate(CObjects* pcObjects)
{
	//Allocate is only called if an object is allocated in Objects.
	CEmbeddedObject::Allocate(pcObjects);
	mpcObjectsThisIn = pcObjects;
	muiWalkStamp = 0;
	SetFlag(OBJECT_FLAGS_CALLED_ALLOCATE, true);
	PreClass();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PreClass(void)
{
	CClasses*		pcClasses;

	if (muiFlags & OBJECT_FLAGS_FREED)
	{
		FailCantInitFreedObject(__METHOD__);
		return;
	}

	if (!HasClass())
	{
		SetFlag(OBJECT_FLAGS_CALLED_CLASS, true);

		PreFields();

		pcClasses = GetClasses();
		CompleteClass(pcClasses);
		EmbedFields();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PreFields(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CBaseObject::CompleteClass(CClasses* pcClasses)
{
	CClass*		pcClass;
	const char* szClassName;

	szClassName = ClassName();
	pcClass = pcClasses->Get(szClassName);
	if (!pcClass)
	{
		pcClass = pcClasses->Add(szClassName, ClassSize());
		if (!pcClass)
		{
			return NULL;
		}
	}
	SetClass(pcClass);
	if (!pcClass->IsComplete())
	{
		Class();
		pcClass->Complete();
	}
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetClass(CClass* pcClass)
{
	if (!mpcClass)
	{
		mpcClass = pcClass;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Class is already set.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PreInit(void)
{
	CBaseObject*	pcContainer;

	if (IsInitialised())
	{
		FailExpectedToNotBeInitialised(__METHOD__);
		return;
	}

	PreClass();

	pcContainer = GetEmbeddingContainer();
	pcContainer->ContainerPreInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PostInit(void)
{
	CBaseObject*	pcContainer;

	pcContainer = GetEmbeddingContainer();
	pcContainer->ContainerPostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ContainerPreInit(void)
{
	if (IsEmbedded())
	{
		FailNotExpectedToBeEmbedded(__METHOD__);
		return;
	}

	muiPreInits++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ContainerPostInit(void)
{
	CChars	sz;

	if (IsEmbedded())
	{
		FailNotExpectedToBeEmbedded(__METHOD__);
		return;
	}

	if (IsInitialised())
	{
		sz.Init();
		gcLogger.Error2(__METHOD__, " Init has already been called on object[", PrintIdentifier(&sz), "].", NULL);
		sz.Kill();
		return;
	}

	muiPostInits++;
	if (muiPreInits == muiPostInits)
	{
		Initialised();
	}		
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Initialised(void)
{
	SetFlag(OBJECT_FLAGS_CALLED_KILL, false);
	SetFlag(OBJECT_FLAGS_CALLED_INIT, true);
	LogicalDirty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Kill(void)
{
	//This method is for the user to forcibly kill an object.
	//It is not called internally except by ~Object().
	//It is the equivalent of setting the last pointer to the object to NULL.

	bool		bHeapFromChanged;
#ifdef _DEBUG
	CObjects*	pcObjectsThisIn;
#endif // _DEBUG

	if (IsKilled())
	{
		return;
	}

#ifdef _DEBUG
	pcObjectsThisIn = GetObjectsThisIn();
#endif // _DEBUG

	SetFlag(OBJECT_FLAGS_CALLED_KILL, true);
	SetFlag(OBJECT_FLAGS_CALLED_INIT, false);

	bHeapFromChanged = HasHeapFroms();
	KillInternal(bHeapFromChanged, true);

#ifdef _DEBUG
	if (pcObjectsThisIn)
	{
		pcObjectsThisIn->ValidateObjectsConsistency();
	}
#endif // _DEBUG
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::KillInternal(bool bHeapFromChanged, bool bFailNotEmbedded)
{
	if ((bFailNotEmbedded) && IsEmbedded())
	{
		FailNotExpectedToBeEmbedded(__METHOD__);
		return;
	}
	

	CDistCalculator			cDistCalculator;
	CArrayBaseObjectPtr*	papcKilled;
	CArrayHeapFromPointer	acHeapFromPointers;
	SHeapFromPointer*		psHeapFromPointer;
	CBaseObject*			pcFromContainer;
	size					i;
	size					uiNumEmbedded;
	size					uiNumPointers;

	if (IsAllocatedInObjects())
	{
		RemoveAllStackFroms();

		//Remember the pointers to this before they are cleared.
		acHeapFromPointers.Init();
		uiNumEmbedded = NumEmbedded();
		for (i = 0; i < uiNumEmbedded; i++)
		{
			GetEmbeddedObject(i)->GetHeapFromPointers(&acHeapFromPointers);
		}

		RemoveAllHeapFroms();

		cDistCalculator.Init();
		papcKilled = cDistCalculator.Calculate(this, bHeapFromChanged);

		//Put back the pointers from Objects that are killed with this.  They must still be valid when those Objects are freed, as they are in TryFree.
		uiNumPointers = acHeapFromPointers.NumElements();
		for (i = 0; i < uiNumPointers; i++)
		{
			psHeapFromPointer = acHeapFromPointers.Get(i);
			pcFromContainer = psHeapFromPointer->pcFrom->GetEmbeddingContainer();
			if (papcKilled->ContainsVoidPtr(pcFromContainer))
			{
				psHeapFromPointer->pcPointer->UnsafePointTo(psHeapFromPointer->pcTo);
				psHeapFromPointer->pcTo->AddHeapPointerForRemap(psHeapFromPointer->pcFrom);
			}
		}
		acHeapFromPointers.Kill();

		//This can be freed by RemoveInKillOrder.
		mpcObjectsThisIn->RemoveInKillOrder(papcKilled);
		cDistCalculator.Kill();
	}
	else
	{
		if (IsNesPointedTo())
		{
			//Objects that depend on this to be freed must be freed before this, as they are when this is allocated in Objects.
			ClearPointerTos(true);  //Handles embedded objects.
			FreeInternal();    //Handles embedded objects (in Object.FreeInternal()).
		}
		else
		{
			FreeInternal();    //Handles embedded objects (in Object.FreeInternal()).
			ClearPointerTos(true);  //Handles embedded objects.
		}

		RemoveAllStackFroms();  //Handles embedded objects (in Object.RemoveAllStackFroms()).
		RemoveAllHeapFroms();   //Handles embedded objects (in Object.RemoveAllHeapFroms()).

		DoneKill(false);    //Handles embedded objects (in Object.DoneKill()).
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsKilled(void)
{
	return FixBool(muiFlags & OBJECT_FLAGS_CALLED_KILL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::CopyFields(CEmbeddedObject* pcOther)
{
	CBaseObject* pcBaseObject;

	pcBaseObject = (CBaseObject*)pcOther;
	moi = pcBaseObject->moi;
	mon = pcBaseObject->mon;
	miDistToRoot = pcBaseObject->miDistToRoot;
	muiWalkStamp = pcBaseObject->muiWalkStamp;
	muiNumEmbedded = pcBaseObject->muiNumEmbedded;
	muiPreInits = 1;
	muiPostInits = 1;
	CEmbeddedObject::CopyFields(pcOther);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::Flush(void)
{
	bool	bResult;
	bool	bCanFindRoot;
	bool	bDirty;

	if (IsEmbedded())
	{
		FailNotExpectedToBeEmbedded(__METHOD__);
		return false;
	}

	bCanFindRoot = GetDistToRoot() != UNATTACHED_DIST_TO_ROOT;
	bDirty = IsDirty();

	if (bDirty)
	{
		if (bCanFindRoot)
		{
			bResult = GetObjectsThisIn()->ForceSave(this);
			SetDirty(false);
			return bResult;
		}
		else
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
void CBaseObject::FreeInternal(void)
{
	//Frees user data.
	Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::DoneKill(bool bAllocatedInObjects)
{
	LOG_OBJECT_DESTRUCTION(this);
	if (mpcObjectsThisIn)
	{
		mpcObjectsThisIn->mDestructionCallback(this);
	}

	if (bAllocatedInObjects)
	{
		SetFlag(OBJECT_FLAGS_FREED, true);
	}

	//Note that FreeIdentifiers cannot be called here because the object is not yet RemovedFromMemory.
}


//////////////////////////////////////////////////////////////////////////
//
// How many objects have heap froms for this: this and the objects embedded
// in it (GetEmbeddedObject), a hollow object's hollow embedded objects too
// (something may point only to one of those).
//
//////////////////////////////////////////////////////////////////////////
size CBaseObject::NumHeapFromHolders(void)
{
	size	uiNumEmbedded;

	//The count is kept once worked out; asking for it is a virtual call for every object walked.
	uiNumEmbedded = muiNumEmbedded;
	if (uiNumEmbedded == 0)
	{
		uiNumEmbedded = NumEmbedded();
	}
	return uiNumEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
// Of the heap froms of this and the objects embedded in it, the first that
// is closest to the root (attached to it), skipping froms whose object has
// the walk stamp uiSkipWalkStamp (0 skips none: a walk's stamp never is).
// The froms are read where they are rather than copied: a hub has
// thousands.
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CBaseObject::GetClosestHeapFrom(uint32 uiSkipWalkStamp)
{
	CEmbeddedObject*	pcHolder;
	CBaseObject*		pcNearest;
	CBaseObject*		pcFrom;
	int					iNearest;
	int					iDist;
	size				uiHolder;
	size				uiNumHolders;
	size				i;
	size				uiNumFroms;

	iNearest = MAX_DIST_TO_ROOT;
	pcNearest = NULL;
	uiNumHolders = NumHeapFromHolders();
	for (uiHolder = 0; uiHolder < uiNumHolders; uiHolder++)
	{
		pcHolder = (uiHolder == 0) ? this : GetEmbeddedObject(uiHolder);
		uiNumFroms = pcHolder->CEmbeddedObject::NumHeapFroms();
		for (i = 0; i < uiNumFroms; i++)
		{
			pcFrom = pcHolder->CEmbeddedObject::GetHeapFrom(i);
			iDist = pcFrom->GetDistToRoot();
			if ((iDist >= 0) && (iDist < iNearest))
			{
				//Asked only of a from that would be nearest: most of a hub's froms aren't attached.
				if ((uiSkipWalkStamp != 0) && (pcFrom->GetEmbeddingContainer()->GetWalkStamp() == uiSkipWalkStamp))
				{
					continue;
				}
				iNearest = iDist;
				pcNearest = pcFrom;
			}
		}
	}
	return pcNearest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CBaseObject::GetClosestFromToRoot(void)
{
	return GetClosestHeapFrom(0);
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the objects holding the heap froms of this and the objects embedded
// in it.
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::AddHeapFromContainers(CArrayBaseObjectPtr* papcFroms)
{
	CEmbeddedObject*	pcHolder;
	size				uiHolder;
	size				uiNumHolders;
	size				i;
	size				uiNumFroms;

	uiNumHolders = NumHeapFromHolders();
	for (uiHolder = 0; uiHolder < uiNumHolders; uiHolder++)
	{
		pcHolder = (uiHolder == 0) ? this : GetEmbeddedObject(uiHolder);
		uiNumFroms = pcHolder->CEmbeddedObject::NumHeapFroms();
		for (i = 0; i < uiNumFroms; i++)
		{
			papcFroms->Add(pcHolder->CEmbeddedObject::GetHeapFrom(i)->GetEmbeddingContainer());
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetFlag(uint16 iFlag, uint16 iFlagValue)
{
	::SetFlagShort(&muiFlags, iFlag, iFlagValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CBaseObject::GetFlags(void)
{
	return muiFlags;
}


//////////////////////////////////////////////////////////////////////////
//
// Follows the closest attached from, then its, and so on, to the root.
// Only that path is tried: it checks the distances, not every path.  The
// objects on it are stamped so a cycle ends it.
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::CanFindRoot(void)
{
	CBaseObject*	pcObject;
	CBaseObject*	pcClosestFrom;
	uint32			uiTested;

	if (IsEmbedded())
	{
		FailNotExpectedToBeEmbedded(__METHOD__);
		return false;
	}

	uiTested = gcObjects.StartWalk() | WALK_STATE_TOUCHED;
	pcObject = this;
	while (!pcObject->IsRoot())
	{
		pcObject->SetWalkStamp(uiTested);
		pcClosestFrom = pcObject->GetClosestHeapFrom(uiTested);
		if (pcClosestFrom == NULL)
		{
			return false;
		}
		pcObject = pcClosestFrom->GetEmbeddingContainer();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::SetDistToRoot(int iDistToRoot)
{
	if (miDistToRoot != iDistToRoot)
	{
		miDistToRoot = iDistToRoot;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::RemoveToFromDontFree(CEmbeddedObject* pcPointedTo)
{
	CBaseObject*	pcBaseObject;

	if ((pcPointedTo) && (pcPointedTo->IsBaseObject()))
	{
		pcBaseObject = (CBaseObject*)pcPointedTo;
		pcBaseObject->UnlinkHeapFrom(this);  //If the object pointed to us is also being killed then we needed remove our from from it.
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::StopDestructor(void)
{
	muiFlags = OBJECT_FLAGS_CALLED_CONSTRUCTOR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CBaseObject::SerialisedSize(void)
{
	//This method should only be used for testing.  It should be a helper somewhere else.
	CObjectWriter	cWriter;
	lsize			iLength;
	CMemoryFile		cMemory;

	cMemory.Init();
	cMemory.Open(EFM_ReadWrite_Create);
	cWriter.Init(&cMemory, NULL);
	cWriter.Write(this);
	cWriter.WriteHeapFroms(this);
	iLength = cMemory.GetBufferSize();
	cMemory.Close();
	cWriter.Kill();
	cMemory.Kill();
	return iLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsRoot(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsNamed(void)
{
	return !mon.Empty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CBaseObject::PrintIdentifier(CChars* psz)
{
	char*	szName;

	psz->Append(ClassName());
	psz->Append(":");
	psz->Append(GetIndex());
	szName = GetName();
	if (!StrEmpty(szName))
	{
		psz->Append(":\"");
		psz->Append(szName);
		psz->Append("\"");
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsSubRoot(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsInvalidated(void)
{
	return FixBool(muiFlags & OBJECT_FLAGS_INVALIDATED);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsDirty(void)
{
	return FixBool(muiFlags & OBJECT_FLAGS_DIRTY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::TestedForSanity(void)
{
	return FixBool(muiFlags & OBJECT_FLAGS_TESTED_FOR_SANITY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetDirty(bool bDirty)
{
	SetFlag(OBJECT_FLAGS_DIRTY, bDirty);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::GetDistToRoot(void)
{
	return miDistToRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CBaseObject::GetWalkStamp(void)
{
	return muiWalkStamp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetWalkStamp(uint32 uiWalkStamp)
{
	muiWalkStamp = uiWalkStamp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::InitIdentifiers(const char* szName, OIndex oi)
{
	moi = oi;
	mon.Init(szName);
	return ClipName();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::FreeIdentifiers(void)
{
	moi = INVALID_O_INDEX;
	mon.Kill();
	mon.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsHollow(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CBaseObject::TestGetPointerTo(size iToIndex)
{
	CEmbeddedObject**					ppTo;
	CEmbeddedObject*					pTo;
	CArrayEmbeddedObjectPtr		apcTos;

	apcTos.Init();
	GetPointerTos(&apcTos);
	ppTo = apcTos.SafeGet(iToIndex);
	if (ppTo)
	{
		pTo = *ppTo;
	}
	else
	{
		pTo = NULL;
	}
	apcTos.Kill();
	return pTo;
}


//////////////////////////////////////////////////////////////////////////
//
// Clears this object's pointers, one at a time, unlinking the froms they
// gave.  With bFree what each pointed to is freed if nothing else reaches
// it, before the next is cleared (the ones not cleared yet still hold
// theirs).  Without, nothing is freed: the objects are being killed
// together.  Objects with no pointers (hollow ones) have none to clear.
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::ClearPointerTos(bool bFree)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether one of this object's own pointers (not those of the objects
// embedded in it) points to pcEmbedded.
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::ContainsPointerTo(CEmbeddedObject* pcEmbedded)
{
	CArrayEmbeddedObjectPtr		apcTos;
	size						i;
	size						uiNumTos;
	bool						bContains;

	bContains = false;
	apcTos.Init();
	BaseGetPointerTos(&apcTos);
	uiNumTos = apcTos.NumElements();
	for (i = 0; i < uiNumTos; i++)
	{
		if (apcTos.GetPtr(i) == pcEmbedded)
		{
			bContains = true;
			break;
		}
	}
	apcTos.Kill();
	return bContains;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::Save(CObjectWriter* pcFile)
{
	return true;
} 


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::SaveManaged(CObjectWriter* pcFile)
{
	bool	bResult;

	bResult = SaveEmbeddedObjectsManaged(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = SavePointers(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = SavePrimitives(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = SaveUnmanaged(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = Save(pcFile);
	if (!bResult)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::SaveHeapFroms(CObjectWriter* pcFile)
{
	bool			bResult;
	size			i;
	size			uiNumElements;
	CBaseObject*	pcHeapFrom;

	//Distances to the root are not saved.  They describe the objects in memory and are worked out from the froms there.
	bResult = SaveEmbeddedObjectsHeapFroms(pcFile);
	ReturnOnFalse(bResult);

	uiNumElements = CEmbeddedObject::NumHeapFroms();
	bResult = pcFile->WriteSize(uiNumElements);

	ReturnOnFalse(bResult);

	for (i = 0; i < uiNumElements; i++)
	{
		pcHeapFrom = CEmbeddedObject::GetHeapFrom(i);
		bResult = pcFile->WriteDependent(pcHeapFrom, false);
		ReturnOnFalse(bResult);
	}
	
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::SaveEmbeddedObjectsManaged(CObjectWriter* pcFile)
{
	size					uiNumFields;
	CEmbeddedObjectField**	ppacEmbeddedObjectFields;
	CArrayVoidPtr*			papv;
	size					i;
	CBaseObject*			pcEmbeddedObject;
	bool					bResult;

	papv = mpcClass->GetEmbeddedObjectFields();
	ppacEmbeddedObjectFields = (CEmbeddedObjectField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcEmbeddedObject = ppacEmbeddedObjectFields[i]->GetEmbeddedObject(this);
		bResult = pcEmbeddedObject->SaveManaged(pcFile);
		if (!bResult)
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
bool CBaseObject::SaveEmbeddedObjectsHeapFroms(CObjectWriter* pcFile)
{
	size					uiNumFields;
	CEmbeddedObjectField**	ppacEmbeddedObjectFields;
	CArrayVoidPtr*			papv;
	size					i;
	CBaseObject*			pcEmbeddedObject;
	bool					bResult;

	papv = mpcClass->GetEmbeddedObjectFields();
	ppacEmbeddedObjectFields = (CEmbeddedObjectField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcEmbeddedObject = ppacEmbeddedObjectFields[i]->GetEmbeddedObject(this);
		bResult = pcEmbeddedObject->SaveHeapFroms(pcFile);
		if (!bResult)
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
bool CBaseObject::SavePointers(CObjectWriter* pcFile)
{
	size				uiNumFields;
	CPointerField**		ppacPointerFields;
	CArrayVoidPtr*		papv;
	size				i;
	CPointer*			pcPointer;
	bool				bResult;

	papv = mpcClass->GetPointerFields();
	ppacPointerFields = (CPointerField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcPointer = ppacPointerFields[i]->GetPointer(this);
		bResult = pcFile->WritePointer(pcPointer);
		if (!bResult)
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
bool CBaseObject::SavePrimitives(CObjectWriter* pcFile)
{
	size				uiNumFields;
	CPrimitiveField**	ppacPrimitiveFields;
	CArrayVoidPtr*		papv;
	size				i;
	bool				bResult;
	CPrimitiveField*	pcPrimitiveField;
	SDataIO*			psIO;
	void*				pvPrimitive;

	papv = mpcClass->GetPrimitiveFields();
	ppacPrimitiveFields = (CPrimitiveField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcPrimitiveField = ppacPrimitiveFields[i];
		psIO = pcPrimitiveField->GetDataIO();
		pvPrimitive = pcPrimitiveField->GetValue(this);
		bResult = (((SDataTypeIO*)pvPrimitive)->*(psIO->fWriter))(pcFile);
		ReturnOnFalse(bResult);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::SaveUnmanaged(CObjectWriter* pcFile)
{
	size				uiNumFields;
	CArrayVoidPtr*		papv;
	size				i;
	CUnmanagedField*	pcUnmanagedField;
	bool				bResult;
	size				uiCount;
	size				uiSize;
	void*				pvUnmanaged;

	papv = mpcClass->GetUnmanagedFields();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcUnmanagedField = (CUnmanagedField*)papv->GetPtr(i);
		pvUnmanaged = pcUnmanagedField->GetData(this);
		uiCount = pcUnmanagedField->GetLength();
		uiSize = pcUnmanagedField->GetSizeOf();
		bResult = pcFile->WriteData(pvUnmanaged, (lsize)uiCount * uiSize);

		if (!bResult)
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
bool CBaseObject::Load(CObjectReader* pcFile)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::LoadManaged(CObjectReader* pcFile)
{
	bool	bResult;

	bResult = LoadEmbeddedObjectsManaged(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = LoadPointers(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = LoadPrimitives(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = LoadUnmanaged(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = Load(pcFile);
	if (!bResult)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::LoadEmbeddedObjectsManaged(CObjectReader* pcFile)
{
	size					uiNumFields;
	CEmbeddedObjectField**	ppacEmbeddedObjectFields;
	CArrayVoidPtr*			papv;
	size					i;
	CBaseObject*			pcEmbeddedObject;
	bool					bResult;

	papv = mpcClass->GetEmbeddedObjectFields();
	ppacEmbeddedObjectFields = (CEmbeddedObjectField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcEmbeddedObject = ppacEmbeddedObjectFields[i]->GetEmbeddedObject(this);
		bResult = pcEmbeddedObject->LoadManaged(pcFile);
		if (!bResult)
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
bool CBaseObject::LoadPointers(CObjectReader* pcFile)
{
	size				uiNumFields;
	CPointerField*		pcPointerField;
	CArrayVoidPtr*		papv;
	size				i;
	CPointer*			pcPointer;
	bool				bResult;

	papv = mpcClass->GetPointerFields();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcPointerField = (CPointerField*)papv->GetPtr(i);
		pcPointer = pcPointerField->GetPointer(this);
		bResult = pcFile->ReadPointer(pcPointer);
		if (!bResult)
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
bool CBaseObject::LoadPrimitives(CObjectReader* pcFile)
{
	size				uiNumFields;
	CPrimitiveField**	ppacPrimitiveFields;
	CArrayVoidPtr*		papv;
	size				i;
	bool				bResult;
	CPrimitiveField*	pcPrimitiveField;
	SDataIO*			psIO;
	void*				pvPrimitive;

	papv = mpcClass->GetPrimitiveFields();
	ppacPrimitiveFields = (CPrimitiveField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcPrimitiveField = ppacPrimitiveFields[i];
		psIO = pcPrimitiveField->GetDataIO();
		pvPrimitive = pcPrimitiveField->GetValue(this);
		bResult = (((SDataTypeIO*)pvPrimitive)->*(psIO->fReader))(pcFile);

		if (!bResult)
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
bool CBaseObject::LoadUnmanaged(CObjectReader* pcFile)
{
	size					uiNumFields;
	CArrayVoidPtr*			papv;
	size					i;
	CUnmanagedField*		pcUnmanagedField;
	bool					bResult;
	size					uiCount;
	size					uiSize;
	void*					pvUnmanaged;

	papv = mpcClass->GetUnmanagedFields();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcUnmanagedField = (CUnmanagedField*)papv->GetPtr(i);
		pvUnmanaged = pcUnmanagedField->GetData(this);
		uiCount = pcUnmanagedField->GetLength();
		uiSize = pcUnmanagedField->GetSizeOf();
		bResult = pcFile->ReadData(pvUnmanaged, (lsize)uiCount * uiSize);

		if (!bResult)
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
bool CBaseObject::LoadHeapFroms(CObjectReader* pcFile)
{
	bool				bResult;
	size				i;
	size				uiNumElements;

	//The distance to the root is not loaded.  It is the one the replaced hollow had from its froms in memory (unattached if none).
	bResult = LoadEmbeddedObjectsHeapFroms(pcFile);
	ReturnOnFalse(bResult);

	bResult = pcFile->ReadSize(&uiNumElements);
	ReturnOnFalse(bResult);

	for (i = 0; i < uiNumElements; i++)
	{
		bResult = pcFile->ReadReverseDependent(NULL, this);
		ReturnOnFalse(bResult);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::LoadEmbeddedObjectsHeapFroms(CObjectReader* pcFile)
{
	size					uiNumFields;
	CEmbeddedObjectField**	ppacEmbeddedObjectFields;
	CArrayVoidPtr*			papv;
	size					i;
	CBaseObject*			pcEmbeddedObject;
	bool					bResult;

	papv = mpcClass->GetEmbeddedObjectFields();
	ppacEmbeddedObjectFields = (CEmbeddedObjectField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcEmbeddedObject = ppacEmbeddedObjectFields[i]->GetEmbeddedObject(this);
		bResult = pcEmbeddedObject->LoadHeapFroms(pcFile);
		if (!bResult)
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
OIndex CBaseObject::GetIndex(void)
{
	return moi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsBaseObject(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CBaseObject::NumEmbedded(void)
{
	SetFlagNumEmbedded(1);
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CBaseObject::GetNumEmbeddedFromFlags(void)
{
	return muiNumEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetFlagNumEmbedded(size iNumEmbedded)
{
	muiNumEmbedded = (uint16)iNumEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ClearFlagNumEmbedded(void)
{
	muiNumEmbedded = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CBaseObject::TestGetNumEmbeddedFromFlags(void)
{
	return GetNumEmbeddedFromFlags();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObjects* CBaseObject::GetObjectsThisIn(void)
{
	CBaseObject*	pcBaseObject;
	CObjects*		pcObjects;

	pcBaseObject = GetEmbeddingContainer();
	pcObjects = pcBaseObject->mpcObjectsThisIn;
	return pcObjects;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClasses* CBaseObject::GetClasses(void)
{
	CObjects*	pcObjects;

	pcObjects = GetObjects();
	if (pcObjects)
	{
		return pcObjects->GetClasses();
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
bool CBaseObject::HasClass(void)
{
	return FixBool(muiFlags & OBJECT_FLAGS_CALLED_CLASS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsInitialised(void)
{
	return FixBool(muiFlags & OBJECT_FLAGS_CALLED_INIT);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ReplaceOneWithX(char* szDest, char* szMask)
{
	size i;

	for (i = 0; ; i++)
	{
		if ((szMask[i] == '\0') || (szDest[i] == '\0'))
		{
			return;
		}

		if (szMask[i] == '1')
		{
			szDest[i] ='X';
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CBaseObject::GetClass(void)
{
	return mpcClass;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::DumpFroms(void)
{
	CChars				sz;
	size				i;
	size				iNumEmbedded;
	CEmbeddedObject*	pcEmbedded;
	size				j;
	size				iNumHeapFroms;
	CEmbeddedObject*	pcFromObject;
	size				iLength;
	CChars				szLine;

	sz.Init();

	sz.Append("-- ");
	PrintObject(&sz);
	sz.Append(" --\n");
	iLength = sz.Length()-1;

	szLine.Init('-', iLength);
	szLine.AppendNewLine();

	sz.Insert(0, &szLine);
	sz.Append("Total Heap Froms [");
	sz.Append(NumHeapFroms());
	sz.Append("], ");

	sz.Append("Stack Froms [");
	sz.Append(NumStackFroms());
	sz.Append("]\n");

	iNumEmbedded = NumEmbedded();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = GetEmbeddedObject(i);
		iNumHeapFroms = pcEmbedded->CEmbeddedObject::NumHeapFroms();
		sz.Append("Embedded ");
		sz.Append(i);
		sz.Append(" Heap Froms [");
		sz.Append(iNumHeapFroms);
		sz.Append("], ");
		sz.Append("Stack Froms [");
		sz.Append(pcEmbedded->CEmbeddedObject::NumStackFroms());
		sz.Append("]\n");

		for (j = 0; j < iNumHeapFroms; j++)
		{
			pcFromObject = pcEmbedded->GetHeapFrom(j);
			sz.Append(" ");
			pcFromObject->PrintObject(&sz);
			sz.AppendNewLine();
		}
	}

	sz.Append(&szLine);
	szLine.Kill();

	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::DumpPointerTos(void)
{
	CChars								sz;
	size								i;
	size								iNumEmbedded;
	CEmbeddedObject*					pcEmbedded;
	size								j;
	size								iNumTos;
	CEmbeddedObject**					ppcToObject;
	size								iLength;
	CChars								szLine;
	CArrayEmbeddedObjectPtr		acTos;
	CBaseObject*						pcToObject;
	size								iTotalTos;

	sz.Init();

	sz.Append("-- ");
	PrintObject(&sz);
	sz.Append(" --\n");
	iLength = sz.Length()-1;

	szLine.Init('-', iLength);
	szLine.AppendNewLine();

	iTotalTos = 0;
	iNumEmbedded = NumEmbedded();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = GetEmbeddedObject(i);
		iTotalTos += pcEmbedded->NumPointerTos();
	}

	sz.Insert(0, &szLine);
	sz.Append("Total Tos [");
	sz.Append(iTotalTos);
	sz.Append("]\n");

	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = GetEmbeddedObject(i);
		acTos.Init();
		pcEmbedded->BaseGetPointerTos(&acTos);
		iNumTos = acTos.NumElements();
		sz.Append("Embedded ");
		sz.Append(i);
		sz.Append(" Tos [");
		sz.Append(iNumTos);
		sz.Append("]\n");

		for (j = 0; j < iNumTos; j++)
		{
			ppcToObject = acTos.Get(j);  //A pointed to never comes back NULL.
			sz.Append(" ");
				
			if ((*ppcToObject)->IsBaseObject())
			{
				pcToObject = (CBaseObject*)*ppcToObject;
				pcToObject->PrintObject(&sz);
			}
			else
			{
				sz.Append("HollwEmbeddedObject");
			}
			sz.AppendNewLine();
		}

		acTos.Kill();
	}

	sz.Append(&szLine);
	szLine.Kill();

	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Print(CChars* psz)
{
	PrintState(psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PrintState(CChars* psz)
{
	PrintIdentifier(psz);
	psz->Append(" R");
	if (miDistToRoot >= 0)
	{
		psz->Append(miDistToRoot);
	}
	else if (miDistToRoot == UNATTACHED_DIST_TO_ROOT)
	{
		psz->Append("u");
	}
	else if (miDistToRoot == CLEARED_DIST_TO_ROOT)
	{
		psz->Append("c");
	}
	else
	{
		psz->Append("x");
	}
	psz->Append(" S");
	if (!IsAllocatedInObjects())
	{
		psz->Append("0");
	}
	else
	{
		psz->Append("u");
	}

	psz->Append(" (");
	PrintFlagsShorthand(psz);
	psz->Append(")");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PrintFlagsShorthand(CChars* psz)
{
	CChars	sz;
	size	uiEnd;
	size	ui;
	char	c;
	bool	bPrintNext;

	sz.Init();
	PrintFlags(&sz);

	bPrintNext = true;
	uiEnd = sz.Length();
	for (ui = 0; ui < uiEnd; ui++)
	{
		c = sz.GetChar(ui);
		if (c == ',')
		{
			psz->Append(c);
		}
		if (c == ' ')
		{
			psz->Append(c);
			bPrintNext = true;
		}
		else if (c == '_')
		{
			bPrintNext = true;
		}
		else
		{
			if (bPrintNext)
			{
				psz->Append(c);
				bPrintNext = false;
			}
		}
	}

	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PrintFlags(CChars* psz)
{
	if (muiFlags & OBJECT_FLAGS_INVALIDATED)
	{
		psz->Append("INVALIDATED, ");
	}
	if (muiFlags & OBJECT_FLAGS_CALLED_CONSTRUCTOR)
	{
		psz->Append("CALLED_CONSTRUCTOR, ");
	}
	if (muiFlags & OBJECT_FLAGS_DIRTY)
	{
		psz->Append("DIRTY, ");
	}
	if (muiFlags & OBJECT_FLAGS_FREED)
	{
		psz->Append("FREED, ");
	}
	if (muiFlags & OBJECT_FLAGS_DUMPED)
	{
		psz->Append("DUMPED, ");
	}
	if (muiFlags & OBJECT_FLAGS_TESTED_FOR_SANITY)
	{
		psz->Append("TESTED_FOR_SANITY, ");
	}
	if (muiFlags & OBJECT_FLAGS_CALLED_ALLOCATE)
	{
		psz->Append("CALLED_ALLOCATE, ");
	}
	if (muiFlags & OBJECT_FLAGS_CALLED_INIT)
	{
		psz->Append("CALLED_INIT, ");
	}
	if (muiFlags & OBJECT_FLAGS_CALLED_KILL)
	{
		psz->Append("CALLED_KILL, ");
	}
	if (muiFlags & OBJECT_FLAGS_CALLED_CLASS)
	{
		psz->Append("CALLED_CLASS, ");
	}

	if (psz->EndsWith(", "))
	{
		psz->RemoveFromEnd(2);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::DumpFlags(void)
{
	CChars	sz;

	sz.Init();
	PrintFlags(&sz);
	sz.AppendNewLine();
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Dump(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateFlagNotSet(uint16 iFlag, const char* szFlag)
{
	if (muiFlags & iFlag)
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} should not have flag [", szFlag,"] set.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateFlagSet(uint16 iFlag, const char* szFlag)
{
	if (!(muiFlags & iFlag))
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} should have flag [", szFlag,"] set (but it is not).", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Checks the distance to the root against the froms', without walking to
// the root (a walk that scanned hubs' froms for every object): an attached
// object's closest attached from is exactly one closer to the root, and an
// unattached object has no attached froms.  Checked for every object, that
// is the same as each attached object finding the root, and its distance
// being the shortest.
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateCanFindRoot(void)
{
	if (IsEmbedded())
	{
		FailNotExpectedToBeEmbedded(__METHOD__);
		return;
	}

	CEmbeddedObject*	pcClosest;
	int					iClosestDistToRoot;

	if (miDistToRoot > ROOT_DIST_TO_ROOT)
	{
		pcClosest = GetClosestFromToRoot();
		if (pcClosest == NULL)
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has a positive dist to root and should be able to find the Root object.", NULL);
			return;
		}
		iClosestDistToRoot = pcClosest->GetDistToRoot();
		if (iClosestDistToRoot != miDistToRoot - 1)
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has dist to root [", IntToString(miDistToRoot), "] but its closest from has [", IntToString(iClosestDistToRoot), "].", NULL);
		}
	}
	else if (miDistToRoot == ROOT_DIST_TO_ROOT)
	{
		if (!IsRoot())
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has a dist to root [0] but is not the Root object.", NULL);
		}
	}
	else if (miDistToRoot == UNATTACHED_DIST_TO_ROOT)
	{
		pcClosest = GetClosestFromToRoot();
		if (pcClosest != NULL)
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has an [UNATTACHED_DIST_TO_ROOT] dist to root should not be able to find the Root object.", NULL);
		}
	}
	else if (miDistToRoot == CLEARED_DIST_TO_ROOT)
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} should not have dist to root [CLEARED_DIST_TO_ROOT].", NULL);
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} should not have dist to root [", IntToString(miDistToRoot), "].", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateContainerFlag(void)
{
	CChars	sz;
	uint16	iEmbeddedFlags;
	uint16	iThisFlags;
	uint16	iIgnoredFlags;
	char*	szEmbeddedFlags;
	char*	szFlags;
	char*	szIgnoredFlags;

	if (IsEmbedded())
	{
		iIgnoredFlags = OBJECT_FLAGS_CALLED_CLASS;
		iEmbeddedFlags = mpcEmbedded->muiFlags & ~iIgnoredFlags;
		iThisFlags = muiFlags & ~iIgnoredFlags;
		if ((iEmbeddedFlags) != (iThisFlags))
		{
			sz.Init();
			PrintObject(&sz);
			szFlags = ShortToFlags(muiFlags);
			szEmbeddedFlags = ShortToFlags(mpcEmbedded->muiFlags);
			szIgnoredFlags = ShortToFlags(iIgnoredFlags);
			ReplaceOneWithX(szFlags, szIgnoredFlags);
			ReplaceOneWithX(szEmbeddedFlags, szIgnoredFlags);

			gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should have the same flags [", szFlags ,"] as it's embedding container's flags [", szEmbeddedFlags, "].", NULL);
			sz.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateFlags(void)
{
	ValidateFlagNotSet(OBJECT_FLAGS_FREED, "OBJECT_FLAGS_FREED");
	ValidateFlagNotSet(OBJECT_FLAGS_DUMPED, "OBJECT_FLAGS_DUMPED");
	ValidateFlagSet(OBJECT_FLAGS_CALLED_CONSTRUCTOR, "OBJECT_FLAGS_CALLED_CONSTRUCTOR");
	ValidateFlagSet(OBJECT_FLAGS_CALLED_ALLOCATE, "OBJECT_FLAGS_CALLED_ALLOCATE");

	ValidateContainerFlag();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateAllocation(void)
{
	bool	bAllocateCalled;
	bool	bInObjects;

	bAllocateCalled = FixBool(muiFlags & OBJECT_FLAGS_CALLED_ALLOCATE);
	bInObjects = IsAllocatedInObjects();
	if (bAllocateCalled != bInObjects)
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} should not have flag OBJECT_FLAGS_CALLED_ALLOCATE [", IntToString(bAllocateCalled), "] and be allocated in Objects [0x", PointerToString(GetObjectsThisIn()), "].", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateDistToRoot(void)
{
	CBaseObject*	pcContainer;

	if (!((miDistToRoot >= ROOT_DIST_TO_ROOT) || (miDistToRoot == UNATTACHED_DIST_TO_ROOT)))
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} should not have a dist to root of [", IntToString(miDistToRoot), "].", NULL);
	}

	if (IsEmbedded())
	{
		pcContainer = GetEmbeddingContainer();
		if (pcContainer->GetDistToRoot() != GetDistToRoot())
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} should have a dist to root [", IntToString(miDistToRoot), "] the same as it's embedding object [", IntToString(pcContainer->GetDistToRoot()),"].", NULL);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateIndex(void)
{
	if (IsEmbedded())
	{
		if (moi != INVALID_O_INDEX)
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} should have an Index [", IndexToString(moi), "] of INVALID_O_INDEX [", IndexToString(INVALID_O_INDEX),"].", NULL);
		}
	}
	else
	{
		if (moi == INVALID_O_INDEX)
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} should not have an Index of INVALID_O_INDEX [", IndexToString(INVALID_O_INDEX),"].", NULL);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateObjectsThisIn(void)
{
	if (IsEmbedded())
	{
		if (mpcObjectsThisIn != NULL)
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} should not have ObjectsThisIn [", PointerToString(mpcObjectsThisIn), "] set.", NULL);
		}
	}
	else
	{
		if (mpcObjectsThisIn == NULL)
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} should have ObjectsThisIn [NULL] set.", NULL);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateBaseObjectDetail(void)
{
	ValidateFlags();
	ValidateAllocation();
	ValidateDistToRoot();
	ValidateIndex();
	ValidateObjectsThisIn();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateEmbeddedConsistency(void)
{
	ValidateBaseObjectDetail();
	ValidateFroms();
	ValidatePointerTos();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateObjectIdentifiers(void)
{
	CBaseObject*	pcContainer;
	char*			szName;
	CBaseObject*	pcThis;
	OIndex			oi;

	if (IsNamed())
	{
		if (IsEmbedded())
		{
			pcContainer = GetEmbeddingContainer();

			gcLogger.Error2(__METHOD__, " Named Object {", ObjectToString(this), "} should not have a name as it's embedded in object {", ObjectToString(pcContainer,  false), "}.", NULL);
			return;
		}
		else
		{
			szName = GetName();
			if (!IsAllocatedInObjects())
			{
				gcLogger.Error2(__METHOD__, " Named Object {", ObjectToString(this), "} should not have a name [", szName, "] as it's on the stack.", NULL);
				return;
			}

			pcThis = mpcObjectsThisIn->GetFromMemory(szName);
			if (pcThis != this)
			{
				gcLogger.Error2(__METHOD__, " 'this' Object {", ObjectToString(this), "} does not match the Named Object {", ObjectToString(pcThis), "} in Objects.", NULL);
				return;
			}
		}
	}

	if (!IsEmbedded())
	{
		oi = GetIndex();
		if (IsAllocatedInObjects())
		{
			if (!IsValidIndex(oi))
			{
				gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} must have a valid identifier [", IndexToString(oi), "].", NULL);
				return;
			}
			else
			{
				pcThis = mpcObjectsThisIn->GetFromMemory(oi);
				if (pcThis != this)
				{
					gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} does not match the Object in Objects.", NULL);
					return;
				}
			}
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} should not be on the stack.", NULL);
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::FailHasClassFlag(char* szMethod)
{
	return gcLogger.Error2(szMethod, " Object {", ObjectToString(this), "} does not have Class initialised.  Call PreInit() first in Init().", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::FailHasClassField(char* szMethod)
{
	return gcLogger.Error2(szMethod, " Object {", ObjectToString(this), "} does not have Class field set.", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateInitCalled(void)
{
	if (muiPreInits != muiPostInits)
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has pre-inits [", IntToString(muiPreInits), "] not equal to post inits [", IntToString(muiPostInits), "].  Call PreInit() first in Init().", NULL);
	}
	else if (muiPreInits == 0)
	{
		if (!IsEmbedded())
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has a pre-init of zero.  Call PreInit() first in Init().", NULL);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateKillCalled(void)
{
	if (!(muiFlags & OBJECT_FLAGS_CALLED_KILL))
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has not beel killed.  Ensure sub-classes are declared DESTRUCTABLE().", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateInternalConsistency(void)
{
	CStackPointer*		pcStackPointer;
	SStackPointer*		psStackPointer;
	void*				pvStart;
	void*				pvEnd;
	void*				pvPointer;
	SFreeListIterator	sIter;

	if (IsEmbeddingContainerAllocatedInObjects())
	{
		pvStart = this;
		pvEnd = RemapSinglePointer(pvStart, ClassSize());

		pcStackPointer = gcStackPointers.StartIteration(&sIter);
		while (pcStackPointer)
		{
			psStackPointer = pcStackPointer->Get();
			if (psStackPointer->eType == SPT_Pointer)
			{
				pvPointer = psStackPointer->u.pcPointer;
			}
			else if (psStackPointer->eType == SPT_Collection)
			{
				pvPointer = psStackPointer->u.pcCollection;
			}
			else
			{
				pvPointer = NULL;
			}
			if ((pvPointer >= pvStart) && (pvPointer <= pvEnd))
			{
				gcLogger.Error2(__METHOD__, " Stack Pointer [", PointerToString(pvPointer), "] appears to belong to Object {", ObjectToString(this), "}.  Esnure it is set up in Class().", NULL);
			}
			pcStackPointer = gcStackPointers.Iterate(&sIter);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CBaseObject::GetName(void)
{
	return mon.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsNamed(const char* szName)
{
	if (StrEmpty(szName))
	{
		return mon.Equals(szName);
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
bool CBaseObject::ClipName(void)
{
	bool	bResult;
	CChars	szNewName;

	bResult = true;
	if (mon.Contains("\\") || mon.Length() >= MAX_NAMED_OBJECT_NAME_LENGTH)
	{
		szNewName.Init(mon.Text());
		if (szNewName.Contains("\\"))
		{
			szNewName.Replace("\\", "/");
			bResult = false;
		}
		if (szNewName.Length() >= MAX_NAMED_OBJECT_NAME_LENGTH)
		{
			szNewName.SetLength(MAX_NAMED_OBJECT_NAME_LENGTH - 1);
			bResult = false;
		}
		mon.Set(szNewName.Text());
		szNewName.Kill();
	}

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::HasNes(void)
{
	return mpcClass->HasNes();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::GetNesPointerToThis(CArrayEmbeddedObjectPtrE8* papNesObjectsPointingToThis)
{
	size				i;
	size				iNumEmbedded;
	CEmbeddedObject*	pcEmbedded;
	CEmbeddedObject*	pcNesObject;
	size				iStart;

	iNumEmbedded = NumEmbedded();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = GetEmbeddedObject(i);
		iStart = 0;
		for (;;)
		{
			pcNesObject = pcEmbedded->GetNesPointerFrom(&iStart);
			if (pcNesObject)
			{
				papNesObjectsPointingToThis->Add(pcNesObject);
				continue;
			}
			break;
		} 
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsNesPointedTo(void)
{
	//Returns true if another Object has a Nes pointer to this or to an Object embedded in this.

	size	i;
	size	iNumEmbedded;
	size	iStart;

	iNumEmbedded = NumEmbedded();
	for (i = 0; i < iNumEmbedded; i++)
	{
		iStart = 0;
		if (GetEmbeddedObject(i)->GetNesPointerFrom(&iStart))
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
bool CBaseObject::HasNesPointerToThis(CEmbeddedObject* pcBaseObject)
{
	CArrayVoidPtr*	papPointerFields;
	CPointerField*	pcPointerField;
	size			iNumFields;
	size			i;
	CPointer*		pcPointer;

	papPointerFields = mpcClass->GetPointerFields();
	iNumFields = papPointerFields->NumElements();
	for (i = 0; i < iNumFields; i++)
	{
		pcPointerField = (CPointerField*)papPointerFields->GetPtr(i);
		if (pcPointerField->IsDoNotFreePointedTo())
		{
			pcPointer = pcPointerField->GetPointer(this);
			if (pcPointer->IsEqual(pcBaseObject))
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
void CBaseObject::Nes(CPointerField* pcField)
{
	pcField->DoNotFreePointedTo(true);
	mpcClass->Nes();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerField* CBaseObject::Pointer(CPointer* pcPointer, const char* szFieldName)
{
	if (mpcClass == NULL)
	{
		FailHasClassField(__METHOD__);
		return NULL;
	}
	return mpcClass->Pointer(this, pcPointer, szFieldName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObjectField* CBaseObject::Embedded(CBaseObject* pcObject, const char* szFieldName)
{
	if (mpcClass == NULL)
	{
		FailHasClassField(__METHOD__);
		return NULL;
	}
	return mpcClass->Embedded(this, pcObject, szFieldName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPrimitiveField* CBaseObject::Primitive(SInt* pcPrimitive, const char* szFieldName)		{ if (mpcClass == NULL) { FailHasClassField(__METHOD__); return NULL; } else return mpcClass->Primitive(this, pcPrimitive, szFieldName); }
CPrimitiveField* CBaseObject::Primitive(SSize* pcPrimitive, const char* szFieldName)		{ if (mpcClass == NULL) { FailHasClassField(__METHOD__); return NULL; } else return mpcClass->Primitive(this, pcPrimitive, szFieldName); }
CPrimitiveField* CBaseObject::Primitive(Int8* pcPrimitive, const char* szFieldName)		{ if (mpcClass == NULL) { FailHasClassField(__METHOD__); return NULL; } else return mpcClass->Primitive(this, pcPrimitive, szFieldName); }
CPrimitiveField* CBaseObject::Primitive(UInt8* pcPrimitive, const char* szFieldName)		{ if (mpcClass == NULL) { FailHasClassField(__METHOD__); return NULL; } else return mpcClass->Primitive(this, pcPrimitive, szFieldName); }
CPrimitiveField* CBaseObject::Primitive(Int16* pcPrimitive, const char* szFieldName)		{ if (mpcClass == NULL) { FailHasClassField(__METHOD__); return NULL; } else return mpcClass->Primitive(this, pcPrimitive, szFieldName); }
CPrimitiveField* CBaseObject::Primitive(UInt16* pcPrimitive, const char* szFieldName)		{ if (mpcClass == NULL) { FailHasClassField(__METHOD__); return NULL; } else return mpcClass->Primitive(this, pcPrimitive, szFieldName); }
CPrimitiveField* CBaseObject::Primitive(Int32* pcPrimitive, const char* szFieldName)		{ if (mpcClass == NULL) { FailHasClassField(__METHOD__); return NULL; } else return mpcClass->Primitive(this, pcPrimitive, szFieldName); }
CPrimitiveField* CBaseObject::Primitive(UInt32* pcPrimitive, const char* szFieldName)		{ if (mpcClass == NULL) { FailHasClassField(__METHOD__); return NULL; } else return mpcClass->Primitive(this, pcPrimitive, szFieldName); }
CPrimitiveField* CBaseObject::Primitive(Int64* pcPrimitive, const char* szFieldName)		{ if (mpcClass == NULL) { FailHasClassField(__METHOD__); return NULL; } else return mpcClass->Primitive(this, pcPrimitive, szFieldName); }
CPrimitiveField* CBaseObject::Primitive(UInt64* pcPrimitive, const char* szFieldName)		{ if (mpcClass == NULL) { FailHasClassField(__METHOD__); return NULL; } else return mpcClass->Primitive(this, pcPrimitive, szFieldName); }
CPrimitiveField* CBaseObject::Primitive(Bool* pcPrimitive, const char* szFieldName)		{ if (mpcClass == NULL) { FailHasClassField(__METHOD__); return NULL; } else return mpcClass->Primitive(this, pcPrimitive, szFieldName); }
CPrimitiveField* CBaseObject::Primitive(Float32* pcPrimitive, const char* szFieldName)	{ if (mpcClass == NULL) { FailHasClassField(__METHOD__); return NULL; } else return mpcClass->Primitive(this, pcPrimitive, szFieldName); }
CPrimitiveField* CBaseObject::Primitive(Float64* pcPrimitive, const char* szFieldName)	{ if (mpcClass == NULL) { FailHasClassField(__METHOD__); return NULL; } else return mpcClass->Primitive(this, pcPrimitive, szFieldName); }
CPrimitiveField* CBaseObject::Primitive(Char8* pcPrimitive, const char* szFieldName)		{ if (mpcClass == NULL) { FailHasClassField(__METHOD__); return NULL; } else return mpcClass->Primitive(this, pcPrimitive, szFieldName); }
CPrimitiveField* CBaseObject::Primitive(Char16* pcPrimitive, const char* szFieldName)		{ if (mpcClass == NULL) { FailHasClassField(__METHOD__); return NULL; } else return mpcClass->Primitive(this, pcPrimitive, szFieldName); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::UnmanagedInt8(int8* pc, const char* szFieldName)						{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedInt8(this, pc, szFieldName); };
void CBaseObject::UnmanagedInt8(int8* pac, size uiLength, const char* szFieldName)		{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedInt8(this, pac, uiLength, szFieldName); };
void CBaseObject::UnmanagedInt16(int16* pi, const char* szFieldName)						{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedInt16(this, pi, szFieldName); };
void CBaseObject::UnmanagedInt16(int16* pai, size uiLength, const char* szFieldName)		{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedInt16(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedInt32(int32* pi, const char* szFieldName)						{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedInt32(this, pi, szFieldName); };
void CBaseObject::UnmanagedInt32(int32* pai, size uiLength, const char* szFieldName)		{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedInt32(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedInt64(int64* pi, const char* szFieldName)						{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedInt64(this, pi, szFieldName); };
void CBaseObject::UnmanagedInt64(int64* pai, size uiLength, const char* szFieldName)		{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedInt64(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedSInt(int* pi, const char* szFieldName)							{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedSInt(this, pi, szFieldName); };
void CBaseObject::UnmanagedSInt(int* pai, size uiLength, const char* szFieldName)			{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedSInt(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedUInt8(uint8* pc, const char* szFieldName)						{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedUInt8(this, pc, szFieldName); };
void CBaseObject::UnmanagedUInt8(uint8* pac, size uiLength, const char* szFieldName)		{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedUInt8(this, pac, uiLength, szFieldName); };
void CBaseObject::UnmanagedUInt16(uint16* pi, const char* szFieldName)					{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedUInt16(this, pi, szFieldName); };
void CBaseObject::UnmanagedUInt16(uint16* pai, size uiLength, const char* szFieldName)	{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedUInt16(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedUInt32(uint32* pi, const char* szFieldName)					{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedUInt32(this, pi, szFieldName); };
void CBaseObject::UnmanagedUInt32(uint32* pai, size uiLength, const char* szFieldName)	{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedUInt32(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedUInt64(uint64* pi, const char* szFieldName)					{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedUInt64(this, pi, szFieldName); };
void CBaseObject::UnmanagedUInt64(uint64* pai, size uiLength, const char* szFieldName)	{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedUInt64(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedSUInt(uint* pi, const char* szFieldName)						{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedSUInt(this, pi, szFieldName); };
void CBaseObject::UnmanagedSUInt(uint* pai, size uiLength, const char* szFieldName)		{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedSUInt(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedSize(size* pi, const char* szFieldName)						{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedSize(this, pi, szFieldName); };
void CBaseObject::UnmanagedSize(size* pai, size uiLength, const char* szFieldName)		{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedSize(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedFloat(float32* pf, const char* szFieldName)					{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedFloat(this, pf, szFieldName); }
void CBaseObject::UnmanagedFloat(float32* paf, size uiLength, const char* szFieldName)	{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedFloat(this, paf, uiLength, szFieldName); }
void CBaseObject::UnmanagedFloat(float64* pf, const char* szFieldName)					{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedFloat(this, pf, szFieldName); }
void CBaseObject::UnmanagedFloat(float64* paf, size uiLength, const char* szFieldName)	{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedFloat(this, paf, uiLength, szFieldName); }
void CBaseObject::UnmanagedFloat(float128* pf, const char* szFieldName)					{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedFloat(this, pf, szFieldName); }
void CBaseObject::UnmanagedFloat(float128* paf, size uiLength, const char* szFieldName)	{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedFloat(this, paf, uiLength, szFieldName); }
void CBaseObject::UnmanagedChar(char8* pc, const char* szFieldName)						{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedChar(this, pc, szFieldName); }
void CBaseObject::UnmanagedChar(char8* pac, size uiLength, const char* szFieldName)		{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedChar(this, pac, uiLength, szFieldName); }
void CBaseObject::UnmanagedChar(char16* pc, const char* szFieldName)						{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedChar(this, pc, szFieldName); }
void CBaseObject::UnmanagedChar(char16* pac, size uiLength, const char* szFieldName)		{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedChar(this, pac, uiLength, szFieldName); }
void CBaseObject::UnmanagedBool(bool* pb, const char* szFieldName)						{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedBool(this, pb, szFieldName); }
void CBaseObject::UnmanagedBool(bool* pab, size uiLength, const char* szFieldName)		{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedBool(this, pab, uiLength, szFieldName); }

void CBaseObject::UnmanagedEnum(void* pe, const char* szFieldName)						{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedEnum(this, pe, szFieldName); }

void CBaseObject::UnmanagedString(CChars* pcChars, const char* szFieldName)				{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedString(this, pcChars, szFieldName); };
void CBaseObject::UnmanagedString(CCharsImmutable* pcChars, const char* szFieldName)		{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedString(this, pcChars, szFieldName); };
void CBaseObject::UnmanagedNumber(CNumber* pcNumber, const char* szFieldName)				{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedNumber(this, pcNumber, szFieldName); };
void CBaseObject::UnmanagedDate(CDate* pcDate, const char* szFieldName)					{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedDate(this, pcDate, szFieldName); };

void CBaseObject::UnmanagedData(void* pv, size uiSizeof, const char* szFieldName)			{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedData(this, pv, uiSizeof, szFieldName); };
void CBaseObject::UnmanagedUnknown(void* pv, size uiSizeof, const char* szFieldName)		{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedUnknown(this, pv, uiSizeof, szFieldName); };
void CBaseObject::UnmanagedPointer(void** pv, const char* szFieldName)					{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->UnmanagedPointer(this, pv, szFieldName); };

void CBaseObject::Unmanaged2Float(SFloat32Vec2* pf, const char* szFieldName)						{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->Unmanaged2Float(this, pf, szFieldName); }
void CBaseObject::Unmanaged2Float(SFloat32Vec2* pf, size uiLength, const char* szFieldName)		{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->Unmanaged2Float(this, pf, uiLength, szFieldName); }
void CBaseObject::Unmanaged3Float(SFloat32Vec3* pf, const char* szFieldName)						{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->Unmanaged3Float(this, pf, szFieldName); }
void CBaseObject::Unmanaged3Float(SFloat32Vec3* pf, size uiLength, const char* szFieldName)		{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->Unmanaged3Float(this, pf, uiLength, szFieldName); }
void CBaseObject::Unmanaged4Float(SFloat32Vec4* pf, const char* szFieldName)						{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->Unmanaged4Float(this, pf, szFieldName); }
void CBaseObject::Unmanaged4Float(SFloat32Vec4* pf, size uiLength, const char* szFieldName)		{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->Unmanaged4Float(this, pf, uiLength, szFieldName); }
void CBaseObject::Unmanaged4x4Float(SFloat4x4* pf, const char* szFieldName)						{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->Unmanaged4x4Float(this, pf, szFieldName); }
void CBaseObject::Unmanaged4x4Float(SFloat4x4* pf, size uiLength, const char* szFieldName)		{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->Unmanaged4x4Float(this, pf, uiLength, szFieldName); }

void CBaseObject::Unmanaged2Int(SInt32Vec2* pf, const char* szFieldName)							{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->Unmanaged2Int(this, pf, szFieldName); }
void CBaseObject::Unmanaged2Int(SInt32Vec2* pf, size uiLength, const char* szFieldName)			{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->Unmanaged2Int(this, pf, uiLength, szFieldName); }
void CBaseObject::Unmanaged3Int(SInt32Vec3* pf, const char* szFieldName)							{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->Unmanaged3Int(this, pf, szFieldName); }
void CBaseObject::Unmanaged3Int(SInt32Vec3* pf, size uiLength, const char* szFieldName)			{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->Unmanaged3Int(this, pf, uiLength, szFieldName); }
void CBaseObject::Unmanaged4Int(SInt32Vec4* pf, const char* szFieldName)							{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->Unmanaged4Int(this, pf, szFieldName); }
void CBaseObject::Unmanaged4Int(SInt32Vec4* pf, size uiLength, const char* szFieldName)			{ if (mpcClass == NULL) FailHasClassField(__METHOD__); else mpcClass->Unmanaged4Int(this, pf, uiLength, szFieldName); }

