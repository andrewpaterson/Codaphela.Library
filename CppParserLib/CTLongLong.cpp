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
#include "CTLongLong.h"
#include "CppTokeniser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCTLongLong::Init(void)
{
	meType = CTNT_LongLong;
	mcValue = 0;
	mbSigned = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCTLongLong::Init(unsigned long long int ulli)
{
	meType = CTNT_LongLong;
	mcValue = ulli;
	mbSigned = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCTLongLong::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCTLongLong::IsSigned(void)
{
	return mbSigned;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCTLongLong::Append(CChars* psz, CTokeniser* pcTokeniser)
{
	psz->Append("(");
	if (mbSigned)
	{
		psz->Append("signed");
	}
	else
	{
		psz->Append("unsigned");
	}
	psz->Append("long long int)");
	psz->Append((unsigned int)mcValue);
}
