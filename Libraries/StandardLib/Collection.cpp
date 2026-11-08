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
#include "Objects.h"
#include "Collection.h"


//A collection holding at least this many members keeps where each is, once it has to find one.
#define COLLECTION_MEMBER_LOCATIONS_THRESHOLD	64


//////////////////////////////////////////////////////////////////////////
//
// A collection's own fields are set in PreFields: this marks the base of
// its sub-classes' Inits.
//
//////////////////////////////////////////////////////////////////////////
void CCollection::Init(void)
{
	PreInit();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCollection::Class(void)
{
	U_Bool(mbSubRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCollection::Kill(void)
{
	if (muiFlags != OBJECT_FLAGS_CALLED_CONSTRUCTOR)
	{
		CBaseObject::Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCollection::IsCollection(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCollection::IsObject(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CCollection::Dehollow(void)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCollection::EmbedFields(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCollection::PreFields(void)
{
	mbSubRoot = false;
	mpcMemberLocations = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCollection::MakeSubRoot(void)
{
	mbSubRoot = true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCollection::IsSubRoot(void)
{
	return mbSubRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCollection::RemoveAll(void)
{
	bool	bResult;

	bResult = ClearPointerTos(true);

#ifdef _DEBUG
	ValidateObjectsConsistency(true);
#endif // _DEBUG

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// pcObject (if not NULL) was put in this collection: it gets this as a
// from, a heap from when the collection is allocated in Objects, otherwise
// a stack from.
//
//////////////////////////////////////////////////////////////////////////
void CCollection::AddObjectFrom(CEmbeddedObject* pcObject, bool bValidate)
{
	MembersChanged();
	if (pcObject == NULL)
	{
		return;
	}

	if (IsEmbeddingContainerAllocatedInObjects())
	{
		pcObject->AddHeapFrom(this, bValidate);
	}
	else
	{
		pcObject->AddStackFrom(this, bValidate);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// pcNew (which may be NULL) has taken the place of pcOld (which may be NULL)
// in this collection.  The new from is added first: they may be the same
// object, which removing first would free.  The objects can only be
// validated once both are done, so that's after the last step there is.
//
//////////////////////////////////////////////////////////////////////////
bool CCollection::ReplaceObjectFrom(CEmbeddedObject* pcNew, CEmbeddedObject* pcOld)
{
	AddObjectFrom(pcNew, pcOld == NULL);
	return RemoveObjectTryFree(pcOld, true);
}


//////////////////////////////////////////////////////////////////////////
//
// A member (or an empty place) was added or taken out: what's saved has
// changed, and the change is done (LogicalDirty: a write-through Codabase
// writes it now, not when a stack pointer happens to be pointed at it
// next).  Every change goes through AddObjectFrom or RemoveObjectTryFree;
// loading doesn't.
//
//////////////////////////////////////////////////////////////////////////
void CCollection::MembersChanged(void)
{
	if (IsEmbeddingContainerAllocatedInObjects())
	{
		GetEmbeddingContainer()->SetDirty(true);
		LogicalDirty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Unlinks this collection's from on pcObject: a heap from when the
// collection is allocated in Objects, otherwise a stack from.
//
//////////////////////////////////////////////////////////////////////////
bool CCollection::UnlinkFrom(CEmbeddedObject* pcObject)
{
	if (IsEmbeddingContainerAllocatedInObjects())
	{
		return pcObject->UnlinkHeapFrom(this);
	}
	else
	{
		return pcObject->UnlinkStackFrom(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// For a collection freed with other objects: those freed with it drop
// their froms anyway, so only the others are unlinked.
//
//////////////////////////////////////////////////////////////////////////
bool CCollection::UnlinkFromUnlessFreed(CEmbeddedObject* pcObject)
{
	if ((pcObject == NULL) || pcObject->IsBeingFreed())
	{
		return true;
	}
	return UnlinkFrom(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
// pcObject (if not NULL) was taken out of this collection: unlinks its
// from and frees it if nothing else reaches it.
//
//////////////////////////////////////////////////////////////////////////
bool CCollection::RemoveObjectTryFree(CEmbeddedObject* pcObject, bool bValidate)
{
	bool		bHeapFrom;
	bool		bResult;
#ifdef _DEBUG
	CObjects*	pcObjectsThisIn;
#endif // _DEBUG

	MembersChanged();
	if (pcObject == NULL)
	{
		return true;
	}

	if (!UnlinkFrom(pcObject))
	{
		return gcLogger.Error2(__METHOD__, " Could not remove Collection {", ObjectToString(this), "} From from Object {", ObjectToString(pcObject), "}.", NULL);
	}

#ifdef _DEBUG
	pcObjectsThisIn = GetObjectsThisIn();
#endif // _DEBUG
	bHeapFrom = IsEmbeddingContainerAllocatedInObjects();
	bResult = GetObjectsOrDefault()->FromRemoved(pcObject, bHeapFrom) != FR_Error;

#ifdef _DEBUG
	if (bValidate && bHeapFrom && pcObjectsThisIn)
	{
		pcObjectsThisIn->ValidateObjectsConsistency();
	}
#endif // _DEBUG
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// An Object in this collection more than once has a from for every copy.
// Removes the froms for uiRemoved copies.  Only the last can free the Object,
// freeing it sooner would leave the remaining froms to be removed from a
// freed Object.
//
//////////////////////////////////////////////////////////////////////////
bool CCollection::RemoveObjectCopiesTryFree(CEmbeddedObject* pcObject, size uiRemoved, bool bValidate)
{
	size	i;
	bool	bResult;

	//Nothing to remove succeeds, as for RemoveObjectTryFree.
	if (uiRemoved == 0)
	{
		return true;
	}
	if (pcObject == NULL)
	{
		return RemoveObjectTryFree(NULL, bValidate);
	}

	bResult = true;
	for (i = 1; i < uiRemoved; i++)
	{
		bResult &= UnlinkFrom(pcObject);
	}
	if (!bResult)
	{
		return false;
	}
	return RemoveObjectTryFree(pcObject, bValidate);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCollection::ValidateConsistency(void)
{
	ValidateEmbeddedConsistency();
	ValidateCanFindRoot();
	ValidateMemberLocations();
}


//////////////////////////////////////////////////////////////////////////
//
// Where each member is, so a member can be found without searching them
// all.  Made the first time it is asked for once there are many members;
// NULL while there are few (searching them is quicker), or if this kind of
// collection doesn't keep them.  Once made, every change to the members
// keeps it up to date, unless they have moved (see HaveMembersMoved) when
// it is made again.
//
//////////////////////////////////////////////////////////////////////////
CPointerLocations* CCollection::GetMemberLocations(void)
{
	if (HaveMembersMoved())
	{
		KillMemberLocations();
	}
	if (mpcMemberLocations != NULL)
	{
		return mpcMemberLocations;
	}
	if (!CanLocateMembers() || (NumElements() < COLLECTION_MEMBER_LOCATIONS_THRESHOLD))
	{
		return NULL;
	}

	mpcMemberLocations = NewMalloc<CPointerLocations>();
	mpcMemberLocations->Init();
	AddMemberLocations(mpcMemberLocations);
	return mpcMemberLocations;
}


//////////////////////////////////////////////////////////////////////////
//
// Forgets where the members are (when they all go, or are loaded anew).
//
//////////////////////////////////////////////////////////////////////////
void CCollection::KillMemberLocations(void)
{
	SPointerLocationsIterator	sIter;
	void*						pvMember;
	ptr							uiLocation;
	bool						bExists;

	if (mpcMemberLocations == NULL)
	{
		return;
	}

	bExists = mpcMemberLocations->StartAllIteration(&sIter, &pvMember, &uiLocation);
	while (bExists)
	{
		FreeMemberLocation(uiLocation);
		bExists = mpcMemberLocations->IterateAll(&sIter, &pvMember, &uiLocation);
	}
	mpcMemberLocations->Kill();
	SafeFree(mpcMemberLocations);
}


//////////////////////////////////////////////////////////////////////////
//
// Takes pcMember out of every location kept for it (RemoveMemberAt) and
// forgets them.  The member locations must be kept.  Returns how many
// there were.
//
//////////////////////////////////////////////////////////////////////////
size CCollection::RemoveLocatedMember(CEmbeddedObject* pcMember)
{
	SPointerLocationsIterator	sIter;
	ptr							uiLocation;
	size						uiRemoved;

	uiRemoved = 0;
	while (mpcMemberLocations->StartIteration(&sIter, pcMember, &uiLocation))
	{
		mpcMemberLocations->Remove(pcMember, uiLocation);
		RemoveMemberAt(uiLocation);
		FreeMemberLocation(uiLocation);
		uiRemoved++;
	}
	return uiRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
// Puts pcNew in every location kept for pcOld (a hollow object replaced by
// the object loaded for it): nothing moves, so the locations become
// pcNew's.  The member locations must be kept.  Returns how many there
// were.
//
//////////////////////////////////////////////////////////////////////////
size CCollection::RemapLocatedMember(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	SPointerLocationsIterator	sIter;
	ptr							uiLocation;
	bool						bExists;

	bExists = mpcMemberLocations->StartIteration(&sIter, pcOld, &uiLocation);
	while (bExists)
	{
		ReplaceMemberAt(uiLocation, pcNew);
		bExists = mpcMemberLocations->Iterate(&sIter, &uiLocation);
	}
	return mpcMemberLocations->MovePointer(pcOld, pcNew);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether this kind of collection keeps where its members are.
//
//////////////////////////////////////////////////////////////////////////
bool CCollection::CanLocateMembers(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds where every member is.
//
//////////////////////////////////////////////////////////////////////////
void CCollection::AddMemberLocations(CPointerLocations* pcLocations)
{
}


//////////////////////////////////////////////////////////////////////////
//
// A location may own memory (an index's copy of a key).
//
//////////////////////////////////////////////////////////////////////////
void CCollection::FreeMemberLocation(ptr uiLocation)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCollection::IsMemberAt(CEmbeddedObject* pcMember, ptr uiLocation)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the member at uiLocation out (see RemoveLocatedMember).
//
//////////////////////////////////////////////////////////////////////////
void CCollection::RemoveMemberAt(ptr uiLocation)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Puts pcMember at uiLocation in place of what is there (see
// RemapLocatedMember).
//
//////////////////////////////////////////////////////////////////////////
void CCollection::ReplaceMemberAt(ptr uiLocation, CEmbeddedObject* pcMember)
{
}


//////////////////////////////////////////////////////////////////////////
//
// How many locations the members have: one for each non NULL member.
//
//////////////////////////////////////////////////////////////////////////
size CCollection::NumLocatedMembers(void)
{
	return NonNullElements();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the members have moved to other locations since they were last
// all added (an array's elements shift when one is inserted).  The
// locations kept are stale then, and are thrown away when next wanted
// rather than moved with every change.
//
//////////////////////////////////////////////////////////////////////////
bool CCollection::HaveMembersMoved(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Every location kept holds its member, and every member is kept (unless
// the members have moved, when the locations are made again).
//
//////////////////////////////////////////////////////////////////////////
void CCollection::ValidateMemberLocations(void)
{
	SPointerLocationsIterator	sIter;
	void*						pvMember;
	ptr							uiLocation;
	bool						bExists;

	if ((mpcMemberLocations == NULL) || HaveMembersMoved())
	{
		return;
	}

	if (mpcMemberLocations->NumLocations() != NumLocatedMembers())
	{
		gcLogger.Error2(__METHOD__, " Collection {", ObjectToString(this), "} keeps [", SizeToString(mpcMemberLocations->NumLocations()), "] member locations but has [", SizeToString(NumLocatedMembers()), "] members.", NULL);
	}

	bExists = mpcMemberLocations->StartAllIteration(&sIter, &pvMember, &uiLocation);
	while (bExists)
	{
		if (!IsMemberAt((CEmbeddedObject*)pvMember, uiLocation))
		{
			gcLogger.Error2(__METHOD__, " Collection {", ObjectToString(this), "} keeps a location for member {", ObjectToString((CEmbeddedObject*)pvMember), "} that it isn't at.", NULL);
		}
		bExists = mpcMemberLocations->IterateAll(&sIter, &pvMember, &uiLocation);
	}
}


