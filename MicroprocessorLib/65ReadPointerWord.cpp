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
#include "65ReadPointerWord.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65ReadPointerWord::Init(uint8 uiWord)
{
	C65DataOperation::Init(false, false, true, true, true);
	muiWord = uiWord;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65ReadPointerWord::Execute(C65C64* pcCPU)
{
	pcCPU->SetPointerWord(muiWord, pcCPU->GetIOData());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65ReadPointerWord::IsInitialSide(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65ReadPointerWord::Print(CChars* psz)
{
	psz->Append("Read(P");
	psz->Append((uint32)muiWord);
	psz->Append(")");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65ReadPointerWord::MustExecute(C65C64* pcCPU)
{
	return muiWord < pcCPU->GetNumPointerWords();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 C65ReadPointerWord::GetByteLanes(C65C64* pcCPU)
{
	return pcCPU->GetPointerWordLanes(muiWord);
}

