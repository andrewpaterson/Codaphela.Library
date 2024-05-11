/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "Define.h"
#include "Chars.h"
#include "ExternalString.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalString::Init(void)
{
	Clear();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalString::Init(char* sz, size iLen)
{
	msz = sz;
	miLen = iLen;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalString::Init(char* szStart, char* szLastCharInclusive)
{
	msz = szStart;
	miLen = StrLen(szStart, szLastCharInclusive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalString::Clear(void)
{
	msz = NULL;
	miLen = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CExternalString::EndInclusive(void)
{
	if (miLen != 0)
	{
		return msz + (miLen - 1);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalString::Dump(void)
{
	CChars	sz;

	sz.Init(msz, 0, miLen);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExternalString::Equals(CExternalString* pcOther)
{
	if (pcOther->miLen == miLen)
	{
		if (miLen == 0)
		{
			return true;
		}

		if (memcmp(msz, pcOther->msz, miLen) == 0)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExternalString::Equals(char* sz)
{
	size iLen;

	iLen = strlen(sz);
	if (iLen == miLen)
	{
		if (miLen == 0)
		{
			return true;
		}

		if (memcmp(msz, sz, miLen) == 0)
		{
			return true;
		}
	}
	return false;
}

