/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "PointerFunctions.h"
#include "ArrayNameIntAndPointer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayNameIntAndPointer::Add(void* pvData, uint uiType, char* szName)
{
	SNameIntAndPointer*	psPtr;

	psPtr = CArrayTemplate<SNameIntAndPointer>::Add();
	psPtr->uiType = uiType;
	psPtr->pvData = pvData;
	psPtr->szName = szName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayNameIntAndPointer::Get(size uiIndex, void** ppvData, uint* uiType, char** pszName)
{
	SNameIntAndPointer*	psPtr;

	psPtr = CArrayTemplate<SNameIntAndPointer>::SafeGet(uiIndex);
	if (psPtr)
	{
		SafeAssign(ppvData, psPtr->pvData);
		SafeAssign(uiType, psPtr->uiType);
		SafeAssign(pszName, psPtr->szName);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayNameIntAndPointer::GetPtr(size uiIndex)
{
	SNameIntAndPointer* psPtr;

	if (uiIndex >= miUsedElements)
	{
		return NULL;
	}
	else
	{
		psPtr = CArrayTemplate<SNameIntAndPointer>::Get(uiIndex);
		return psPtr->pvData;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayNameIntAndPointer::GetType(size uiIndex)
{
	SNameIntAndPointer*	psPtr;

	if (uiIndex >= miUsedElements)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	else
	{
		psPtr = CArrayTemplate<SNameIntAndPointer>::Get(uiIndex);
		return psPtr->uiType;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CArrayNameIntAndPointer::GetName(size uiIndex)
{
	SNameIntAndPointer* psPtr;

	if (uiIndex >= miUsedElements)
	{
		return NULL;
	}
	else
	{
		psPtr = CArrayTemplate<SNameIntAndPointer>::Get(uiIndex);
		return psPtr->szName;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayNameIntAndPointer::Set(size uiIndex, void* pvData, uint uiType, char* szName)
{
	SNameIntAndPointer* psPtr;

	psPtr = CArrayTemplate<SNameIntAndPointer>::SafeGet(uiIndex);
	if (psPtr)
	{
		psPtr->uiType = uiType;
		psPtr->pvData = pvData;
		psPtr->szName = szName;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayNameIntAndPointer::RemoveWithData(void* pv)
{
	size	iIndex;

	iIndex = FindWithKey((SNameIntAndPointer*)pv, 0, sizeof(void*));
	if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		RemoveAt(iIndex);
	}
}

