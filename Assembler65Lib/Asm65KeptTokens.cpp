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
#include "Asm65KeptTokens.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65KeptTokens::Init(void)
{
	masTokens.Init();
	mszText.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65KeptTokens::Kill(void)
{
	mszText.Kill();
	masTokens.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65KeptTokens::Clear(void)
{
	Kill();
	Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65Token* CAsm65KeptTokens::Add(SAsm65Token* psToken)
{
	SAsm65KeptToken*	psKept;

	psKept = masTokens.Add();
	psKept->sToken = *psToken;
	psKept->uiText = mszText.Length();
	mszText.Append(psToken->szText, (lsize)psToken->uiLength);
	mszText.Append('\0');
	return Get(masTokens.NumElements() - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CAsm65KeptTokens::NumTokens(void)
{
	return masTokens.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// The text moves as more tokens are kept, so the token's text is pointed at
// its copy each time it's got.
//
//////////////////////////////////////////////////////////////////////////
SAsm65Token* CAsm65KeptTokens::Get(lsize uiIndex)
{
	SAsm65KeptToken*	psKept;

	psKept = masTokens.Get(uiIndex);
	psKept->sToken.szText = mszText.Text(psKept->uiText);
	return &psKept->sToken;
}
