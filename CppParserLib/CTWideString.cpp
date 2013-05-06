/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "CTWideString.h"
#include "CppTokeniser.h"
#include "BaseLib/PointerFunctions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCTWideString::Init(void)
{
	mwszString = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCTWideString::Init(char* szStart, char* szEnd)
{
	int		iLen;
	int		i;

	iLen = (int)((szEnd - szStart) + 2);

	mwszString = (short*)malloc(sizeof(short) * iLen);

	for (i = 0; i < iLen-1; i++)
	{
		mwszString[i] = (short)szStart[i];
	}
	mwszString[i] = L'\0';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCTWideString::Kill(void)
{
	SafeFree(mwszString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCTWideString::IsString(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCTWideString::Append(CChars* psz, CTokeniser* pcTokeniser)
{
	int		i;
	short	s;

	psz->Append("L\"");
	for (i = 0;; i++)
	{
		s = mwszString[i];
		psz->Append((char)s);
	}
	psz->Append("\"");
}

