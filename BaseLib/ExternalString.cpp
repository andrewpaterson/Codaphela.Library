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
#include "BaseLib/Define.h"
#include "BaseLib/Chars.h"
#include "ExternalString.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalString::Init(void)
{
	msz = NULL;
	miLen = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalString::Init(char* sz, int iLen)
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
	miLen = StrLen(szLastCharInclusive);
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
BOOL CExternalString::Equals(CExternalString* pcOther)
{
	if (pcOther->miLen == miLen)
	{
		if (miLen == 0)
		{
			return TRUE;
		}

		if (memcmp(msz, pcOther->msz, miLen) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CExternalString::Equals(char* sz)
{
	int iLen;

	iLen = (int)strlen(sz);
	if (iLen == miLen)
	{
		if (miLen == 0)
		{
			return TRUE;
		}

		if (memcmp(msz, sz, miLen) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

