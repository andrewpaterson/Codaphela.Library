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
#include "EmbeddedObjectField.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObjectField::Init(CClass* pcFieldClass, size uiOffset, CClass* pcContainingClass, const char* szName)
{
	CField::Init(pcFieldClass, uiOffset, pcContainingClass, szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObjectField::Kill(void)
{
	CField::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObjectField::IsEmbeddedObject(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CEmbeddedObjectField::GetSizeOf(void)
{
	return mpcFieldClass->GetSizeOf();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CEmbeddedObjectField::GetEmbeddedObject(CBaseObject* pcFieldContainer)
{
	CBaseObject*	pcEmbeddedObject;

	pcEmbeddedObject = (CBaseObject*)RemapSinglePointer(pcFieldContainer, muiOffset);
	return pcEmbeddedObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CEmbeddedObjectField::GetNameOffset(void)
{
	return sizeof(CEmbeddedObjectField);
}