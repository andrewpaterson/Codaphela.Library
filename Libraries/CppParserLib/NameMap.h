/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __NAME_MAP_H__
#define __NAME_MAP_H__
#include "BaseLib/IndexPrimitiveTemplate.h"
#include "BaseLib/ExternalString.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/Chars.h"
#include "ASCIITree.h"
#include "ASCIINameIndex.h"


//A name to ID tree and an ID to value index.  The values (D) are stored by
//value and must have Kill and Dump.  The subclass adds values with AddEntry
//and then Inits them, because each value type Inits differently.
template<class D>
class CNameMap
{
protected:
	CASCIITree							mcNameToIDIndex;
	CIndexPrimitiveTemplate<int64, D>	mcIDToValueIndex;

public:
	void 	Init(void);
	void 	Kill(void);
	D*		Get(CExternalString* pcName, bool bExact);
	D*		Get(CChars* pszName, bool bExact);
	D*		Get(char* szName, bool bExact);
	void	Remove(CExternalString* pcName);
	void	Remove(char* szName);
	void	Dump(void);

protected:
	D*		AddEntry(CExternalString* pcName, int64* plliID);
};


//////////////////////////////////////////////////////////////////////////
//
// Starts both indices empty.
//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CNameMap<D>::Init(void)
{
	mcIDToValueIndex.Init();
	mcNameToIDIndex.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// Kills every value and then both indices.
//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CNameMap<D>::Kill(void)
{
	D*								pcValue;
	SIndexTreeMemoryUnsafeIterator	sIter;
	int64							lli;
	bool							bHasNext;

	bHasNext = mcIDToValueIndex.StartIteration(&sIter, &pcValue, &lli);
	while (bHasNext)
	{
		pcValue->Kill();
		bHasNext = mcIDToValueIndex.Iterate(&sIter, &pcValue, &lli);
	}

	mcIDToValueIndex.Kill();
	mcNameToIDIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the name and returns the uninitialised value stored for it, with
// its ID in plliID.  Returns NULL if the name is already in the map or
// either index could not take it.
//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CNameMap<D>::AddEntry(CExternalString* pcName, int64* plliID)
{
	D		cValue;
	int64	lliID;

	if (mcNameToIDIndex.Get(pcName->msz, pcName->EndInclusive(), true) != NULL)
	{
		return NULL;
	}

	lliID = mcNameToIDIndex.Add(pcName->msz, pcName->EndInclusive());
	if (lliID == -1)
	{
		return NULL;
	}

	if (!mcIDToValueIndex.Put(lliID, &cValue))
	{
		mcNameToIDIndex.Remove(pcName->msz, pcName->EndInclusive());
		return NULL;
	}

	*plliID = lliID;
	return mcIDToValueIndex.Get(lliID);
}


//////////////////////////////////////////////////////////////////////////
//
// Returns the value for the name, or NULL.  bExact false matches the
// longest name in the map that pcName starts with.
//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CNameMap<D>::Get(CExternalString* pcName, bool bExact)
{
	SASCIINameIndex*	psNameIndex;

	psNameIndex = mcNameToIDIndex.Get(pcName->msz, pcName->EndInclusive(), bExact);
	if (psNameIndex)
	{
		return mcIDToValueIndex.Get(psNameIndex->mlliID);
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Returns the value for the name in pszName, or NULL.
//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CNameMap<D>::Get(CChars* pszName, bool bExact)
{
	CExternalString	cExternalString;

	cExternalString.Init(pszName->Text(), pszName->Length());
	return Get(&cExternalString, bExact);
}


//////////////////////////////////////////////////////////////////////////
//
// Returns the value for the zero terminated name, or NULL.
//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CNameMap<D>::Get(char* szName, bool bExact)
{
	CExternalString	cExternalString;

	cExternalString.Init(szName, StrLen(szName));
	return Get(&cExternalString, bExact);
}


//////////////////////////////////////////////////////////////////////////
//
// Kills and removes the value for the name, if it is in the map.
//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CNameMap<D>::Remove(CExternalString* pcName)
{
	SASCIINameIndex*	psNameIndex;
	D*					pcValue;

	psNameIndex = mcNameToIDIndex.Get(pcName->msz, pcName->EndInclusive(), true);
	if (psNameIndex)
	{
		pcValue = mcIDToValueIndex.Get(psNameIndex->mlliID);
		if (pcValue)
		{
			pcValue->Kill();
			//The ID first: removing the name frees psNameIndex.
			mcIDToValueIndex.Remove(psNameIndex->mlliID);
			mcNameToIDIndex.Remove(pcName->msz, pcName->EndInclusive());
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Kills and removes the value for the zero terminated name, if it is in
// the map.
//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CNameMap<D>::Remove(char* szName)
{
	CExternalString	cExternalString;

	cExternalString.Init(szName, StrLen(szName));
	Remove(&cExternalString);
}


//////////////////////////////////////////////////////////////////////////
//
// Dumps every value in name order.
//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CNameMap<D>::Dump(void)
{
	SIndexTreeMemoryUnsafeIterator	sIter;
	int64							lliID;
	D*								pcValue;

	lliID = mcNameToIDIndex.StartIteration(&sIter);
	while (lliID != -1)
	{
		pcValue = mcIDToValueIndex.Get(lliID);
		pcValue->Dump();

		lliID = mcNameToIDIndex.Iterate(&sIter);
	}
}


#endif // !__NAME_MAP_H__

