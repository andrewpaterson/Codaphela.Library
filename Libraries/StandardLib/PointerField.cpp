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
#include "Class.h"
#include "Classes.h"
#include "Pointer.h"
#include "PointerField.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointerField::Init(ptrdiff_t iOffset, CClass* pcContainingClass, const char* szName)
{
	CClasses*	pcClasses;

	//CClass only makes fields inside the object (iOffset isn't negative).
	pcClasses = pcContainingClass->GetClasses();
	CField::Init(pcClasses->GetPointer(), iOffset, pcContainingClass, szName);

	mbDoNoFreePointedTo = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointerField::Kill(void)
{
	CField::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointerField::IsPointer(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPointerField::GetSizeOf(void)
{
	return mpcFieldClass->GetSizeOf();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer* CPointerField::GetPointer(CBaseObject* pcFieldContainer)
{
	CPointer*	pcPointer;

	pcPointer = (CPointer*)RemapSinglePointer(pcFieldContainer, muiOffset);
	return pcPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPointerField::GetNameOffset(void)
{
	return sizeof(CPointerField);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointerField::DoNotFreePointedTo(bool bValue)
{
	mbDoNoFreePointedTo = bValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointerField::IsDoNotFreePointedTo(void)
{
	return mbDoNoFreePointedTo;
}

