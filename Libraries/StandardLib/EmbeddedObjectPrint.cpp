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
#include "ObjectsLock.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CEmbeddedObject::PrintObject(CChars* psz)
{
	return PrintObject(psz, IsEmbedded());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CEmbeddedObject::PrintObject(CChars* psz, bool bEmbedded, bool bPointerMemory)
{
	int		iDistToRoot;
	size	uiNumWeakFroms;

	if (bPointerMemory)
	{
		psz->Append(PointerToString(this));
		psz->Append(" ");
	}
	psz->Append("[");
	iDistToRoot = GetDistToRoot();
	if (iDistToRoot >= 0 && iDistToRoot <= 9)
	{
		psz->Append(" ");
	}
	psz->Append(iDistToRoot);
	if (!IsAllocatedInObjects())
	{
		psz->Append(",0");
	}
	psz->Append("]:");

	if (bEmbedded)
	{
		psz->Append("(");
	}
	psz->Append(ClassName());
	psz->Append("(");
	psz->Append(ClassSize());
	psz->Append(") Index:");
	psz->Append(GetIndex());
	if (IsNamed())
	{
		psz->Append(" Name:");
		psz->Append(GetName());
	}
	psz->Append(" Froms:");
	psz->Append(CEmbeddedObject::NumHeapFroms());
	psz->Append(",");
	psz->Append(CEmbeddedObject::NumStackFroms());
	uiNumWeakFroms = GetObjectsOrDefault()->GetWeakFroms()->NumWeakFroms(this);
	if (uiNumWeakFroms > 0)
	{
		psz->Append(" Weak:");
		psz->Append(uiNumWeakFroms);
	}
	if (bEmbedded)
	{
		psz->Append(")");
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
// The state of each object in the heap that points at this.
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::PrintHeapFroms(CChars* psz)
{
	OBJECTS_READ();

	size			uiNumElements;
	size			i;
	CBaseObject*	pcFromObject;

	uiNumElements = CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < uiNumElements; i++)
	{
		pcFromObject = CEmbeddedObject::GetHeapFrom(i);
		psz->Append(i);
		psz->Append(": ");
		pcFromObject->PrintState(psz);
		psz->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::DumpHeapFroms(void)
{
	CChars	sz;

	sz.Init();
	PrintHeapFroms(&sz);
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each weak reference to this: its holder and the slot in it.
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::PrintWeakFroms(CChars* psz)
{
	CWeakFroms*	pcWeakFroms;
	SWeakFrom*	psFrom;
	size		i;

	pcWeakFroms = GetObjectsOrDefault()->GetWeakFroms();
	for (i = 0; i < pcWeakFroms->NumWeakFroms(this); i++)
	{
		psFrom = pcWeakFroms->GetWeakFrom(this, i);
		psz->Append(" Weak from ");
		psz->Append(PointerToString(psFrom->pcHolder));
		psz->Append(" slot ");
		psz->Append(psFrom->uiSlot);
		psz->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* ObjectToString(CEmbeddedObject* pcObject)
{
	return ObjectToString(pcObject, (pcObject != NULL) && pcObject->IsEmbedded());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* ObjectToString(CEmbeddedObject* pcObject, bool bEmbedded)
{
	CChars	szChars;
	size	uiCount;
	char*	sz;

	szChars.Init();
	if (pcObject)
	{
		pcObject->PrintObject(&szChars, bEmbedded);
	}
	else
	{
		szChars.Append("NULL");
	}

	uiCount = IncrementLogToStringCount();
	sz = gaszLogToStringScratchPad[uiCount];
	StrCpySafe(sz, szChars.Text(), LOG_TO_STRING_MAX_LENGTH);
	szChars.Kill();
	return sz;
}

