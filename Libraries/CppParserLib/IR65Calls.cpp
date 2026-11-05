/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "IR65BackendInternal.h"


//The 65C64 back end's calls: the arguments pushed, the callee's operand, a thiscall's this in X, and the clean up
//after it.


//////////////////////////////////////////////////////////////////////////
//
// The arg instructions before the call (CallArguments) are pushed last
// first (PushArguments), the callee found (CalleeOperand), a thiscall's
// this put in X (LoadRegisterArguments), then jsr and the caller removes
// the arguments and keeps the result (CallerCleanUp).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Call(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psCall;
	size			uiFirst;
	int32			iBytes;
	bool			bThisCall;
	CChars			szCallee;

	psCall = pcBlock->masInstructions.Get(uiIndex);
	uiFirst = CallArguments(pcBlock, uiIndex);
	bThisCall = (psCall->uiFlags & IRF_ThisCall) && (uiFirst < uiIndex);

	iBytes = PushArguments(pcBlock, uiFirst + (bThisCall ? 1 : 0), uiIndex);
	szCallee.Init();
	CalleeOperand(psCall->asArguments[0], bThisCall, &szCallee);
	if (bThisCall)
	{
		LoadRegisterArguments(pcBlock, uiFirst);
	}
	Line("jsr ")->Append(szCallee)->AppendNewLine();
	szCallee.Kill();
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
// Pushes the arg instructions uiFirst to uiEnd - 1, last first, each at its
// own size.  Returns the bytes pushed.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR65Backend::PushArguments(CIRBlock* pcBlock, size uiFirst, size uiEnd)
{
	size			ui;
	int32			iBytes;
	SIRInstruction*	psArg;

	iBytes = 0;
	for (ui = uiEnd; ui > uiFirst; ui--)
	{
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
	else if (InRegister(sCallee) && ((PlaceOf(sCallee)->iOffset == IR65R_Y) || !bThisCall))
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
// The arguments a call takes in registers: a thiscall's this (the arg
// instruction uiFirst) in X.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::LoadRegisterArguments(CIRBlock* pcBlock, size uiFirst)
{
	LoadValue(pcBlock->masInstructions.Get(uiFirst)->asArguments[0], "X", IR65E_Zero);
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
