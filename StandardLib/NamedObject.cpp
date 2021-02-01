/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "NamedObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedObject::FreeIdentifiers(void)
{
	mon.Kill();
	CObject::FreeIdentifiers();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CNamedObject::GetName(void)
{
	return mon.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedObject::IsNamed(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedObject::IsNamed(char* szName)
{
	if (szName != NULL)
	{
		return mon.Equals(szName);
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedObject::InitName(char* szName)
{
	mon.Init(szName);
	return ClipName();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedObject::SetName(char* szName)
{
	mon.Set(szName);
	return ClipName();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedObject::ClipName(void)
{
	BOOL	bResult;

	bResult = TRUE;
	if (mon.Contains("\\"))
	{
		mon.Replace("\\", "/");
		bResult = FALSE;
	}
	if (mon.Length() >= MAX_NAMED_OBJECT_NAME_LENGTH)
	{
		mon.SetLength(MAX_NAMED_OBJECT_NAME_LENGTH - 1);
		bResult = FALSE;
	}
	
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedObject::ClearName(void)
{
	mon.Kill();
	mon.Init();
}

