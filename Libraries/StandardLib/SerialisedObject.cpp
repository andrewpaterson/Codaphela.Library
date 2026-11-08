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
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "SerialisedObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSerialisedObject::IsNamed(void)
{
	bool bNamed;

	bNamed = meType == OBJECT_POINTER_NAMED;
	if (!bNamed)
	{
		return false;
	}

	if (msName.iLength == 1)
	{
		return false;
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////
////
////
////////////////////////////////////////////////////////////////////////////
bool CSerialisedObject::IsIndexed(void)
{
	bool bIndexed;
	bool bNamed;

	bIndexed = meType == OBJECT_POINTER_ID;
	if (bIndexed)
	{
		return true;
	}

	bNamed = meType == OBJECT_POINTER_NAMED;
	if (bNamed && msName.iLength == 1)
	{
		return true;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSerialisedObject::IsVoid(void)
{
	return meType == OBJECT_POINTER_NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CSerialisedObject::GetName(void)
{
	return msName.sz;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CSerialisedObject::GetIndex(void)
{
	return moi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CSerialisedObject::GetLength(void)
{
	return miLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSerialisedObject::SetLength(size iLength)
{
	if ((uint64)iLength > MAX_UINT32)
	{
		return gcLogger.Error2(__METHOD__, " A serialised object can't be [", SizeToString(iLength), "] bytes long (4GB at most).", NULL);
	}
	miLength = (uint32)iLength;
	return true;
}

