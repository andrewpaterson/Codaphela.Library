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
#include "Asm65Errors.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Errors::Init(void)
{
	mszErrors.Init();
	muiErrors = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Errors::Kill(void)
{
	mszErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Errors::Clear(void)
{
	mszErrors.Clear();
	muiErrors = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Errors::Add(uint32 uiLine, uint32 uiColumn, const char* szMessage, const char* szDetail)
{
	mszErrors.Append(uiLine);
	mszErrors.Append(':');
	mszErrors.Append(uiColumn);
	mszErrors.Append(": ");
	mszErrors.Append(szMessage);
	if (szDetail)
	{
		mszErrors.Append(" '");
		mszErrors.Append(szDetail);
		mszErrors.Append('\'');
	}
	mszErrors.AppendNewLine();
	muiErrors++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Errors::NumErrors(void)
{
	return muiErrors;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65Errors::Text(void)
{
	return mszErrors.Text();
}
