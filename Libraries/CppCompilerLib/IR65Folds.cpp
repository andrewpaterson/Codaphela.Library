/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "IR65BackendInternal.h"


//The 65C64 back end's folds: a ptradd of a constant made part of the address of the load or store that alone
//uses it (MarkFolded).


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Folds::Init(CIR65Backend* pcBackend)
{
	mpcBackend = pcBackend;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Folds::Kill(void)
{
	mpcBackend = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A ptradd of a constant just before the load or store that alone uses it
// becomes part of the load or store's address.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Folds::MarkFolded(void)
{
	size			ui;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRInstruction*	psPrevious;
	SIRReference	sAddress;
	SIR65Place*		psPlace;

	for (ui = 0; ui < mpcBackend->mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcBackend->mpcFunction->GetBlock(ui);
		for (uiInstruction = 1; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			sAddress = IRAddressOf(psInstruction);
			if (sAddress.IsNone())
			{
				continue;
			}
			psPrevious = pcBlock->masInstructions.Get(uiInstruction - 1);
			if (mpcBackend->ConstantPtrAdd(psPrevious) && psPrevious->sResult.Equals(sAddress) && (mpcBackend->masTemporaries.Get(sAddress.Index())->uiUses == 1))
			{
				psPlace = mpcBackend->mcPlaces.PlaceOf(sAddress);
				psPlace->uiKind = IR65P_Folded;
				psPlace->iOffset = (int32)mpcBackend->mpcFunction->GetSigned(psPrevious->asArguments[1]);
				psPlace->sBase = psPrevious->asArguments[0];
				psPlace->bInA = false;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if psNext is a load or store whose address is a folded ptradd of
// sValue.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Folds::FoldedBase(SIRInstruction* psNext, SIRReference sValue)
{
	SIRReference	sAddress;

	sAddress = IRAddressOf(psNext);
	return sAddress.IsTemporary() && (mpcBackend->mcPlaces.PlaceOf(sAddress)->uiKind == IR65P_Folded) && mpcBackend->mcPlaces.PlaceOf(sAddress)->sBase.Equals(sValue);
}
