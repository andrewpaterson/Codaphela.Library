/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include <stdlib.h>
#include <string.h>
#include "NameSet.h"
#include "FNVHash.h"


#define NAME_SET_FIRST_CAPACITY	64


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNameSet::Init(void)
{
	masNames = NULL;
	muiCapacity = 0;
	muiNames = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNameSet::Kill(void)
{
	if (masNames)
	{
		free(masNames);
	}
	masNames = NULL;
	muiCapacity = 0;
	muiNames = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// FNV-1a.
//
//////////////////////////////////////////////////////////////////////////
uint64 CNameSet::Hash(const char* szName, size uiLength)
{
	return FNVHash64(szName, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNameSet::Contains(const char* szName, size uiLength)
{
	size			uiMask;
	size			i;
	SNameSetName*	psName;

	if (muiCapacity == 0)
	{
		return false;
	}
	uiMask = muiCapacity - 1;
	for (i = (size)Hash(szName, uiLength) & uiMask;; i = (i + 1) & uiMask)
	{
		psName = &masNames[i];
		if (psName->szName == NULL)
		{
			return false;
		}
		if ((psName->uiLength == uiLength) && (memcmp(psName->szName, szName, uiLength) == 0))
		{
			return true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the name (its text must last as long as the set).  False if it was
// in already.
//
//////////////////////////////////////////////////////////////////////////
bool CNameSet::Add(const char* szName, size uiLength)
{
	size			uiMask;
	size			i;
	SNameSetName*	psName;

	if ((muiNames + 1) * 2 > muiCapacity)
	{
		Grow();
	}
	uiMask = muiCapacity - 1;
	for (i = (size)Hash(szName, uiLength) & uiMask;; i = (i + 1) & uiMask)
	{
		psName = &masNames[i];
		if (psName->szName == NULL)
		{
			psName->szName = szName;
			psName->uiLength = (uint32)uiLength;
			muiNames++;
			return true;
		}
		if ((psName->uiLength == uiLength) && (memcmp(psName->szName, szName, uiLength) == 0))
		{
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNameSet::Grow(void)
{
	SNameSetName*	asOld;
	size			uiOldCapacity;
	size			ui;
	size			uiMask;
	size			i;

	asOld = masNames;
	uiOldCapacity = muiCapacity;
	muiCapacity = (muiCapacity == 0) ? NAME_SET_FIRST_CAPACITY : muiCapacity * 2;
	masNames = (SNameSetName*)malloc(muiCapacity * sizeof(SNameSetName));
	memset(masNames, 0, muiCapacity * sizeof(SNameSetName));
	uiMask = muiCapacity - 1;
	for (ui = 0; ui < uiOldCapacity; ui++)
	{
		if (asOld[ui].szName)
		{
			for (i = (size)Hash(asOld[ui].szName, asOld[ui].uiLength) & uiMask; masNames[i].szName != NULL; i = (i + 1) & uiMask)
			{
			}
			masNames[i] = asOld[ui];
		}
	}
	if (asOld)
	{
		free(asOld);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CNameSet::NumNames(void)
{
	return muiNames;
}
