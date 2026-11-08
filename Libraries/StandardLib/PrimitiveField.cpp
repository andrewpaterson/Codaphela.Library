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
#include "PrimitiveObject.h"
#include "PrimitiveField.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPrimitiveField::Init(CClass* pcFieldClass, ptrdiff_t iObjectOffset, ptrdiff_t iValueOffset, CClass* pcContainingClass, SDataIO* pcIO, const char* szName)
{
	//CClass only makes fields inside the object (the offsets aren't negative).
	CField::Init(pcFieldClass, iObjectOffset, pcContainingClass, szName);
	muiValueOffset = iValueOffset;
	mpcIO = pcIO;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPrimitiveField::Kill(void)
{
	CField::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPrimitiveField::IsPrimitive(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPrimitiveField::GetSizeOf(void)
{
	//This is the size of the whole primitive object; not just the primitive part.
	return mpcFieldClass->GetSizeOf();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPrimitiveObject* CPrimitiveField::GetPrimitiveObject(CBaseObject* pcFieldContainer)
{
	CPrimitiveObject* pcPrimitive;

	pcPrimitive = (CPrimitiveObject*)RemapSinglePointer(pcFieldContainer, muiOffset);
	return pcPrimitive;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CPrimitiveField::GetValue(CBaseObject* pcFieldContainer)
{
	return RemapSinglePointer(pcFieldContainer, muiValueOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPrimitiveField::GetNameOffset(void)
{
	return sizeof(CPrimitiveField);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDataIO* CPrimitiveField::GetDataIO(void)
{
	return mpcIO;
}

