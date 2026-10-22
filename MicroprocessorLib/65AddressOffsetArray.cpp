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
#include "BaseLib/PointerFunctions.h"
#include "65AddressOffsetArray.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65AddressOffsetArray::Kill(void)
{
	size				i;
	size				uiNumElements;
	C65AddressOffset*	pcOffset;

	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcOffset = __C65AddressOffsetArray::GetPtr(i);
		SafeKill(pcOffset);
	}

	__C65AddressOffsetArray::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 C65AddressOffsetArray::Calculate(C65C64* pcCPU)
{
	size				i;
	size				uiNumElements;
	C65AddressOffset*	pcOffset;
	uint32				uiAddress;

	uiAddress = 0;
	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcOffset = GetPtr(i);
		uiAddress += pcOffset->GetOffset(pcCPU);
	}

	//The data bus is 16 bits.  Every bus cycle addresses an aligned word.
	return uiAddress & 0xFFFFFFFE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65AddressOffsetArray::IsProgramCounter(void)
{
	size				i;
	size				uiNumElements;
	C65AddressOffset*	pcOffset;

	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcOffset = GetPtr(i);
		if (pcOffset->IsProgramCounter())
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
void C65AddressOffsetArray::Print(CChars* psz)
{
	size				i;
	size				uiNumElements;
	C65AddressOffset*	pcOffset;

	uiNumElements = NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcOffset = GetPtr(i);
		if ((i != 0) && !pcOffset->IsConstantOffset())
		{
			psz->Append("+");
		}
		pcOffset->Print(psz);
	}
}

