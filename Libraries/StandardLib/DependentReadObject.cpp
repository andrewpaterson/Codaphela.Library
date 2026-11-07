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
#include "BaseLib/IntegerHelper.h"
#include "ObjectFileGeneral.h"
#include "DependentReadObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObject::Init(CObjectIdentifier* pcObjectIdentifier)
{
	meType = pcObjectIdentifier->meType;
	moi = pcObjectIdentifier->moi;
	mszObjectName.Init(pcObjectIdentifier->GetName());
	muiFlags = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObject::Kill(void)
{
	CObjectIdentifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CDependentReadObject::GetName(void)
{
	return mszObjectName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CDependentReadObject::GetOldIndex(void)
{
	return moi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObject::SetRead(void)
{
	muiFlags |= DEPENDENT_READ_OBJECT_FLAG_READ;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObject::SetExisting(void)
{
	muiFlags |= DEPENDENT_READ_OBJECT_FLAG_EXISTED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDependentReadObject::IsRead(void)
{
	return FixBool(muiFlags & DEPENDENT_READ_OBJECT_FLAG_READ);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDependentReadObject::ExistsInObjects(void)
{
	return FixBool(muiFlags & DEPENDENT_READ_OBJECT_FLAG_EXISTED);
}

