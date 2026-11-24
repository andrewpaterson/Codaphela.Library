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
//
//////////////////////////////////////////////////////////////////////////
void CIR65Calls::Init(CIR65Backend* pcBackend)
{
	mpcBackend = pcBackend;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Calls::Kill(void)
{
	mpcBackend = NULL;
}


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
bool CIR65Calls::Call(CIRBlock* pcBlock, size uiIndex)
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
	SIRInstruction*				psThrown;
	SIR65Stub*					psStub;

	psCall = pcBlock->masInstructions.Get(uiIndex);
	uiFirst = CallArguments(pcBlock, uiIndex);
	bThisCall = (psCall->uiFlags & IRF_ThisCall) && (uiFirst < uiIndex);

	asArguments.Init();
	for (ui = uiFirst + (bThisCall ? 1 : 0); ui < uiIndex; ui++)
	{
		psArgument = asArguments.Add();
		psArgument->iUnits = mpcBackend->mcPlaces.ArgumentUnits(pcBlock->masInstructions.Get(ui)->sType);
		psArgument->iRegister = -1;
	}
	iRegisters = (mpcBackend->IsRegisterCall(psCall->asArguments[0]) && !(psCall->uiFlags & IRF_Variadic)) ? mpcBackend->AssignRegisters(&asArguments, IR65_ARGUMENT_REGISTERS, 1) : 0;

	iBytes = PushArguments(pcBlock, uiFirst + (bThisCall ? 1 : 0), uiIndex, &asArguments);
	szCallee.Init();
	CalleeOperand(psCall->asArguments[0], bThisCall, &szCallee);
	if (iRegisters > 0)
	{
		mpcBackend->AppendRegisterSuffix(&szCallee, &asArguments, IR65_REGISTER_LETTERS);
	}
	if (bThisCall || (iRegisters > 0))
	{
		LoadRegisterArguments(pcBlock, uiFirst, bThisCall, &asArguments);
	}
	mpcBackend->Line("jsr ")->Append(szCallee)->AppendNewLine();
	szCallee.Kill();
	asArguments.Kill();

	//A thrown comes next (it is the block's last instruction, and its brif's: IsolateThrowns).  The callee returned with
	//the carry set if it threw: the way to the brif's then block, with the arguments still to remove (which would clear
	//the carry), is written after the function's ret.
	psThrown = pcBlock->masInstructions.SafeGet(uiIndex + 1);
	if (psThrown && (psThrown->uiOpcode == IRO_Thrown))
	{
		psStub = mpcBackend->masStubs.Add();
		psStub->iLabel = mpcBackend->NewLabel();
		psStub->pcFrom = pcBlock;
		psStub->pcTo = pcBlock->msJump.pcThen;
		psStub->iRemove = iBytes;
		psStub->bTested = true;
		psStub->bWritten = false;
		mpcBackend->Branch("bras c, ", psStub->iLabel);
	}
	CallerCleanUp(iBytes, psCall);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The first of the arg instructions just before the call uiCall (uiCall if
// it has none).
//
//////////////////////////////////////////////////////////////////////////
size CIR65Calls::CallArguments(CIRBlock* pcBlock, size uiCall)
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
int32 CIR65Calls::PushArguments(CIRBlock* pcBlock, size uiFirst, size uiEnd, CArrayIRRegisterArguments* pasArguments)
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
		mpcBackend->mcSelector.Push(psArg->asArguments[0], psArg->sType);
		iBytes += mpcBackend->ValueBytes(psArg->sType);
	}
	return iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// The callee is jumped to where it is: a register (X only when this doesn't
// go there; not W, which is no address register), or through a pointer in a pseudo
// register or the frame (jsr [n + W]); else it is loaded into A.  Appends
// it to pszCallee.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Calls::CalleeOperand(SIRReference sCallee, bool bThisCall, CChars* pszCallee)
{
	if (sCallee.IsSymbol())
	{
		pszCallee->Append(mpcBackend->mpcModule->GetSymbolName(sCallee.Index()));
	}
	else if (mpcBackend->mcPlaces.InRegister(sCallee) && (mpcBackend->mcPlaces.PlaceOf(sCallee)->iOffset != IR65R_W) && (mpcBackend->mcPlaces.PlaceOf(sCallee)->iOffset != IR65R_B) && ((mpcBackend->mcPlaces.PlaceOf(sCallee)->iOffset != IR65R_X) || !bThisCall))
	{
		mpcBackend->mcSelector.AppendPlace(pszCallee, mpcBackend->mcPlaces.PlaceOf(sCallee));
	}
	else if (sCallee.IsTemporary() && !mpcBackend->mcPlaces.IsInA(sCallee) && mpcBackend->mcPlaces.IsMemoryHome(mpcBackend->mcPlaces.PlaceOf(sCallee)) && ((mpcBackend->mcPlaces.PlaceOf(sCallee)->uiKind == IR65P_Pseudo) || !mpcBackend->msFrame.bFrameless))
	{
		mpcBackend->mcSelector.AppendPlace(pszCallee, mpcBackend->mcPlaces.PlaceOf(sCallee));
	}
	else
	{
		mpcBackend->mcSelector.LoadValue(sCallee, "A", IR65E_Zero);
		pszCallee->Append('A');
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The registers loading sValue reads (a bit each: 1 A, 2 X, 4 Y, 8 B): A
// when it is what A holds, else its home when that is X, Y or B (V is never
// an argument's register).
//
//////////////////////////////////////////////////////////////////////////
uint8 CIR65Calls::ReadsRegisters(SIRReference sValue)
{
	if (mpcBackend->mcPlaces.IsInA(sValue))
	{
		return 1;
	}
	if (mpcBackend->mcPlaces.InRegister(sValue))
	{
		switch (mpcBackend->mcPlaces.PlaceOf(sValue)->iOffset)
		{
		case IR65R_X:
			return 2;
		case IR65R_Y:
			return 4;
		case IR65R_B:
			return 8;
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
// of X and Y) are pushed and pulled.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Calls::LoadRegisterArguments(CIRBlock* pcBlock, size uiFirst, bool bThisCall, CArrayIRRegisterArguments* pasArguments)
{
	static const char*	aszRegisters[IR65_ARGUMENT_REGISTERS + 1] = { "A", "B", "Y", "X" };
	static const uint8	auiWrites[IR65_ARGUMENT_REGISTERS + 1] = { 1, 8, 4, 2 };
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
			mpcBackend->mcSelector.Push(asValues[i], IRType(IRTK_I32));
			abPull[i] = true;
		}
		else
		{
			mpcBackend->mcSelector.LoadValue(asValues[i], aszRegisters[i], IR65E_Zero);
			if (i == 0)
			{
				mpcBackend->msAvailable = IRNone();
			}
		}
		abPending[i] = false;
		iPending--;
	}
	for (i = 0; i <= IR65_ARGUMENT_REGISTERS; i++)
	{
		if (abPull[i])
		{
			mpcBackend->Line("pul ")->Append(aszRegisters[i])->AppendNewLine();
			if (i == 0)
			{
				mpcBackend->msAvailable = IRNone();
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
void CIR65Calls::CallerCleanUp(int32 iBytes, SIRInstruction* psCall)
{
	RemoveArguments(iBytes);
	mpcBackend->msAvailable = IRNone();
	mpcBackend->mcSelector.StoreResult(psCall->sResult);
}


//////////////////////////////////////////////////////////////////////////
//
// Removes iBytes of arguments a call was given.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Calls::RemoveArguments(int32 iBytes)
{
	if (iBytes > 0)
	{
		mpcBackend->Line("add S, ")->Append(iBytes)->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Calls a soft float routine (_ and its name) and removes iBytes of
// arguments.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Calls::CallRuntime(const char* szName, int iBytes)
{
	CChars	szLabel;

	szLabel.Init("_");
	szLabel.Append(szName);
	mpcBackend->UseRuntime(szLabel.Text());
	mpcBackend->Line("jsr ")->Append(szLabel)->AppendNewLine();
	szLabel.Kill();
	RemoveArguments(iBytes);
}


//////////////////////////////////////////////////////////////////////////
//
// A register function's plain label, for callers that push every argument
// (function pointers, vtables, thunks, assembly): on entry the arguments
// are at [5 + S] on, as they were pushed (a thiscall's this is in X either
// way).  Those the body wants in A, B and Y are loaded from there and it
// goes straight on to the body (szBody), which follows.  When it also takes
// arguments on the stack they must be just above its return address: they
// are pushed again (through A, before it is loaded), the body is called,
// they are removed and it returns.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Calls::RegisterStub(const char* szBody)
{
	static const char*	aszRegisters[IR65_ARGUMENT_REGISTERS] = { "A", "B", "Y" };
	SIR65Frame			sFrame;
	SIR65Place			sArrival;
	SIRType				sType;
	size				ui;
	size				uiFirst;
	int32				iOffset;
	int32				iPushed;
	int32				iThrown;
	int					iBytes;
	int					i;

	//The arguments as they were all pushed, from S.
	sFrame = mpcBackend->msFrame;
	mpcBackend->msFrame.cRegister = 'S';
	mpcBackend->msFrame.iArguments = 1 + 4;
	uiFirst = (mpcBackend->mpcFunction->muiAttributes & IRA_ThisCall) ? 1 : 0;
	sArrival.uiKind = IR65P_Argument;
	sArrival.bInA = false;

	//The pushed ones again, last first.
	iPushed = 0;
	for (ui = mpcBackend->mpcFunction->masParameters.NumElements(); ui > uiFirst; ui--)
	{
		if (mpcBackend->mcPlaces.ParameterRegister(ui - 1) >= 0)
		{
			continue;
		}
		sType = mpcBackend->mpcFunction->GetTemporaryType(*mpcBackend->mpcFunction->masParameters.Get(ui - 1));
		iBytes = mpcBackend->ValueBytes(sType);
		iOffset = 0;
		for (i = (int)uiFirst; i < (int)(ui - 1); i++)
		{
			iOffset += mpcBackend->ValueBytes(mpcBackend->mpcFunction->GetTemporaryType(*mpcBackend->mpcFunction->masParameters.Get(i)));
		}
		sArrival.iOffset = iOffset + iPushed;
		sArrival.iBytes = iBytes;
		mpcBackend->mcSelector.LoadPlace(&sArrival, sType, "A", IR65E_Zero);
		if (iBytes != 4)
		{
			mpcBackend->Line("wth A, ")->Append(8 * iBytes)->AppendNewLine();
		}
		mpcBackend->Line("psh A")->AppendNewLine();
		if (iBytes != 4)
		{
			mpcBackend->Line("wth A, 32")->AppendNewLine();
		}
		iPushed += iBytes;
	}

	//The registers, A last.
	for (i = IR65_ARGUMENT_REGISTERS - 1; i >= 0; i--)
	{
		iOffset = 0;
		for (ui = uiFirst; ui < mpcBackend->mpcFunction->masParameters.NumElements(); ui++)
		{
			sType = mpcBackend->mpcFunction->GetTemporaryType(*mpcBackend->mpcFunction->masParameters.Get(ui));
			if (mpcBackend->mcPlaces.ParameterRegister(ui) == i)
			{
				sArrival.iOffset = iOffset + iPushed;
				sArrival.iBytes = mpcBackend->ValueBytes(sType);
				mpcBackend->mcSelector.LoadPlace(&sArrival, sType, aszRegisters[i], IR65E_Zero);
			}
			iOffset += mpcBackend->ValueBytes(sType);
		}
	}
	mpcBackend->msFrame = sFrame;

	if (iPushed > 0)
	{
		mpcBackend->Line("jsr ")->Append(szBody)->AppendNewLine();
		iThrown = -1;
		if (mpcBackend->mpcOptions->mbExceptions && !mpcBackend->mpcModule->IsNoThrow(mpcBackend->mpcFunction->muiSymbol))
		{
			//Removing the pushed bytes clears the carry: a body that threw leaves by an exit of its own.
			iThrown = mpcBackend->NewLabel();
			mpcBackend->Branch("bras c, ", iThrown);
		}
		mpcBackend->Line("add S, ")->Append(iPushed)->AppendNewLine();
		mpcBackend->Line("ret")->AppendNewLine();
		if (iThrown >= 0)
		{
			mpcBackend->Label(iThrown);
			mpcBackend->Line("add S, ")->Append(iPushed)->AppendNewLine();
			mpcBackend->Line("cmp S, S")->AppendNewLine();
			mpcBackend->Line("ret")->AppendNewLine();
		}
	}
}
