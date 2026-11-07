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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::PrintMemoryUseIteration(CChars* psz)
{
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
void CObjects::PrintMemoryUseRecursion(CChars* psz)
{
	mcMemory.GetObjects()->Print(psz, true, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::PrintMemoryNames(CChars* psz)
{
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
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpMemoryUseIteration(void)
{
	CChars	sz;

	sz.Init("------------------------- Indices -------------------------- \n");
	PrintMemoryUseIteration(&sz);
	sz.Append("------------------------------------------------------------ \n");
	sz.DumpKill();

	mcMemory.GetNames()->Dump();
	sz.Init("------------------------------------------------------------ \n");
	sz.DumpKill();

	mcMemory.GetObjects()->GetIndexes()->Dump();
	sz.Init("------------------------------------------------------------ \n");
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpMemoryUseRecursion(void)
{
	CChars	sz;

	sz.Init("-------------------------- Indices --------------------------- \n");
	PrintMemoryUseRecursion(&sz);
	sz.Append("------------------------------------------------------------ \n");
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpNames(void)
{
	CChars	sz;

	sz.Init("---------------------------- Names --------------------------- \n");
	sz.DumpKill();

	//mpcDataConnection->StartNameIteration()
	//	...

	sz.Init();
	sz.Append("------------------------------------------------------------ \n");
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpMemoryNames(void)
{
	CChars						sz;
	CIndexTreeMemory*			pcIndex;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;

	sz.Init("------------------------- Memory Names ----------------------- \n");
	sz.DumpKill();

	pcIndex = mcMemory.GetNames();

	bExists = pcIndex->StartIteration(&sIter, NULL, NULL, 0, NULL, NULL, 0);
	while (bExists)
	{
		sz.Init((char*)sIter.acKey.GetData(), 0, sIter.iKeyLength);
		sz.AppendNewLine();
		sz.DumpKill();

		bExists = pcIndex->Iterate(&sIter, NULL, NULL, 0, NULL, NULL, 0);
	}

	sz.Init();
	sz.Append("------------------------------------------------------------ \n");
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::PrintStackPointers(CChars* psz, bool bPointerMemory)
{
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
	Ptr<CRoot>	pRoot;

	psz->Init("-------------------------- Graph --------------------------- \n");

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
	CBaseObject*		pcEmbeddedObject;
	CObject*			pcObject;
	size				i;
	size				uiNumEmbedded;

	pcBaseObject = mcMemory.StartIteration(&sIter);
	while (pcBaseObject)
	{
		SetFlagShort(&pcBaseObject->muiFlags, OBJECT_FLAGS_DUMPED, false);

		if (pcBaseObject->IsObject())
		{
			pcObject = (CObject*)pcBaseObject;
			uiNumEmbedded = pcObject->mapEmbedded.NumElements();
			for (i = 0; i < uiNumEmbedded; i++)
			{
				pcEmbeddedObject = *pcObject->mapEmbedded.Get(i);
				SetFlagShort(&pcEmbeddedObject->muiFlags, OBJECT_FLAGS_DUMPED, false);
			}
		}
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

	psz->Append(' ', iLevel * 3);
	if ((pcBaseObject->GetFlags() & OBJECT_FLAGS_DUMPED) || (pcBaseObject->GetDistToRoot() < iLevel))
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
		RecurseDumpGraph(psz, pcToObject, iLevel+1, false, bPointerMemory);
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

