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
#include "SetObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetObject::Init(bool bSortPointers)
{
	CArrayCommonObject::Init(false, true, bSortPointers, bSortPointers);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CSetObject::UnsafeGet(size iIndex)
{
	CBaseObject* pcObject;
	CPointer		pObject;
	size			uiNumElements;

	uiNumElements = mcArray.UnsafeNumElements();
	if (iIndex < uiNumElements)
	{
		pcObject = (CBaseObject*)mcArray.UnsafeGet(iIndex);
		pObject.AssignObject(pcObject);
		return pObject;
	}
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CSetObject::UnsafeGetObject(size iIndex)
{
	CBaseObject*	pcObject;
	CPointer		pObject;
	size			uiNumElements;

	uiNumElements = mcArray.UnsafeNumElements();
	if (iIndex < uiNumElements)
	{
		pcObject = (CBaseObject*)mcArray.UnsafeGet(iIndex);
		return pcObject;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetObject::RemoveDuringIteration(SSetIterator* psIter)
{
	//The Object being removed must lose its from, as it does in Remove.

	CBaseObject*	pcObject;
	size			uiIndex;

	uiIndex = psIter->iIndex;
	pcObject = UnsafeGetObject(uiIndex);
	if (mcArray.RemoveDuringIteration(psIter))
	{
		MemberRemoved(pcObject, uiIndex);
		RemoveObjectTryFree(pcObject, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetObject::TouchAll(void)
{
	SSetIterator	sIter;
	CPointer		p;
	bool			bExists;

	p = StartIteration(&sIter, &bExists);
	while (bExists)
	{
		p = Iterate(&sIter, &bExists);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetObject::Dump(void)
{
	CChars	sz;

	sz.Init();
	PrintSlots(&sz);
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Every slot, a NULL where a member was removed (a set ignores NULLs, so
// it leaves them until it cleans): its index and the member's address.
//
//////////////////////////////////////////////////////////////////////////
void CSetObject::PrintSlots(CChars* psz)
{
	size				uiNumElements;
	size				ui;
	CEmbeddedObject*	pcEmbedded;
	CChars				szLineNumber;
	size				uiPadding;

	uiNumElements = mcArray.UnsafeNumElements();
	uiPadding = NumberOfDigits(uiNumElements);
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pcEmbedded = (CEmbeddedObject*)mcArray.UnsafeGet(ui);
		szLineNumber.Init();
		szLineNumber.Append(ui);
		szLineNumber.RightAlign(' ', uiPadding);
		szLineNumber.Append(": ");

		psz->Append(szLineNumber);
		psz->AppendPointer(pcEmbedded);
		psz->AppendNewLine();

		szLineNumber.Kill();
	}
}

