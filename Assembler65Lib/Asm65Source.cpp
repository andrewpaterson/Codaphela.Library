/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "Asm65Source.h"


//////////////////////////////////////////////////////////////////////////
//
// The text must outlive the source.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65TextSource::Init(const char* szText)
{
	mszText = szText;
	Rewind();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65TextSource::Kill(void)
{
	mszText = NULL;
	mszPos = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65TextSource::Rewind(void)
{
	mszPos = mszText;
	mbDone = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CAsm65TextSource::NextLine(void)
{
	const char*	szLine;
	const char*	szEnd;

	if (mbDone)
	{
		return NULL;
	}
	szLine = mszPos;
	szEnd = strchr(mszPos, '\n');
	if (szEnd)
	{
		mszPos = szEnd + 1;
	}
	else
	{
		mbDone = true;
	}
	return szLine;
}
