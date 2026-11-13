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


//The 65C64 back end's calls: the arguments pushed or put in registers, the callee's operand, a thiscall's this in X,
//and the clean up after it.


//////////////////////////////////////////////////////////////////////////
//
// The arg instructions before the call (CallArguments) are placed: a
// function called by name that takes register arguments (IsRegisterCall,
// not a variadic call) gets its first three of 32 bits or less in A, B and
// Y (CIRBackend::AssignRegisters) and is called at its register label
// (AppendRegisterSuffix); any other gets them all on the stack.  The pushed
// ones go last first (PushArguments), the callee is found (CalleeOperand),
// the registers and a thiscall's this in X are loaded
// (LoadRegisterArguments), then jsr and the caller removes what it pushed
// and keeps the result (CallerCleanUp).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Call(CIRBlock* pcBlock, size uiIndex)
{
	CArrayIRRegisterArguments	asArguments;
	SIRRegisterArgument*		psArgument;
	SIRInstruction*				psCall;
	size						uiFirst;
	size						ui;
	int32						iBytes;
	int							iRegisters;
	bool						bThisCall;
	CChars						szCallee;

	psCall = pcBlock->masInstructions.Get(uiIndex);
	uiFirst = CallArguments(pcBlock, uiIndex);
	bThisCall = (psCall->uiFlags & IRF_ThisCall) && (uiFirst < uiIndex);

	asArguments.Init();
	for (ui = uiFirst + (bThisCall ? 1 : 0); ui < uiIndex; ui++)
	{
		psArgument = asArguments.Add();
		psArgument->iUnits = ArgumentUnits(pcBlock->masInstructions.Get(ui)->sType);
		psArgument->iRegister = -1;
	}
	iRegisters = (IsRegisterCall(psCall->asArguments[0]) && !(psCall->uiFlags & IRF_Variadic)) ? AssignRegisters(&asArguments, IR65_ARGUMENT_REGISTERS, 1) : 0;

	iBytes = PushArguments(pcBlock, uiFirst + (bThisCall ? 1 : 0), uiIndex, &asArguments);
	szCallee.Init();
	CalleeOperand(psCall->asArguments[0], bThisCall, &szCallee);
	if (iRegisters > 0)
	{
		AppendRegisterSuffix(&szCallee, &asArguments, IR65_REGISTER_LETTERS);
	}
	if (bThisCall || (iRegisters > 0))
	{
		LoadRegisterArguments(pcBlock, uiFirst, bThisCall, &asArguments);
	}
	Line("jsr ")->Append(szCallee)->AppendNewLine();
	szCallee.Kill();
	asArguments.Kill();
	CallerCleanUp(iBytes, psCall);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The first of the arg instructions just before the call uiCall (uiCall if
// it has none).
//
//////////////////////////////////////////////////////////////////////////
size CIR65Backend::CallArguments(CIRBlock* pcBlock, size uiCall)
{
	size	uiFirst;

	uiFirst = uiCall;
	while ((uiFirst > 0) && (pcBlock->masInstructions.Get(uiFirst - 1)->uiOpcode == IRO_Arg))
	{
		uiFirst--;
	}
	return uiFirst;
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the arg instructions uiFirst to uiEnd - 1 that aren't given a
// register (pasArguments, one for each), last first, each at its own size.
// Returns the bytes pushed.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR65Backend::PushArguments(CIRBlock* pcBlock, size uiFirst, size uiEnd, CArrayIRRegisterArguments* pasArguments)
{
	size			ui;
	int32			iBytes;
	SIRInstruction*	psArg;

	iBytes = 0;
	for (ui = uiEnd; ui > uiFirst; ui--)
	{
		if (pasArguments->Get(ui - 1 - uiFirst)->iRegister >= 0)
		{
			continue;
		}
		psArg = pcBlock->masInstructions.Get(ui - 1);
		Push(psArg->asArguments[0], psArg->sType);
		iBytes += ValueBytes(psArg->sType);
	}
	return iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// The callee is jumped to where it is: a register (X only when this doesn't
// go there), or through a pointer in a pseudo register or the frame (jsr
// [n + W]); else it is loaded into A.  Appends it to pszCallee.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::CalleeOperand(SIRReference sCallee, bool bThisCall, CChars* pszCallee)
{
	if (sCallee.IsSymbol())
	{
		pszCallee->Append(mpcModule->GetSymbolName(sCallee.Index()));
	}
	else if (InRegister(sCallee) && ((PlaceOf(sCallee)->iOffset != IR65R_X) || !bThisCall))
	{
		AppendPlace(pszCallee, PlaceOf(sCallee));
	}
	else if (sCallee.IsTemporary() && !IsInA(sCallee) && IsMemoryHome(PlaceOf(sCallee)) && ((PlaceOf(sCallee)->uiKind == IR65P_Pseudo) || !msFrame.bFrameless))
	{
		AppendPlace(pszCallee, PlaceOf(sCallee));
	}
	else
	{
		LoadValue(sCallee, "A", IR65E_Zero);
		pszCallee->Append('A');
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The registers loading sValue reads (a bit each: 1 A, 2 X, 4 Y): A when it
// is what A holds, else its home when that is X or Y (V is never an
// argument's register).
//
//////////////////////////////////////////////////////////////////////////
uint8 CIR65Backend::ReadsRegisters(SIRReference sValue)
{
	if (IsInA(sValue))
	{
		return 1;
	}
	if (InRegister(sValue))
	{
		switch (PlaceOf(sValue)->iOffset)
		{
		case IR65R_X:
			return 2;
		case IR65R_Y:
			return 4;
		default:
			return 0;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The arguments a call takes in registers: a thiscall's this (the arg
// instruction uiFirst) in X, and those pasArguments gives A, B and Y (the
// arg instructions after this, one for each).  A register is loaded once
// no argument still to be loaded reads it (one kept in Y for X before
// another is put in Y); arguments that read each other's registers (a swap
// of X and Y) are pushed and pulled.  B is nobody's home, so it is always
// free.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::LoadRegisterArguments(CIRBlock* pcBlock, size uiFirst, bool bThisCall, CArrayIRRegisterArguments* pasArguments)
{
	static const char*	aszRegisters[IR65_ARGUMENT_REGISTERS + 1] = { "A", "B", "Y", "X" };
	static const uint8	auiWrites[IR65_ARGUMENT_REGISTERS + 1] = { 1, 0, 4, 2 };
	SIRReference		asValues[IR65_ARGUMENT_REGISTERS + 1];
	bool				abPending[IR65_ARGUMENT_REGISTERS + 1];
	bool				abPull[IR65_ARGUMENT_REGISTERS + 1];
	size				ui;
	size				uiArguments;
	int					iPending;
	int					iRegister;
	int					i;
	int					j;
	uint8				uiRead;

	iPending = 0;
	for (i = 0; i <= IR65_ARGUMENT_REGISTERS; i++)
	{
		abPending[i] = false;
		abPull[i] = false;
	}
	uiArguments = uiFirst + (bThisCall ? 1 : 0);
	for (ui = 0; ui < pasArguments->NumElements(); ui++)
	{
		iRegister = pasArguments->Get(ui)->iRegister;
		if (iRegister >= 0)
		{
			asValues[iRegister] = pcBlock->masInstructions.Get(uiArguments + ui)->asArguments[0];
			abPending[iRegister] = true;
			iPending++;
		}
	}
	if (bThisCall)
	{
		asValues[IR65_ARGUMENT_REGISTERS] = pcBlock->masInstructions.Get(uiFirst)->asArguments[0];
		abPending[IR65_ARGUMENT_REGISTERS] = true;
		iPending++;
	}

	while (iPending > 0)
	{
		//One whose register no other still reads.
		for (i = 0; i <= IR65_ARGUMENT_REGISTERS; i++)
		{
			if (!abPending[i])
			{
				continue;
			}
			uiRead = 0;
			for (j = 0; j <= IR65_ARGUMENT_REGISTERS; j++)
			{
				if (abPending[j] && (j != i))
				{
					uiRead |= ReadsRegisters(asValues[j]);
				}
			}
			if (!(uiRead & auiWrites[i]))
			{
				break;
			}
		}
		if (i > IR65_ARGUMENT_REGISTERS)
		{
			//Each waits for another: one is pushed, and pulled when the rest are in.
			for (i = 0; !abPending[i]; i++)
			{
			}
			Push(asValues[i], IRType(IRTK_I32));
			abPull[i] = true;
		}
		else
		{
			LoadValue(asValues[i], aszRegisters[i], IR65E_Zero);
			if (i == 0)
			{
				msAvailable = IRNone();
			}
		}
		abPending[i] = false;
		iPending--;
	}
	for (i = 0; i <= IR65_ARGUMENT_REGISTERS; i++)
	{
		if (abPull[i])
		{
			Line("pul ")->Append(aszRegisters[i])->AppendNewLine();
			if (i == 0)
			{
				msAvailable = IRNone();
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// After a call: the iBytes of arguments are removed and the result (in A)
// goes to its home, or stays in A.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::CallerCleanUp(int32 iBytes, SIRInstruction* psCall)
{
	RemoveArguments(iBytes);
	msAvailable = IRNone();
	StoreResult(psCall->sResult);
}


//////////////////////////////////////////////////////////////////////////
//
// Removes iBytes of arguments a call was given.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::RemoveArguments(int32 iBytes)
{
	if (iBytes > 0)
	{
		Line("add S, ")->Append(iBytes)->AppendNewLine();
	}
}
