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
#include "IR816BackendInternal.h"


//The W65C816 back end's calls: arguments pushed or loaded into registers (the register rule), the call, and the
//clean up after it.


//////////////////////////////////////////////////////////////////////////
//
// After a call: the arguments are removed (pulled into Y, or S moved with A kept in Y; X is kept), then the result (A,
// and X for 32 bits) goes to its place.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::CallerCleanUp(int32 iBytes, SIRInstruction* psInstruction)
{
	bool	bResult;
	int		iWords;
	int		i;

	bResult = !psInstruction->sResult.IsNone() && !IsHiddenResult(psInstruction->sType);
	iWords = bResult ? Words(psInstruction->sType) : 0;
	if ((iBytes == 2) || (iBytes == 4))
	{
		//Pulled into Y, which keeps A and X.
		for (i = 0; i < iBytes; i += 2)
		{
			Line("ply");
		}
	}
	else if (iBytes != 0)
	{
		if (bResult)
		{
			Line("tay");
		}
		Line("tsc");
		Line("clc");
		Line("adc ", iBytes);
		Line("tcs");
		if (bResult)
		{
			Line("tya");
		}
	}
	if (bResult)
	{
		if (iWords == 1)
		{
			Mask(psInstruction->sType);
			Store(psInstruction->sResult, 0);
		}
		else if (IR816OnDirectPage(OffsetOf(psInstruction->sResult) + 2))
		{
			Store(psInstruction->sResult, 0);
			Frame("stx", OffsetOf(psInstruction->sResult) + 2);
		}
		else
		{
			//Past the direct page Frame goes through X: the high word waits in the scratch word.
			Frame("stx", IR816_SCRATCH_RESULT + 2);
			Store(psInstruction->sResult, 0);
			Frame("lda", IR816_SCRATCH_RESULT + 2);
			Store(psInstruction->sResult, 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the arguments last first (and the hidden result pointer), calls and removes them.  A call through a pointer
// pushes the return address and the target - 1, and rtl's to it (rtl adds 1 to the low 16 bits only).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Call(CIRBlock* pcBlock, SIRInstruction* psInstruction)
{
	int32					iBytes;
	size					ui;
	SIRInstruction*			psArgument;
	SIRReference			sCallee;
	int32					iReturn;
	int32					iCallee;
	int						iRegisters;
	CChars					sz;
	CArrayIR816Arguments	asArguments;
	SIR816Argument*			psPlaced;

	//The arguments as passed: the hidden result pointer first, each pushed unless a register function takes it in
	//registers.
	sCallee = psInstruction->asArguments[0];
	asArguments.Init(CIR816Backend::GetScratch());
	if (!psInstruction->sResult.IsNone() && IsHiddenResult(psInstruction->sType))
	{
		psPlaced = asArguments.Add();
		psPlaced->iWords = 2;
		psPlaced->iRegister = -1;
	}
	for (ui = 0; ui < mauiPendingArguments.NumElements(); ui++)
	{
		psArgument = pcBlock->masInstructions.Get(*mauiPendingArguments.Get(ui));
		psPlaced = asArguments.Add();
		psPlaced->iWords = Words(psArgument->sType);
		psPlaced->iRegister = -1;
	}
	iRegisters = IsRegisterCall(sCallee) ? IR816AssignRegisters(&asArguments) : 0;

	iBytes = PushArguments(pcBlock, psInstruction, &asArguments);
	if (iRegisters > 0)
	{
		LoadRegisterArguments(pcBlock, psInstruction, &asArguments);
	}
	mauiPendingArguments.FakeSetUsedElements(0);

	sz.Init(CIR816Backend::GetScratch());
	if (sCallee.IsSymbol())
	{
		sz.Append("jsl ");
		sz.Append(mpcModule->GetSymbolName(sCallee.Index()));
		if (iRegisters > 0)
		{
			IR816AppendRegisterSuffix(&sz, &asArguments);
		}
		Line(sz.Text());
	}
	else
	{
		iReturn = NewLabel();
		iCallee = OffsetOf(sCallee);
		Line("phk");
		sz.Append("per L");
		sz.Append(iReturn);
		sz.Append(" - 1");
		Line(sz.Text());
		Line("sep m");
		Frame("lda", iCallee + 2);
		Line("pha");
		Line("rep m");
		Frame("lda", iCallee);
		Line("dec A");
		Line("pha");
		Line("rtl");
		Label(iReturn);
	}
	sz.Kill();
	asArguments.Kill();
	CallerCleanUp(iBytes, psInstruction);
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the call's arguments that aren't in registers (pasArguments, as
// Call places them), last first, then the hidden result pointer if it
// isn't.  Returns the bytes pushed.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR816Backend::PushArguments(CIRBlock* pcBlock, SIRInstruction* psInstruction, CArrayIR816Arguments* pasArguments)
{
	int32			iBytes;
	size			ui;
	size			uiFirst;
	SIRInstruction*	psArgument;

	iBytes = 0;
	uiFirst = pasArguments->NumElements() - mauiPendingArguments.NumElements();
	for (ui = mauiPendingArguments.NumElements(); ui > 0; ui--)
	{
		if (pasArguments->Get(uiFirst + ui - 1)->iRegister >= 0)
		{
			continue;
		}
		psArgument = pcBlock->masInstructions.Get(*mauiPendingArguments.Get(ui - 1));
		PushValue(psArgument->asArguments[0], psArgument->sType);
		iBytes += ValueBytes(psArgument->sType);
	}
	if ((uiFirst == 1) && (pasArguments->Get(0)->iRegister < 0))
	{
		PushFrameAddress(OffsetOf(psInstruction->sResult));
		iBytes += 4;
	}
	return iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the call's register argument word for iRegister comes from: its
// value and word, or the hidden result pointer (*piHiddenResult its frame
// offset, else -1).
//
//////////////////////////////////////////////////////////////////////////
static void IR816RegisterSource(CIRBlock* pcBlock, SIRInstruction* psInstruction, CArrayTemplate<size>* pauiPending, CArrayIR816Arguments* pasArguments, int iRegister, int32 iHiddenOffset, SIRReference* psValue, int* piWord, int32* piHiddenResult)
{
	size			ui;
	size			uiFirst;
	SIR816Argument*	psArgument;

	uiFirst = pasArguments->NumElements() - pauiPending->NumElements();
	*psValue = IRNone();
	*piWord = 0;
	*piHiddenResult = -1;
	for (ui = 0; ui < pasArguments->NumElements(); ui++)
	{
		psArgument = pasArguments->Get(ui);
		if ((psArgument->iRegister >= 0) && (iRegister >= psArgument->iRegister) && (iRegister < psArgument->iRegister + psArgument->iWords))
		{
			*piWord = iRegister - psArgument->iRegister;
			if (ui < uiFirst)
			{
				*piHiddenResult = iHiddenOffset;
			}
			else
			{
				*psValue = pcBlock->masInstructions.Get(*pauiPending->Get(ui - uiFirst))->asArguments[0];
			}
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Puts the call's register arguments in A, X and Y (after PushArguments has
// pushed the others): LoadRegisters.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::LoadRegisterArguments(CIRBlock* pcBlock, SIRInstruction* psInstruction, CArrayIR816Arguments* pasArguments)
{
	SIRReference	asValues[IR816_ARGUMENT_REGISTERS];
	int				aiWords[IR816_ARGUMENT_REGISTERS];
	int32			aiHidden[IR816_ARGUMENT_REGISTERS];
	int32			iHiddenOffset;
	int				iRegisters;
	int				i;

	iHiddenOffset = psInstruction->sResult.IsNone() ? -1 : OffsetOf(psInstruction->sResult);
	iRegisters = 0;
	for (i = 0; i < IR816_ARGUMENT_REGISTERS; i++)
	{
		IR816RegisterSource(pcBlock, psInstruction, &mauiPendingArguments, pasArguments, i, iHiddenOffset, &asValues[i], &aiWords[i], &aiHidden[i]);
		if (asValues[i].IsNone() && (aiHidden[i] < 0))
		{
			break;
		}
		iRegisters++;
	}
	LoadRegisters(asValues, aiWords, aiHidden, iRegisters);
}


//////////////////////////////////////////////////////////////////////////
//
// Loads the first iRegisters of A, X and Y with their words: word
// aiWords[i] of asValues[i], or of the hidden result pointer when
// aiHidden[i] isn't -1 (its frame offset).  Straight from where they are
// when each can be (LoadsRegisterDirectly): Y and X first and A last,
// unless A's value is kept in X (then A first).  Otherwise they are pushed
// and pulled.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::LoadRegisters(SIRReference* asValues, int* aiWords, int32* aiHidden, int iRegisters)
{
	int		i;
	bool	bDirect;
	bool	bAFirst;

	bDirect = true;
	for (i = 0; i < iRegisters; i++)
	{
		bDirect &= (aiHidden[i] >= 0) || LoadsRegisterDirectly(asValues[i], aiWords[i]);
	}

	if (bDirect)
	{
		bAFirst = (aiHidden[0] < 0) && (aiWords[0] == 0) && InX(asValues[0]);
		if (bAFirst)
		{
			LoadRegisterWord(0, asValues[0], aiWords[0], aiHidden[0]);
		}
		for (i = iRegisters - 1; i > 0; i--)
		{
			LoadRegisterWord(i, asValues[i], aiWords[i], aiHidden[i]);
		}
		if (!bAFirst)
		{
			LoadRegisterWord(0, asValues[0], aiWords[0], aiHidden[0]);
		}
		return;
	}

	//Pushed Y's first, so A's is pulled first.
	for (i = iRegisters - 1; i >= 0; i--)
	{
		if ((aiHidden[i] >= 0) && (aiWords[i] == 0))
		{
			Line("tdc");
			Line("clc");
			Line("adc ", aiHidden[i]);
			Line("pha");
		}
		else if (aiHidden[i] >= 0)
		{
			Line("pea 0");
		}
		else
		{
			PushWord(asValues[i], aiWords[i]);
		}
	}
	Line("pla");
	if (iRegisters > 1)
	{
		Line("plx");
	}
	if (iRegisters > 2)
	{
		Line("ply");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a register argument's word can be loaded straight into A, X or Y
// (LoadRegisterWord) without A or X: a constant, a symbol's address, a
// value kept in X, or a word in the direct page.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::LoadsRegisterDirectly(SIRReference sValue, int iWord)
{
	if (!sValue.IsTemporary())
	{
		return true;
	}
	if ((iWord == 0) && InX(sValue))
	{
		return true;
	}
	return (iWord >= Words(mpcFunction->GetTemporaryType(sValue))) || IR816OnDirectPage(OffsetOf(sValue) + 2 * iWord);
}


//////////////////////////////////////////////////////////////////////////
//
// Loads a register argument's word into A (iRegister 0), X (1) or Y (2):
// the word of the hidden result pointer when iHiddenResult isn't -1 (its
// frame offset; the high word is 0), else of sValue.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::LoadRegisterWord(int iRegister, SIRReference sValue, int iWord, int32 iHiddenResult)
{
	static const char*	aszLoads[IR816_ARGUMENT_REGISTERS] = { "lda", "ldx", "ldy" };
	static const char*	aszZeros[IR816_ARGUMENT_REGISTERS] = { "lda 0", "ldx 0", "ldy 0" };

	if ((iHiddenResult >= 0) && (iWord == 0))
	{
		//The hidden result pointer is the first argument: its low word is A's.
		Line("tdc");
		Line("clc");
		Line("adc ", iHiddenResult);
	}
	else if (iHiddenResult >= 0)
	{
		Line(aszZeros[iRegister]);
	}
	else
	{
		Operand(aszLoads[iRegister], sValue, iWord);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a call of sCallee passes its arguments in registers: a symbol the
// front end marked (CIRModule::SetRegisterCall).
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::IsRegisterCall(SIRReference sCallee)
{
	return gbIR816RegisterArguments && sCallee.IsSymbol() && mpcModule->IsRegisterCall(sCallee.Index());
}
