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
#include "BaseLib/Log.h"
#include "ObjectSerialiser.h"
#include "ObjectDeserialiser.h"
#include "BaseObject.h"
#include "Objects.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject::CBaseObject()
{
	mpcObjectsThisIn = NULL;
	miDistToRoot = UNATTACHED_DIST_TO_ROOT;
	moi = INVALID_O_INDEX;
	miFlags = OBJECT_FLAGS_DIRTY;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PreInit(CObjects* pcObjects)
{
	mpcObjectsThisIn = pcObjects;
	Class();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PreInit(void)
{
	//Call this if you need to allocate objects on the stack.
	PreInit(NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Class(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Kill(void)
{
	CArrayEmbeddedBaseObjectPtr		apcFromsChanged;
	int								iNumKilled;
	int								iNumFroms;
	//This method is for the user to forcibly kill an object.
	//It is not called internally.  KillThisGraph is method used to free objects that are unattached.

	//This does NOT call CUnknown::Kill.  That is handled elsewhere.

	ValidateNotEmbedded(__METHOD__);

	RemoveAllStackFroms();
	
	iNumFroms = NumHeapFroms();
	if (iNumFroms == 0)
	{
		apcFromsChanged.Init();
		RemoveAllTos(&apcFromsChanged);
		mpcObjectsThisIn->UpdateDistToRootFromSubRoot(&apcFromsChanged);
		apcFromsChanged.Kill();
	}
	else
	{
		RemoveAllHeapFroms();
	}

	iNumKilled = KillThisGraph();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::KillDontFree(void)
{
	LOG_OBJECT_DESTRUCTION(this);

	KillData();
	KillInternalData();

	miFlags |= OBJECT_FLAGS_KILLED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Free(void)
{
	if (IsNotEmbedded())
	{
		if (mpcObjectsThisIn)
		{
			mpcObjectsThisIn->RemoveInKill(this);
		}
		moi = INVALID_O_INDEX;
		CUnknown::Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::KillInternalData(void)
{
	CEmbeddedObject::KillInternalData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::TryKill(BOOL bKillIfNoRoot)
{
	ValidateNotEmbedded(__METHOD__);

	BOOL			bHasStackPointers;
	BOOL			bHasHeapPointers;
	BOOL			bCanFindRoot;
	BOOL			bMustKill;

	if (bKillIfNoRoot)
	{
		bHasStackPointers = HasStackPointers();
		bCanFindRoot = CanFindRoot();

		//If we removed a heap pointer and have no stack pointers and cannot find the root
		bMustKill = !bCanFindRoot && !bHasStackPointers;
		if (bMustKill)
		{
			//then we can kill this object.
			KillThisGraph();
		}
		else if (bCanFindRoot)
		{
			UpdateDistToRootFromPointedFroms();
		}
		else if (bHasStackPointers)
		{
			ClearDistToRootForPathToNearestSubRoot();
			UnattachDistToRoot();
		}
	}
	else
	{
		bHasHeapPointers = HasHeapPointers();
		bHasStackPointers = HasStackPointers();
		bCanFindRoot = IsRoot();

		//If we removed a stack pointer and have no more stack pointers and have no heap pointers (regardless of whether or not they can find the root)
		bMustKill = !bHasHeapPointers && !bHasStackPointers && !bCanFindRoot;
		if (bMustKill)
		{
			//then we can kill this object.
			KillThisGraph();
		}

		//If we still have heap pointers but no stack pointers and we can't find the root then this object is still being initialised 
		//and should not be killed.
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::KillThisGraph(void)
{
	CArrayBaseObjectPtr		apcKilled;
	int						iNumKilled;

	//This method will never be called on an embedded object.
	//ie: Only the containing object can KillThisGraph.

	//This method will only be called on an object that cannot find the root.
	//If an object is being forcibly killed then it's tos and froms will have already been removed.

	if (mpcObjectsThisIn)
	{
		apcKilled.Init(1024);

		CollectThoseToBeKilled(&apcKilled);
		KillCollected(&apcKilled);

		iNumKilled = apcKilled.NumElements();
		apcKilled.Kill();
		return iNumKilled;
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
void CBaseObject::CollectPointedToToBeKilled(CArrayBaseObjectPtr* papcKilled, CBaseObject* pcPointedTo)
{
	BOOL			bHasStackPointers;
	BOOL			bCanFindRoot;
	BOOL			bMustKill;
	CBaseObject*	pcContainer;

	if (pcPointedTo)
	{
		if (!pcPointedTo->IsMarkedUnreachable())
		{
			pcContainer = pcPointedTo->GetEmbeddingContainer();

			bHasStackPointers = pcContainer->HasStackPointers();
			bCanFindRoot = pcContainer->CanFindRoot();

			bMustKill = !bCanFindRoot && !bHasStackPointers;
			if (bMustKill)
			{
				pcContainer->CollectThoseToBeKilled(papcKilled);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::MarkThisForKilling(CArrayBaseObjectPtr* papcKilled)
{
	CBaseObject*		pcTemp;

	//These both assume we are the embedding container.
	ClearDistToRoot();
	SetFlag(OBJECT_FLAGS_UNREACHABLE, TRUE);

	pcTemp = this;
	papcKilled->Add(&pcTemp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::CollectThoseToBeKilled(CArrayBaseObjectPtr* papcKilled)
{
	ValidateNotEmbedded(__METHOD__);

	MarkThisForKilling(papcKilled);
	CollectPointedToToBeKilled(papcKilled);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::KillCollected(CArrayBaseObjectPtr* papcKilled)
{
	mpcObjectsThisIn->Remove(papcKilled);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ClearDistToRoot(void)
{
	miDistToRoot = CLEARED_DIST_TO_ROOT;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CBaseObject::ClearDistToRootForPathToNearestSubRoot(void)
{
	//The object returned here is always the object immediately below the sub-root, not the sub-root itself.
	ValidateNotEmbedded(__METHOD__);

	int								i;
	int								iNumFroms;
	CBaseObject**					ppcPointedFrom;
	CBaseObject*					pcPointedFrom;
	CBaseObject*					pcRootSet;
	CBaseObject*					pcTemp;
	CArrayEmbeddedBaseObjectPtr		apcFroms;
	CBaseObject*					pcContainer;

	ClearDistToRoot();
	
	pcRootSet = NULL;

	apcFroms.Init();
	GetHeapFroms(&apcFroms);
	iNumFroms = apcFroms.NumElements();
	ppcPointedFrom = apcFroms.GetData();
	for (i = 0; i < iNumFroms; i++)
	{
		pcPointedFrom = ppcPointedFrom[i];
		if (pcPointedFrom->miDistToRoot >= ROOT_DIST_TO_ROOT)
		{
			if (pcPointedFrom->IsSubRoot())
			{
				//No point in stepping back to the sub root.  It's root distance is always correct.
				//Stop at the object immediately pointed from the sub root.
				apcFroms.Kill();
				return this;
			}

			pcContainer = pcPointedFrom->GetEmbeddingContainer();
			pcTemp = pcContainer->ClearDistToRootForPathToNearestSubRoot();
			if (pcTemp != NULL)
			{
				pcRootSet = pcTemp;
			}
		}
	}

	apcFroms.Kill();
	return pcRootSet;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetFlag(int iFlag, int iFlagValue)
{
	::SetFlag(&miFlags, iFlag, iFlagValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::CanFindRoot(void)
{
	ValidateNotEmbedded(__METHOD__);

	CEmbeddedObject*				pcPointedFrom;
	BOOL							bResult;

	if (this->IsRoot())
	{
		return TRUE;
	}

	SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, TRUE);

	pcPointedFrom = GetClosestFromToRoot();
	if (pcPointedFrom == NULL)
	{
		SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
		return FALSE;
	}

	bResult = pcPointedFrom->GetEmbeddingContainer()->CanFindRoot();

	SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetExpectedDistToRoot(int iExpectedDistToRoot)
{
	ValidateNotEmbedded(__METHOD__);

	int	iBestDistToRoot;

	iBestDistToRoot = CalculateDistToRootFromPointedFroms(iExpectedDistToRoot);
	SetDistToRootAndSetPointedTosExpectedDistToRoot(iBestDistToRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetCalculatedDistToRoot(void)
{
	ValidateNotEmbedded(__METHOD__);

	int	iBestDistToRoot;

	iBestDistToRoot = CalculateDistToRootFromPointedFroms();
	SetDistToRootAndSetPointedTosExpectedDistToRoot(iBestDistToRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::CalculateDistToRootFromPointedFroms(void)
{
	int iDistToRoot;

	iDistToRoot = CalculateDistToRootFromPointedFroms(MAX_DIST_TO_ROOT);
	if (iDistToRoot != MAX_DIST_TO_ROOT)
	{
		return iDistToRoot;
	}
	else
	{
		return UNATTACHED_DIST_TO_ROOT;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::CalculateDistToRootFromPointedFroms(int iDistToRoot)
{
	int				iNumFroms;
	int				i;
	CBaseObject*	pcFrom;
	int				iBestDistToRoot;

	iBestDistToRoot = iDistToRoot;
	iNumFroms = CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < iNumFroms; i++)
	{
		pcFrom = CEmbeddedObject::GetHeapFrom(i);
		if (pcFrom)
		{
			if (pcFrom->miDistToRoot < iBestDistToRoot)
			{
				if (pcFrom->GetEmbeddingContainer()->CanFindRoot())
				{
					if (pcFrom->miDistToRoot >= ROOT_DIST_TO_ROOT)
					{
						iBestDistToRoot = pcFrom->miDistToRoot+1;
					}
				}
			}
		}
	}
	return iBestDistToRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::UpdateDistToRootFromPointedFroms(void)
{
	ValidateNotEmbedded(__METHOD__);

	int	iNearestRoot;

	iNearestRoot = CalculateDistToRootFromPointedFroms();
	if (iNearestRoot != UNATTACHED_DIST_TO_ROOT)
	{
		SetDistToRootAndSetPointedTosExpectedDistToRoot(iNearestRoot);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::UnattachDistToRoot(void)
{
	ValidateNotEmbedded(__METHOD__);

	SetDistToRootAndSetPointedTosExpectedDistToRoot(UNATTACHED_DIST_TO_ROOT);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::RemoveToFrom(CEmbeddedObject* pcPointedTo, CArrayEmbeddedBaseObjectPtr* papcFromsChanged)
{
	CBaseObject*	pcBaseObject;

	if (pcPointedTo)
	{
		if (pcPointedTo->IsBaseObject())
		{
			pcBaseObject = (CBaseObject*)pcPointedTo;
			if (pcBaseObject->miDistToRoot >= ROOT_DIST_TO_ROOT)
			{
				pcBaseObject->PrivateRemoveHeapFrom(this);
				papcFromsChanged->Add(&pcBaseObject);
			}
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::SerialisedSize(void)
{
	CObjectSerialiser	cSerialiser;
	int					iLength;

	cSerialiser.Init(NULL, this);
	cSerialiser.Save();
	iLength = cSerialiser.GetLength();
	cSerialiser.Kill();
	return iLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsRoot(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsNamed(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CBaseObject::GetName(void)
{
	return "";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsSubRoot(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsInvalidated(void)
{
	return miFlags & OBJECT_FLAGS_INVALIDATED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsDirty(void)
{
	return miFlags & OBJECT_FLAGS_DIRTY;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::TestedForRoot(void)
{
	return miFlags & OBJECT_FLAGS_TESTED_FOR_ROOT;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::TestedForSanity(void)
{
	return miFlags & OBJECT_FLAGS_TESTED_FOR_SANITY;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetDirty(void)
{
	miFlags |= OBJECT_FLAGS_DIRTY;
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
void CBaseObject::SetObjectID(OIndex oi)
{
	moi = oi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ClearIndex(void)
{
	SetObjectID(INVALID_O_INDEX);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsUnknown(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsHollow(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CBaseObject::TestGetTo(int iToIndex)
{
	CEmbeddedObject**			ppTo;
	CEmbeddedObject*			pTo;
	CArrayEmbeddedObjectPtr		apcTos;

	apcTos.Init(32);
	GetTos(&apcTos);
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
//
//////////////////////////////////////////////////////////////////////////
OIndex CBaseObject::GetOI(void)
{
	return moi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetName(char* szName)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsBaseObject(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::GetNumEmbedded(void)
{
	SetFlagNumEmbedded(1);
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::GetNumEmbeddedFromFlags(void)
{
	int	iNumEmbedded;

	iNumEmbedded = (miFlags & OBJECT_FLAGS_NUM_EMBEDDED) >> 8;
	return iNumEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetFlagNumEmbedded(int iNumEmbedded)
{
	iNumEmbedded = iNumEmbedded << 8;
	miFlags &= ~OBJECT_FLAGS_NUM_EMBEDDED;
	miFlags |= iNumEmbedded & OBJECT_FLAGS_NUM_EMBEDDED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::TestGetNumEmbeddedFromFlags(void)
{
	return GetNumEmbeddedFromFlags();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObjects* CBaseObject::GetObjects(void)
{
	CBaseObject*	pcBaseObject;

	pcBaseObject = GetEmbeddingContainer();
	return pcBaseObject->mpcObjectsThisIn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointers* CBaseObject::GetStackPointers(void)
{
	CObjects*	pcObjects;

	pcObjects = GetObjects();
	if (pcObjects)
	{
		return pcObjects->GetStackPointers();
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
BOOL CBaseObject::IsMarkedUnreachable(void)
{
	return miFlags & OBJECT_FLAGS_UNREACHABLE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::TestCanFindRoot(void)
{
	return CanFindRoot();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::DumpFroms(void)
{
	CChars				sz;
	int					i;
	int					iNumEmbedded;
	CEmbeddedObject*	pcEmbedded;
	int					j;
	int					iNumHeapFroms;
	CBaseObject*		pcFromObject;
	int					iLength;
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

	iNumEmbedded = GetNumEmbedded();
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
void CBaseObject::DumpTos(void)
{
	CChars						sz;
	int							i;
	int							iNumEmbedded;
	CEmbeddedObject*			pcEmbedded;
	int							j;
	int							iNumTos;
	CEmbeddedObject**			ppcToObject;
	int							iLength;
	CChars						szLine;
	CArrayEmbeddedObjectPtr		acTos;
	CBaseObject*				pcToObject;
	int							iTotalTos;

	sz.Init();

	sz.Append("-- ");
	PrintObject(&sz);
	sz.Append(" --\n");
	iLength = sz.Length()-1;

	szLine.Init('-', iLength);
	szLine.AppendNewLine();

	iTotalTos = 0;
	iNumEmbedded = GetNumEmbedded();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = GetEmbeddedObject(i);
		iTotalTos += pcEmbedded->NumTos();
	}

	sz.Insert(0, &szLine);
	sz.Append("Total Tos [");
	sz.Append(iTotalTos);
	sz.Append("]\n");

	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = GetEmbeddedObject(i);
		acTos.Init();
		pcEmbedded->UnsafeGetEmbeddedObjectTos(&acTos);
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
void CBaseObject::Dump(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateFlag(int iFlag, char* szFlag)
{
	CChars	sz;

	if (miFlags & iFlag)
	{
		sz.Init();
		PrintObject(&sz, IsEmbedded());
		gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should not have flag [", szFlag,"] set.", NULL);
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateFlags(void)
{
	CChars			sz;
	CBaseObject*	pcContainer;

	ValidateFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, "OBJECT_FLAGS_TESTED_FOR_ROOT");
	ValidateFlag(OBJECT_FLAGS_KILLED, "OBJECT_FLAGS_KILLED");
	ValidateFlag(OBJECT_FLAGS_DUMPED, "OBJECT_FLAGS_DUMPED");
	ValidateFlag(OBJECT_FLAGS_UNREACHABLE, "OBJECT_FLAGS_UNREACHABLE");

	if (!((miDistToRoot >= ROOT_DIST_TO_ROOT) || (miDistToRoot == UNATTACHED_DIST_TO_ROOT)))
	{
		sz.Init();
		PrintObject(&sz, IsEmbedded());
		gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should not have a dist to root of [", IntToString(miDistToRoot), "].", NULL);
		sz.Kill();
	}

	if (IsEmbedded())
	{
		pcContainer = GetEmbeddingContainer();
		if (pcContainer->GetDistToRoot() != GetDistToRoot())
		{
			sz.Init();
			PrintObject(&sz, IsEmbedded());
			gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should have a dist to root [", IntToString(miDistToRoot), "] the same as it's embedding object [", IntToString(pcContainer->GetDistToRoot()),"].", NULL);
			sz.Kill();
		}

		if (moi != INVALID_O_INDEX)
		{
			sz.Init();
			PrintObject(&sz, IsEmbedded());
			gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should have an Index [", IndexToString(moi), "] of INVALID_O_INDEX [", IndexToString(INVALID_O_INDEX),"].", NULL);
			sz.Kill();
		}
	}
	else
	{
		if (moi == INVALID_O_INDEX)
		{
			sz.Init();
			PrintObject(&sz, IsEmbedded());
			gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should not have an Index of INVALID_O_INDEX [", IndexToString(INVALID_O_INDEX),"].", NULL);
			sz.Kill();
		}
	}
}

