/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Numbers.h"
#include "DefineMap.h"
#include "BaseLib/FNVHash.h"


#define DEFINE_MAP_CHUNK			(16 KB)


void CDefineMap::Init(void)
{
	mcDefinesByName.Init();
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

	for (i = 0; i < mcDefinesByName.NumElements(); i++)
	{
		(*mcDefinesByName.GetValue(i))->Kill();
	}
	mcDefinesByName.Kill();
	mcDefines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::GetExact(const char* szName, size uiLength)
{
	SHashText	sName;
	CDefine**	ppcDefine;

	sName.Init(szName, uiLength);
	ppcDefine = mcDefinesByName.Get(&sName);
	return ppcDefine ? *ppcDefine : NULL;
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
	SHashText	sName;

	if (GetExact(pcName->msz, pcName->miLen))
	{
		return NULL;
	}
	pcDefine = (CDefine*)mcDefines.Add(sizeof(CDefine));
	pcDefine->Init(pcName, mlliNextID, this);
	mlliNextID++;

	//Keyed by the define's own copy of its name, which lasts as long as it is in the map.
	sName.Init(pcDefine->GetName(), pcDefine->GetNameLength());
	mcDefinesByName.Put(&sName, &pcDefine);
	return pcDefine;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds an empty define called szName.  Returns NULL if it is already
// defined.
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::AddDefine(const char* szName)
{
	CExternalString		cExternalString;

	cExternalString.Init(szName, StrLen(szName));
	return AddDefine(&cExternalString);
}


//////////////////////////////////////////////////////////////////////////
//
// The define called pcName, or NULL.
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::Get(CExternalString* pcName)
{
	return GetExact(pcName->msz, pcName->miLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDefine* CDefineMap::Get(const char* szName)
{
	CExternalString		cExternalString;

	cExternalString.Init(szName, StrLen(szName));
	return Get(&cExternalString);
}


//////////////////////////////////////////////////////////////////////////
//
// Kills and removes the define called pcName, if there is one.
//
//////////////////////////////////////////////////////////////////////////
void CDefineMap::Remove(CExternalString* pcName)
{
	SHashText	sName;
	size		uiIndex;
	CDefine*	pcDefine;

	sName.Init(pcName->msz, pcName->miLen);
	uiIndex = mcDefinesByName.Find(&sName);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return;
	}
	pcDefine = *mcDefinesByName.GetValue(uiIndex);
	mcDefinesByName.RemoveAt(uiIndex);
	pcDefine->Kill();
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
size CDefineMap::NumDefines(void) const
{
	return mcDefinesByName.NumElements();
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

	apcDefines = (CDefine**)malloc((mcDefinesByName.NumElements() + 1) * sizeof(CDefine*));
	uiNum = 0;
	for (i = 0; i < mcDefinesByName.NumElements(); i++)
	{
		//Insertion sort: dumping is for debugging.
		pcDefine = *mcDefinesByName.GetValue(i);
		for (j = uiNum; (j > 0) && (strcmp(apcDefines[j - 1]->GetName(), pcDefine->GetName()) > 0); j--)
		{
			apcDefines[j] = apcDefines[j - 1];
		}
		apcDefines[j] = pcDefine;
		uiNum++;
	}
	for (i = 0; i < uiNum; i++)
	{
		apcDefines[i]->Dump();
	}
	free(apcDefines);
}
