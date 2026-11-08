/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Numbers.h"
#include "DefineMap.h"
#include "FNVHash.h"


#define DEFINE_MAP_FIRST_CAPACITY	256
#define DEFINE_MAP_CHUNK			(16 KB)


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::Init(void)
{
	mapcTable = NULL;
	muiCapacity = 0;
	muiDefines = 0;
	mlliNextID = 0;
	mcDefines.Init(DEFINE_MAP_CHUNK);
}


//////////////////////////////////////////////////////////////////////////
//
// Kills every define.
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::Kill(void)
{
	size	i;

	for (i = 0; i < muiCapacity; i++)
	{
		if (mapcTable[i])
		{
			mapcTable[i]->Kill();
		}
	}
	if (mapcTable)
	{
		free(mapcTable);
	}
	mapcTable = NULL;
	muiCapacity = 0;
	muiDefines = 0;
	mcDefines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// FNV-1a.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDefineMap::Hash(const char* szName, size uiLength)
{
	return FNVHash32(szName, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
// The slot holding the define named szName, or the empty slot where it
// would go.  The table mustn't be full.
//
//////////////////////////////////////////////////////////////////////////
size CDefineMap::Find(const char* szName, size uiLength, uint32 uiHash)
{
	size		uiMask;
	size		i;
	CDefine*	pcDefine;

	uiMask = muiCapacity - 1;
	for (i = uiHash & uiMask;; i = (i + 1) & uiMask)
	{
		pcDefine = mapcTable[i];
		if (!pcDefine)
		{
			return i;
		}
		if ((pcDefine->GetNameHash() == uiHash) && (pcDefine->GetNameLength() == uiLength) && (memcmp(pcDefine->GetName(), szName, uiLength) == 0))
		{
			return i;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::GetExact(const char* szName, size uiLength)
{
	if (muiDefines == 0)
	{
		return NULL;
	}
	return mapcTable[Find(szName, uiLength, Hash(szName, uiLength))];
}


//////////////////////////////////////////////////////////////////////////
//
// The table twice as long (or its first), the defines put in again.
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::Grow(void)
{
	CDefine**	apcOld;
	size		uiOldCapacity;
	size		i;
	size		uiMask;
	size		j;

	apcOld = mapcTable;
	uiOldCapacity = muiCapacity;
	muiCapacity = (muiCapacity == 0) ? DEFINE_MAP_FIRST_CAPACITY : muiCapacity * 2;
	mapcTable = (CDefine**)malloc(muiCapacity * sizeof(CDefine*));
	memset(mapcTable, 0, muiCapacity * sizeof(CDefine*));
	uiMask = muiCapacity - 1;
	for (i = 0; i < uiOldCapacity; i++)
	{
		if (apcOld[i])
		{
			for (j = apcOld[i]->GetNameHash() & uiMask; mapcTable[j] != NULL; j = (j + 1) & uiMask)
			{
			}
			mapcTable[j] = apcOld[i];
		}
	}
	if (apcOld)
	{
		free(apcOld);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds an empty define called pcName.  Returns NULL if it is already
// defined.
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::AddDefine(CExternalString* pcName)
{
	CDefine*	pcDefine;
	uint32		uiHash;
	size		uiSlot;

	if ((muiDefines + 1) * 2 > muiCapacity)
	{
		Grow();
	}
	uiHash = Hash(pcName->msz, pcName->miLen);
	uiSlot = Find(pcName->msz, pcName->miLen, uiHash);
	if (mapcTable[uiSlot])
	{
		return NULL;
	}

	pcDefine = (CDefine*)mcDefines.Add(sizeof(CDefine));
	pcDefine->Init(pcName, mlliNextID, this);
	pcDefine->SetNameHash(uiHash);
	mlliNextID++;
	mapcTable[uiSlot] = pcDefine;
	muiDefines++;
	return pcDefine;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds an empty define called szName.  Returns NULL if it is already
// defined.
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::AddDefine(char* szName)
{
	CExternalString		cExternalString;

	cExternalString.Init(szName, StrLen(szName));
	return AddDefine(&cExternalString);
}


//////////////////////////////////////////////////////////////////////////
//
// The define called pcName, or NULL.  bExact false finds the longest name
// defined that pcName starts with.
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::Get(CExternalString* pcName, bool bExact)
{
	CDefine*	pcDefine;
	size		uiLength;

	if (bExact)
	{
		return GetExact(pcName->msz, pcName->miLen);
	}
	for (uiLength = pcName->miLen; uiLength > 0; uiLength--)
	{
		pcDefine = GetExact(pcName->msz, uiLength);
		if (pcDefine)
		{
			return pcDefine;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::Get(char* szName, bool bExact)
{
	CExternalString		cExternalString;

	cExternalString.Init(szName, StrLen(szName));
	return Get(&cExternalString, bExact);
}


//////////////////////////////////////////////////////////////////////////
//
// Kills and removes the define called pcName, if there is one.  The
// defines after it in the table move back into the gap.
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::Remove(CExternalString* pcName)
{
	size		uiMask;
	size		i;
	size		j;
	size		uiHome;
	CDefine*	pcDefine;

	if (muiDefines == 0)
	{
		return;
	}
	i = Find(pcName->msz, pcName->miLen, Hash(pcName->msz, pcName->miLen));
	pcDefine = mapcTable[i];
	if (!pcDefine)
	{
		return;
	}
	pcDefine->Kill();
	muiDefines--;

	uiMask = muiCapacity - 1;
	for (j = (i + 1) & uiMask; mapcTable[j] != NULL; j = (j + 1) & uiMask)
	{
		//A define stays where it is if its home slot is cyclically after the gap and at or before it.
		uiHome = mapcTable[j]->GetNameHash() & uiMask;
		if (((j > i) && ((uiHome <= i) || (uiHome > j))) || ((j < i) && ((uiHome <= i) && (uiHome > j))))
		{
			mapcTable[i] = mapcTable[j];
			i = j;
		}
	}
	mapcTable[i] = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::Remove(char* szName)
{
	CExternalString		cExternalString;

	cExternalString.Init(szName, StrLen(szName));
	Remove(&cExternalString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDefineMap::NumDefines(void)
{
	return muiDefines;
}


//////////////////////////////////////////////////////////////////////////
//
// Dumps every define in name order.
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::Dump(void)
{
	CDefine**	apcDefines;
	size		uiNum;
	size		i;
	size		j;
	CDefine*	pcDefine;

	apcDefines = (CDefine**)malloc((muiDefines + 1) * sizeof(CDefine*));
	uiNum = 0;
	for (i = 0; i < muiCapacity; i++)
	{
		if (mapcTable[i])
		{
			//Insertion sort: dumping is for debugging.
			pcDefine = mapcTable[i];
			for (j = uiNum; (j > 0) && (strcmp(apcDefines[j - 1]->GetName(), pcDefine->GetName()) > 0); j--)
			{
				apcDefines[j] = apcDefines[j - 1];
			}
			apcDefines[j] = pcDefine;
			uiNum++;
		}
	}
	for (i = 0; i < uiNum; i++)
	{
		apcDefines[i]->Dump();
	}
	free(apcDefines);
}
