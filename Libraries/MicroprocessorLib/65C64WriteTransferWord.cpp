/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

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
#include "65C64.h"
#include "65C64WriteTransferWord.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64WriteTransferWord::Init(uint8 uiWord)
{
	C65C64DataOperation::Init(false, false, true, false, true);
	muiWord = uiWord;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64WriteTransferWord::Execute(C65C64* pcCPU)
{
	pcCPU->SetIOData(pcCPU->GetWriteWord(muiWord));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65C64WriteTransferWord::IsInitialSide(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64WriteTransferWord::Print(CChars* psz)
{
	psz->Append("Write(T");
	psz->Append((uint32)muiWord);
	psz->Append(")");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65C64WriteTransferWord::MustExecute(C65C64* pcCPU)
{
	return muiWord < pcCPU->GetNumWriteWords();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65C64WriteTransferWord::IsConditional(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 C65C64WriteTransferWord::GetByteLanes(C65C64* pcCPU)
{
	return pcCPU->GetWriteWordLanes(muiWord);
}

