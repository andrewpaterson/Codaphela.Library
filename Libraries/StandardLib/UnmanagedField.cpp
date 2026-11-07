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
#include "BaseLib/TypeNames.h"
#include "Class.h"
#include "Classes.h"
#include "UnmanagedField.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUnmanagedField::Init(EPrimitiveType eType, ptrdiff_t iOffset, CClass* pcContainingClass, const char* szName)
{
	Init(eType, iOffset, pcContainingClass, 1, szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUnmanagedField::Init(EPrimitiveType eType, ptrdiff_t iOffset, CClass* pcContainingClass, size uiLength, const char* szName)
{
	CClasses*	pcClasses;
	CClass*		pcFieldClass;

	pcClasses = pcContainingClass->GetClasses();
	pcFieldClass = pcClasses->GetValid(eType);
	CField::Init(pcFieldClass, iOffset, pcContainingClass, szName);
	meType = eType;
	if (pcFieldClass != NULL)
	{
		muiElementSize = pcFieldClass->GetSizeOf();
		muiLength = uiLength;
	}
	else
	{
		//GetValid has reported it.  The field is kept, named, but has nothing to save.
		muiElementSize = 0;
		muiLength = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUnmanagedField::Init(EPrimitiveType eType, ptrdiff_t iOffset, CClass* pcContainingClass, size uiElementSize, size uiLength, const char* szName)
{
	CClasses*	pcClasses;
	CClass*		pcFieldClass;

	pcClasses = pcContainingClass->GetClasses();
	pcFieldClass = pcClasses->Get(eType);
	CField::Init(pcFieldClass, iOffset, pcContainingClass, szName);
	meType = eType;
	muiElementSize = uiElementSize;
	muiLength = uiLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUnmanagedField::Kill(void)
{
	CField::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CUnmanagedField::IsArray(void)
{
	return muiLength > 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CUnmanagedField::GetLength(void)
{
	return muiLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CUnmanagedField::GetSizeOf(void)
{
	return muiElementSize;
}


//////////////////////////////////////////////////////////////////////////
//
// What the field was registered as, which decides how it is saved (see
// CBaseObject::SaveUnmanagedField).  U_Data has no class of its own.
//
//////////////////////////////////////////////////////////////////////////
EPrimitiveType CUnmanagedField::GetType(void)
{
	return meType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CUnmanagedField::IsUnmanaged(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CUnmanagedField::GetData(CBaseObject* pcFieldContainer)
{
	return RemapSinglePointer(pcFieldContainer, muiOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CUnmanagedField::GetNameOffset(void)
{
	return sizeof(CUnmanagedField);
}

