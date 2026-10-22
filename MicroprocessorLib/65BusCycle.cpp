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
#include "BaseLib/Logger.h"
#include "65BusCycle.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65BusCycle::Init(C65AddressOffsetArray* papcAddressOffsets, C65OperationArray* papcOperations)
{
	size			uiNumOperations;
	size			i;
	C65Operation*	pcOperation;

	memcpy(&mapcAddressOffsets, papcAddressOffsets, sizeof(C65AddressOffsetArray));
	free(papcAddressOffsets);

	memcpy(&mapcOperations, papcOperations, sizeof(C65OperationArray));
	free(papcOperations);

	miCycle = -1;
	mpcDataOperation = NULL;

	uiNumOperations = mapcOperations.NumElements();
	for (i = 0; i < uiNumOperations; i++)
	{
		pcOperation = mapcOperations.GetPtr(i);
		if (pcOperation->IsData())
		{
			mpcDataOperation = (C65DataOperation*)pcOperation;
		}
	}

	Validate();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65BusCycle::Kill(void)
{
	mpcDataOperation = NULL;

	mapcAddressOffsets.Kill();
	mapcOperations.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65BusCycle::Validate(void)
{
	size			uiNumOperations;
	size			i;
	C65Operation*	pcOperation;
	size			uiDataOperations;

	uiDataOperations = 0;
	uiNumOperations = mapcOperations.NumElements();
	for (i = 0; i < uiNumOperations; i++)
	{
		pcOperation = mapcOperations.GetPtr(i);
		if (pcOperation->IsData())
		{
			uiDataOperations++;
		}
	}

	if (uiDataOperations != 1)
	{
		LOG_ERROR("Exactly [1] data bus operation must be specified in a bus cycle.");
	}

	if (mapcAddressOffsets.NumElements() == 0)
	{
		LOG_ERROR("At least [1] address offset must be specified in a bus cycle.");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 C65BusCycle::GetAddress(C65C64* pcCPU)
{
	return mapcAddressOffsets.Calculate(pcCPU);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65DataOperation* C65BusCycle::GetDataOperation(void)
{
	return mpcDataOperation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65BusCycle::MustExecute(C65C64* pcCPU)
{
	size			uiNumOperations;
	size			i;
	C65Operation*	pcOperation;

	uiNumOperations = mapcOperations.NumElements();
	for (i = 0; i < uiNumOperations; i++)
	{
		pcOperation = mapcOperations.GetPtr(i);
		if (!pcOperation->MustExecute(pcCPU))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65BusCycle::ExecuteInitialSide(C65C64* pcCPU)
{
	size			uiNumOperations;
	size			i;
	C65Operation*	pcOperation;

	uiNumOperations = mapcOperations.NumElements();
	for (i = 0; i < uiNumOperations; i++)
	{
		pcOperation = mapcOperations.GetPtr(i);
		if (pcOperation->IsInitialSide())
		{
			pcOperation->Execute(pcCPU);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65BusCycle::ExecuteTrailingSide(C65C64* pcCPU, bool bSkipData)
{
	size			uiNumOperations;
	size			i;
	C65Operation*	pcOperation;

	uiNumOperations = mapcOperations.NumElements();
	for (i = 0; i < uiNumOperations; i++)
	{
		pcOperation = mapcOperations.GetPtr(i);
		if (pcOperation->IsTrailingSide())
		{
			if (!(bSkipData && pcOperation->IsData()))
			{
				pcOperation->Execute(pcCPU);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65BusCycle::IsProgramAddress(void)
{
	return mapcAddressOffsets.IsProgramCounter();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65BusCycle::ToAddressOffsetString(CChars* psz)
{
	mapcAddressOffsets.Print(psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65BusCycle::ToOperationString(CChars* psz)
{
	size			uiNumOperations;
	size			i;
	C65Operation*	pcOperation;
	CChars			sz;

	uiNumOperations = mapcOperations.NumElements();
	for (i = 0; i < uiNumOperations; i++)
	{
		pcOperation = mapcOperations.GetPtr(i);
		sz.Init();
		pcOperation->Print(&sz);
		if (!sz.Empty())
		{
			if (!psz->Empty())
			{
				psz->Append(" ");
			}
			psz->Append(sz);
		}
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65BusCycle::SetCycle(int16 iCycle)
{
	miCycle = iCycle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int16 C65BusCycle::GetCycle(void)
{
	return miCycle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65OperationArray* C65BusCycle::GetOperations(void)
{
	return &mapcOperations;
}

