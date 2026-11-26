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
#include "BaseObject.h"
#include "Objects.h"
#include "ObjectsLock.h"
#include "ObjectsThreads.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::PrintMemoryUseIteration(CChars* psz)
{
	OBJECTS_READ();

	SIndexesIterator	sIter;
	CBaseObject*		pcBaseObject;

	pcBaseObject = mcMemory.StartIteration(&sIter);
	while (pcBaseObject)
	{
		pcBaseObject->PrintObject(psz);

		psz->Append("\n");
		pcBaseObject = mcMemory.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::PrintMemoryPages(CChars* psz)
{
	OBJECTS_READ();

	mcMemory.GetObjects()->Print(psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::PrintMemoryNames(CChars* psz)
{
	OBJECTS_READ();

	CIndexTreeMemory*			pcIndex;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;

	pcIndex = mcMemory.GetNames();

	bExists = pcIndex->StartIteration(&sIter, NULL, NULL, 0, NULL, NULL, 0);
	while (bExists)
	{
		psz->Append((char*)sIter.acKey.GetData(), sIter.iKeyLength);
		psz->AppendNewLine();

		bExists = pcIndex->Iterate(&sIter, NULL, NULL, 0, NULL, NULL, 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A Dump is its Print written to the console, and has no guard of its own.  This one has: it also dumps the two
// indexes themselves.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpMemoryUseIteration(void)
{
	OBJECTS_READ();

	CChars	sz;

	sz.Init("------------------------- Indices -------------------------- \n");
	PrintMemoryUseIteration(&sz);
	sz.Append("------------------------------------------------------------ \n");
	sz.DumpKill();

	mcMemory.GetNames()->Dump();
	sz.Init("------------------------------------------------------------ \n");
	sz.DumpKill();

	sz.Init();
	PrintMemoryPages(&sz);
	sz.Append("------------------------------------------------------------ \n");
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpMemoryPages(void)
{
	CChars	sz;

	sz.Init("--------------------------- Pages ---------------------------- \n");
	PrintMemoryPages(&sz);
	sz.Append("------------------------------------------------------------ \n");
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpMemoryNames(void)
{
	CChars	sz;

	sz.Init("------------------------- Memory Names ----------------------- \n");
	PrintMemoryNames(&sz);
	sz.Append("------------------------------------------------------------ \n");
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void PrintStackPointerThread(CChars* psz, SStackPointer* psPointer)
{
#ifdef OBJECTS_THREAD_CHECKS
	if (IsObjectsLocking())
	{
		psz->Append(" on thread ");
		psz->Append(psPointer->iThread);
	}
#else
	(void)psz;
	(void)psPointer;
#endif // OBJECTS_THREAD_CHECKS
}


//////////////////////////////////////////////////////////////////////////
//
// With the locking on, a Debug build says which thread's stack each pointer is on.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::PrintStackPointers(CChars* psz, bool bPointerMemory)
{
	OBJECTS_WRITE();

	CStackPointer*		pcPointer;
	SStackPointer*		psPointer;
	SFreeListIterator	sIter;
	size				ui;
	CBaseObject*		pcBaseObject;

	ui = 0;
	pcPointer = GetStackPointers()->StartIteration(&sIter);
	while (pcPointer)
	{
		psPointer = pcPointer->Get();
		if (psPointer->eType == SPT_Unknown)
		{
			psz->Append(ui);
			psz->Append(": Unknown");
			psz->AppendNewLine();
		}
		else if (psPointer->eType == SPT_Pointer)
		{
			if (bPointerMemory)
			{
				psz->AppendPointer(psPointer->u.pcPointer);
				psz->Append(" ");
			}
			psz->Append(ui);
			psz->Append(": Pointer [");
			pcBaseObject = psPointer->u.pcPointer->BaseObject();
			if (pcBaseObject)
			{
				pcBaseObject->PrintState(psz);
			}
			else if (psPointer->u.pcPointer->Object())
			{
				psPointer->u.pcPointer->Object()->PrintObject(psz);
			}
			else
			{
				psz->Append("NULL");
			}
			psz->Append("]");
			PrintStackPointerThread(psz, psPointer);
			psz->AppendNewLine();
		}
		else if (psPointer->eType == SPT_Collection)
		{
			if (bPointerMemory)
			{
				psz->AppendPointer(psPointer->u.pcCollection);
				psz->Append(" ");
			}
			psz->Append(ui);
			psz->Append(": Collection []");  //The collection holding the pointer isn't known.
			PrintStackPointerThread(psz, psPointer);
			psz->AppendNewLine();
		}
		else
		{
			psz->Append(ui);
			psz->Append(": ERROR");
			psz->AppendNewLine();
		}
		ui++;
		pcPointer = GetStackPointers()->Iterate(&sIter);
	}
	ThreadStackFromsPrint(psz, bPointerMemory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpStackPointers(void)
{
	CChars	sz;

	sz.Init();
	PrintStackPointers(&sz);
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::PrintGraph(CChars* psz, bool bPointerMemory)
{
	OBJECTS_WRITE();

	Ptr<CRoot>	pRoot;

	psz->Append("-------------------------- Graph --------------------------- \n");

	pRoot = Get(ROOT_NAME);
	if (pRoot.IsNotNull())
	{
		RecurseDumpGraph(psz, pRoot.Object(), 0, false, bPointerMemory);
	}

	psz->Append("------------------------------------------------------------ \n");

	UnsetDumpFlags();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::UnsetDumpFlags(void)
{
	SIndexesIterator	sIter;
	CBaseObject*		pcBaseObject;

	//DUMPED is a container flag: the objects embedded in these share it.
	pcBaseObject = mcMemory.StartIteration(&sIter);
	while (pcBaseObject)
	{
		SetFlagShort(&pcBaseObject->muiFlags, OBJECT_FLAGS_DUMPED, false);
		pcBaseObject = mcMemory.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpGraph(void)
{
	CChars		sz;

	sz.Init();
	PrintGraph(&sz);
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::RecurseDumpGraph(CChars* psz, CEmbeddedObject* pcIncoming, int iLevel, bool bEmbedded, bool bPointerMemory)
{
	CObject*					pcObject;
	CArrayEmbeddedObjectPtr		apcTos;
	size						i;
	CEmbeddedObject*			pcToObject;
	CBaseObject*				pcEmbeddedObject;
	CBaseObject*				pcBaseObject;
	size						uiNumTos;
	size						uiIndex;

	if (!pcIncoming->IsBaseObject())
	{
		return;
	}

	pcBaseObject = (CBaseObject*)pcIncoming;

	//DUMPED is a container flag, so an object embedded in the container being printed has it already.
	psz->Append(' ', iLevel * 3);
	if ((!bEmbedded && (pcBaseObject->GetFlags() & OBJECT_FLAGS_DUMPED)) || (pcBaseObject->GetDistToRoot() < iLevel))
	{
		psz->Append('*');
		pcBaseObject->PrintObject(psz, bEmbedded, bPointerMemory);
		psz->AppendNewLine();
		return;
	}
	else
	{
		psz->Append(' ');
		pcBaseObject->PrintObject(psz, bEmbedded, bPointerMemory);
		psz->AppendNewLine();
	}

	pcBaseObject->SetFlag(OBJECT_FLAGS_DUMPED, true);

	apcTos.Init();
	pcBaseObject->BaseGetPointerTos(&apcTos);
	uiNumTos = apcTos.NumElements();
	for (i = 0; i < uiNumTos; i++)
	{
		pcToObject = *apcTos.Get(i);
		RecurseDumpGraph(psz, pcToObject, iLevel + 1, false, bPointerMemory);
	}

	if (pcBaseObject->IsObject())
	{
		pcObject = (CObject*)pcBaseObject;
		uiIndex = 0;
		pcEmbeddedObject = pcObject->IterateDirectlyEmbedded(&uiIndex);
		while (pcEmbeddedObject)
		{
			RecurseDumpGraph(psz, pcEmbeddedObject, iLevel, true, bPointerMemory);
			pcEmbeddedObject = pcObject->IterateDirectlyEmbedded(&uiIndex);
		}
	}

	apcTos.Kill();
}

