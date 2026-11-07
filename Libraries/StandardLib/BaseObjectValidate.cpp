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
#include "BaseObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateFlagNotSet(uint16 iFlag, const char* szFlag)
{
	if (GetFlags() & iFlag)
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
	if (!(GetFlags() & iFlag))
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
	int					iDistToRoot;

	iDistToRoot = GetDistToRoot();
	if (iDistToRoot > ROOT_DIST_TO_ROOT)
	{
		pcClosest = GetClosestFromToRoot();
		if (pcClosest == NULL)
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has a positive dist to root and should be able to find the Root object.", NULL);
			return;
		}
		iClosestDistToRoot = pcClosest->GetDistToRoot();
		if (iClosestDistToRoot != iDistToRoot - 1)
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has dist to root [", IntToString(iDistToRoot), "] but its closest from has [", IntToString(iClosestDistToRoot), "].", NULL);
		}
	}
	else if (iDistToRoot == ROOT_DIST_TO_ROOT)
	{
		if (!IsRoot())
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has a dist to root [0] but is not the Root object.", NULL);
		}
	}
	else if (iDistToRoot == UNATTACHED_DIST_TO_ROOT)
	{
		pcClosest = GetClosestFromToRoot();
		if (pcClosest != NULL)
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has an [UNATTACHED_DIST_TO_ROOT] dist to root should not be able to find the Root object.", NULL);
		}
	}
	else if (iDistToRoot == CLEARED_DIST_TO_ROOT)
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} should not have dist to root [CLEARED_DIST_TO_ROOT].", NULL);
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} should not have dist to root [", IntToString(iDistToRoot), "].", NULL);
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

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateAllocation(void)
{
	bool	bAllocateCalled;
	bool	bInObjects;

	bAllocateCalled = FixBool(GetFlags() & OBJECT_FLAGS_CALLED_ALLOCATE);
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
	int		iDistToRoot;

	iDistToRoot = GetDistToRoot();
	if (!((iDistToRoot >= ROOT_DIST_TO_ROOT) || (iDistToRoot == UNATTACHED_DIST_TO_ROOT)))
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} should not have a dist to root of [", IntToString(iDistToRoot), "].", NULL);
	}
	ValidateKnownHeldFromStack();
}


//////////////////////////////////////////////////////////////////////////
//
// An object known held from the stack has a stack from, or a from that is
// known held and older: following them always ends on the stack.
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateKnownHeldFromStack(void)
{
	SHeapFromIterator	sIter;
	CBaseObject*		pcFrom;

	if (!IsKnownHeldFromStack() || IsEmbedded() || HasStackFroms())
	{
		return;
	}

	pcFrom = StartHeapFromIteration(&sIter);
	while (pcFrom)
	{
		if (pcFrom->IsKnownHeldFromStack() && pcFrom->IsOlderThan(this))
		{
			return;
		}
		pcFrom = IterateHeapFrom(&sIter);
	}
	gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} is known held from the stack but has no stack from and no older from known held.", NULL);
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
	if (!(GetFlags() & OBJECT_FLAGS_CALLED_KILL))
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has not been killed.  Ensure sub-classes are declared DESTRUCTABLE().", NULL);
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

		pcStackPointer = GetStackPointers()->StartIteration(&sIter);
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
			if ((pvPointer >= pvStart) && (pvPointer < pvEnd))
			{
				gcLogger.Error2(__METHOD__, " Stack Pointer [", PointerToString(pvPointer), "] appears to belong to Object {", ObjectToString(this), "}.  Ensure it is set up in Class().", NULL);
			}
			pcStackPointer = GetStackPointers()->Iterate(&sIter);
		}
	}
}

