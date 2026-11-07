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
#include "BaseLib/IndexTreeMemoryAccess.h"
#include "NamedIndexedObjects.h"
#include "Object.h"
#include "HollowObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedObjects::Init(void)
{
	mcMemoryNames.Init();
	mcMemoryIndexedObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedObjects::Kill(void)
{
	mcMemoryIndexedObjects.Kill();
	mcMemoryNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedObjects::ReInit(void)
{
	Kill();
	Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CNamedIndexedObjects::Get(OIndex oi)
{
	return (CBaseObject*)mcMemoryIndexedObjects.Get(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CNamedIndexedObjects::Get(const char* szName)
{
	OIndex					oi;
	CIndexTreeMemoryAccess	cAccess;
	bool					bExists;

	cAccess.Init(&mcMemoryNames);
	bExists = cAccess.GetStringData(szName, &oi);
	cAccess.Kill();
	if (bExists)
	{
		return Get(oi);
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
CBaseObject* CNamedIndexedObjects::Get(const char* szObjectName, OIndex oi)
{
	CBaseObject* pvObject;

	pvObject = Get(szObjectName);
	if (pvObject)
	{
		if (pvObject->GetIndex() != oi)
		{
			gcLogger.Error2(__METHOD__, " Cannot get object named [", szObjectName, "].  Mismatch expected index [", IndexToString(oi), "] with object index [", IndexToString(pvObject->GetIndex()), "].", NULL);
			return NULL;
		}
		return pvObject;
	}

	pvObject = Get(oi);
	if (pvObject)
	{
		if (StringCompare(pvObject->GetName(), szObjectName) != 0)
		{
			gcLogger.Error2(__METHOD__, " Cannot get object with index [", IndexToString(oi), "].  Mismatch expected name [", szObjectName, "] with object name [", pvObject->GetName(), "].", NULL);
			return NULL;
		}
		return pvObject;
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
bool CNamedIndexedObjects::Has(OIndex oi)
{
	return mcMemoryIndexedObjects.Has(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedObjects::Has(const char* szName)
{
	CIndexTreeMemoryAccess	cAccess;
	bool					bHasObject;

	cAccess.Init(&mcMemoryNames);
	bHasObject = cAccess.HasString(szName);
	cAccess.Kill();
	return bHasObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedObjects::RemoveIndex(OIndex oi)
{
	bool	bResult;

	if (oi != INVALID_O_INDEX)
	{
		//This only removes the index from the indexes, it does not free the object pointer too.
		bResult = mcMemoryIndexedObjects.Remove(oi);
		return bResult;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedObjects::RemoveName(const char* szName)
{
	CIndexTreeMemoryAccess	cAccess;
	bool					bResult;

	//This only removes the name from the names, it does not free the object pointer to.
	cAccess.Init(&mcMemoryNames);
	bResult = cAccess.DeleteString(szName);
	cAccess.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// By index, and by name too if it has one.  Nothing is added if the index
// or the name is taken.
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedObjects::CanAdd(CBaseObject* pcObject)
{
	OIndex	oi;
	char*	szName;

	oi = pcObject->GetIndex();
	szName = pcObject->GetName();
	if (!IsValidIndex(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot add object named [", szName, "] with an invalid index [", IndexToString(oi), "].", NULL);
	}
	if (!StrEmpty(szName) && Has(szName))
	{
		return gcLogger.Error2(__METHOD__, " Cannot add object named [", szName, "].  It already exists.", NULL);
	}
	if (Has(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot add object with index [", IndexToString(oi), "].  It already exists.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedObjects::Add(CBaseObject* pcObject)
{
	OIndex					oi;
	char*					szName;
	bool					bResult;
	CIndexTreeMemoryAccess	cAccess;

	if (!CanAdd(pcObject))
	{
		return false;
	}

	oi = pcObject->GetIndex();
	if (!mcMemoryIndexedObjects.Add(oi, pcObject))
	{
		return false;
	}

	szName = pcObject->GetName();
	if (StrEmpty(szName))
	{
		return true;
	}

	cAccess.Init(&mcMemoryNames);
	bResult = cAccess.PutStringLong(szName, oi) != NULL;
	cAccess.Kill();
	if (!bResult)
	{
		mcMemoryIndexedObjects.Remove(oi);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CNamedIndexedObjects::NumIndexed(void)
{
	return mcMemoryIndexedObjects.NumIndexed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CNamedIndexedObjects::NumNames(void)
{
	return mcMemoryNames.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedObjects* CNamedIndexedObjects::GetObjects(void)
{
	return &mcMemoryIndexedObjects;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeMemory* CNamedIndexedObjects::GetNames(void)
{
	return &mcMemoryNames;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CNamedIndexedObjects::StartIteration(SIndexesIterator* psIter)
{
	return mcMemoryIndexedObjects.StartIterationObject(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CNamedIndexedObjects::Iterate(SIndexesIterator* psIter)
{
	return mcMemoryIndexedObjects.IterateObject(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedObjects::ValidateNoDirty(void)
{
	SIndexesIterator	sIter;
	OIndex				oi;
	CBaseObject*		pcObject;
	CChars				szIdentifier;

	oi = mcMemoryIndexedObjects.StartIteration(&sIter);
	while (oi != INVALID_O_INDEX)
	{
		pcObject = Get(oi);

		//The objects Flush would save: they must be flushed (or discarded) before Objects is killed.
		if (pcObject->NeedsSaving())
		{
			szIdentifier.Init();
			pcObject->PrintIdentifier(&szIdentifier);
			gcLogger.Error2(__METHOD__, " Object [", szIdentifier.Text(), "] is dirty.", NULL);
			szIdentifier.Kill();
			sIter.Kill();
			return false;
		}
		oi = mcMemoryIndexedObjects.Iterate(&sIter);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedObjects::FreeObjects(CArrayBaseObjectPtr* papcObjects)
{
	//The Objects must already have been freed (FreeInternal and DoneKill).  They are left in memory for Kill.

	size			i;
	size			uiNumElements;
	CBaseObject*	pcObject;

	uiNumElements = papcObjects->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcObject = papcObjects->GetPtr(i);
		pcObject->FreeIdentifiers();
		pcObject->FreePointers();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedObjects::FreeObject(CBaseObject* pcObject)
{
	//This order is important.
	pcObject->RemoveAllStackFroms();
	pcObject->FreePointers();
	RemoveIdentifiers(pcObject);
	pcObject->FreeIdentifiers();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedObjects::RemoveIdentifiers(CBaseObject* pcObject)
{
	char*	szName;
	bool	bResult;

	bResult = RemoveIndex(pcObject->GetIndex());

	szName = pcObject->GetName();
	if (!StrEmpty(szName))
	{
		bResult &= RemoveName(szName);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedObjects::Dump(void)
{
	CChars	sz;

	sz.Init("\n=============================== Memory Names ===============================\n");
	sz.DumpKill();

	mcMemoryNames.Dump();

	sz.Init("\n============================== Memory Indexes ==============================\n");
	sz.DumpKill();

	mcMemoryIndexedObjects.GetIndexes()->Dump();
}

