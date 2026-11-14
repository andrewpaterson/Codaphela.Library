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
#include "IRW65C816BackendInternal.h"


//The W65C816 back end's calls: arguments pushed or loaded into registers (the register rule), the call, and the
//clean up after it.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Calls::Init(CIRW65C816Backend* pcBackend)
{
	mpcBackend = pcBackend;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Calls::Kill(void)
{
	mpcBackend = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// After a call: the arguments are removed (pulled into Y, or S moved with A kept in Y; X is kept), then the result (A,
// and X for 32 bits) goes to its place.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Calls::CallerCleanUp(int32 iBytes, SIRInstruction* psInstruction)
{
	bool	bResult;
	int		iWords;
	int		i;

	bResult = !psInstruction->sResult.IsNone() && !mpcBackend->IsHiddenResult(psInstruction->sType);
	iWords = bResult ? mpcBackend->Words(psInstruction->sType) : 0;
	if ((iBytes == 2) || (iBytes == 4))
	{
		//Pulled into Y, which keeps A and X.
		for (i = 0; i < iBytes; i += 2)
		{
			mpcBackend->Line("ply");
		}
	}
	else if (iBytes != 0)
	{
		if (bResult)
		{
			mpcBackend->Line("tay");
		}
		mpcBackend->Line("tsc");
		mpcBackend->Line("clc");
		mpcBackend->Line("adc ", iBytes);
		mpcBackend->Line("tcs");
		if (bResult)
		{
			mpcBackend->Line("tya");
		}
	}
	if (bResult)
	{
		if (iWords == 1)
		{
			mpcBackend->mcSelector.Mask(psInstruction->sType);
			mpcBackend->mcSelector.Store(psInstruction->sResult, 0);
		}
		else if (IR816OnDirectPage(mpcBackend->mcPlaces.OffsetOf(psInstruction->sResult) + 2))
		{
			mpcBackend->mcSelector.Store(psInstruction->sResult, 0);
			mpcBackend->mcSelector.Frame("stx", mpcBackend->mcPlaces.OffsetOf(psInstruction->sResult) + 2);
		}
		else
		{
			//Past the direct page Frame goes through X: the high word waits in the scratch word.
			mpcBackend->mcSelector.Frame("stx", IR816_SCRATCH_RESULT + 2);
			mpcBackend->mcSelector.Store(psInstruction->sResult, 0);
			mpcBackend->mcSelector.Frame("lda", IR816_SCRATCH_RESULT + 2);
			mpcBackend->mcSelector.Store(psInstruction->sResult, 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the arguments last first (and the hidden result pointer), calls and removes them.  A call through a pointer
// pushes the return address and the target - 1, and rtl's to it (rtl adds 1 to the low 16 bits only).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Calls::Call(CIRBlock* pcBlock, SIRInstruction* psInstruction)
{
	int32					iBytes;
	size					ui;
	SIRInstruction*			psArgument;
	SIRReference			sCallee;
	int32					iReturn;
	int32					iCallee;
	int						iRegisters;
	CChars					sz;
	CArrayIRRegisterArguments	asArguments;
	SIRRegisterArgument*			psPlaced;

	//The arguments as passed: the hidden result pointer first, each pushed unless a register function takes it in
	//registers.
	sCallee = psInstruction->asArguments[0];
	asArguments.Init(CIRW65C816Backend::GetScratch());
	if (!psInstruction->sResult.IsNone() && mpcBackend->IsHiddenResult(psInstruction->sType))
	{
		psPlaced = asArguments.Add();
		psPlaced->iUnits = 2;
		psPlaced->iRegister = -1;
	}
	for (ui = 0; ui < mpcBackend->mauiPendingArguments.NumElements(); ui++)
	{
		psArgument = pcBlock->masInstructions.Get(*mpcBackend->mauiPendingArguments.Get(ui));
		psPlaced = asArguments.Add();
		psPlaced->iUnits = mpcBackend->Words(psArgument->sType);
		psPlaced->iRegister = -1;
	}
	iRegisters = mpcBackend->IsRegisterCall(sCallee) ? mpcBackend->AssignRegisters(&asArguments, IR816_ARGUMENT_REGISTERS, IR816_ARGUMENT_REGISTERS) : 0;

	iBytes = PushArguments(pcBlock, psInstruction, &asArguments);
	if (iRegisters > 0)
	{
		LoadRegisterArguments(pcBlock, psInstruction, &asArguments);
	}
	mpcBackend->mauiPendingArguments.FakeSetUsedElements(0);

	sz.Init(CIRW65C816Backend::GetScratch());
	if (sCallee.IsSymbol())
	{
		sz.Append("jsl ");
		sz.Append(mpcBackend->mpcModule->GetSymbolName(sCallee.Index()));
		if (iRegisters > 0)
		{
			mpcBackend->AppendRegisterSuffix(&sz, &asArguments, IR816_REGISTER_LETTERS);
		}
		mpcBackend->Line(sz.Text());
	}
	else
	{
		iReturn = mpcBackend->NewLabel();
		iCallee = mpcBackend->mcPlaces.OffsetOf(sCallee);
		mpcBackend->Line("phk");
		sz.Append("per L");
		sz.Append(iReturn);
		sz.Append(" - 1");
		mpcBackend->Line(sz.Text());
		if (IR816PointerOnDirectPage(iCallee))
		{
			//The target's bank and high byte pushed as one word and a byte's room below them (phb: any byte), then the
			//target - 1 written over that byte and the high byte (which the - 1 may have borrowed from): no change of
			//A's width for the bank's byte.
			mpcBackend->mcSelector.Frame("pei", iCallee + 1);
			mpcBackend->Line("phb");
			mpcBackend->mcSelector.Frame("lda", iCallee);
			mpcBackend->Line("dec A");
			mpcBackend->Line("sta [S + 1]");
		}
		else
		{
			mpcBackend->Line("sep m");
			mpcBackend->mcSelector.Frame("lda", iCallee + 2);
			mpcBackend->Line("pha");
			mpcBackend->Line("rep m");
			mpcBackend->mcSelector.Frame("lda", iCallee);
			mpcBackend->Line("dec A");
			mpcBackend->Line("pha");
		}
		mpcBackend->Line("rtl");
		mpcBackend->Label(iReturn);
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
int32 CIRW65C816Calls::PushArguments(CIRBlock* pcBlock, SIRInstruction* psInstruction, CArrayIRRegisterArguments* pasArguments)
{
	int32			iBytes;
	size			ui;
	size			uiFirst;
	SIRInstruction*	psArgument;

	iBytes = 0;
	uiFirst = pasArguments->NumElements() - mpcBackend->mauiPendingArguments.NumElements();
	for (ui = mpcBackend->mauiPendingArguments.NumElements(); ui > 0; ui--)
	{
		if (pasArguments->Get(uiFirst + ui - 1)->iRegister >= 0)
		{
			continue;
		}
		psArgument = pcBlock->masInstructions.Get(*mpcBackend->mauiPendingArguments.Get(ui - 1));
		mpcBackend->mcSelector.PushValue(psArgument->asArguments[0], psArgument->sType);
		iBytes += mpcBackend->ValueBytes(psArgument->sType);
	}
	if ((uiFirst == 1) && (pasArguments->Get(0)->iRegister < 0))
	{
		mpcBackend->mcSelector.PushFrameAddress(mpcBackend->mcPlaces.OffsetOf(psInstruction->sResult));
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
static void IR816RegisterSource(CIRBlock* pcBlock, SIRInstruction* psInstruction, CArrayTemplate<size>* pauiPending, CArrayIRRegisterArguments* pasArguments, int iRegister, int32 iHiddenOffset, SIRReference* psValue, int* piWord, int32* piHiddenResult)
{
	size			ui;
	size			uiFirst;
	SIRRegisterArgument*	psArgument;

	uiFirst = pasArguments->NumElements() - pauiPending->NumElements();
	*psValue = IRNone();
	*piWord = 0;
	*piHiddenResult = -1;
	for (ui = 0; ui < pasArguments->NumElements(); ui++)
	{
		psArgument = pasArguments->Get(ui);
		if ((psArgument->iRegister >= 0) && (iRegister >= psArgument->iRegister) && (iRegister < psArgument->iRegister + psArgument->iUnits))
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
void CIRW65C816Calls::LoadRegisterArguments(CIRBlock* pcBlock, SIRInstruction* psInstruction, CArrayIRRegisterArguments* pasArguments)
{
	SIRReference	asValues[IR816_ARGUMENT_REGISTERS];
	int				aiWords[IR816_ARGUMENT_REGISTERS];
	int32			aiHidden[IR816_ARGUMENT_REGISTERS];
	int32			iHiddenOffset;
	int				iRegisters;
	int				i;

	iHiddenOffset = psInstruction->sResult.IsNone() ? -1 : mpcBackend->mcPlaces.OffsetOf(psInstruction->sResult);
	iRegisters = 0;
	for (i = 0; i < IR816_ARGUMENT_REGISTERS; i++)
	{
		IR816RegisterSource(pcBlock, psInstruction, &mpcBackend->mauiPendingArguments, pasArguments, i, iHiddenOffset, &asValues[i], &aiWords[i], &aiHidden[i]);
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
void CIRW65C816Calls::LoadRegisters(SIRReference* asValues, int* aiWords, int32* aiHidden, int iRegisters)
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
		bAFirst = (aiHidden[0] < 0) && (aiWords[0] == 0) && mpcBackend->mcPlaces.InX(asValues[0]);
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
			mpcBackend->Line("tdc");
			mpcBackend->Line("clc");
			mpcBackend->Line("adc ", aiHidden[i]);
			mpcBackend->Line("pha");
		}
		else if (aiHidden[i] >= 0)
		{
			mpcBackend->Line("pea 0");
		}
		else
		{
			mpcBackend->mcSelector.PushWord(asValues[i], aiWords[i]);
		}
	}
	mpcBackend->Line("pla");
	if (iRegisters > 1)
	{
		mpcBackend->Line("plx");
	}
	if (iRegisters > 2)
	{
		mpcBackend->Line("ply");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a register argument's word can be loaded straight into A, X or Y
// (LoadRegisterWord) without A or X: a constant, a symbol's address, a
// value kept in X, or a word in the direct page.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Calls::LoadsRegisterDirectly(SIRReference sValue, int iWord)
{
	if (!sValue.IsTemporary())
	{
		return true;
	}
	if ((iWord == 0) && mpcBackend->mcPlaces.InX(sValue))
	{
		return true;
	}
	return (iWord >= mpcBackend->Words(mpcBackend->mpcFunction->GetTemporaryType(sValue))) || IR816OnDirectPage(mpcBackend->mcPlaces.OffsetOf(sValue) + 2 * iWord);
}


//////////////////////////////////////////////////////////////////////////
//
// Loads a register argument's word into A (iRegister 0), X (1) or Y (2):
// the word of the hidden result pointer when iHiddenResult isn't -1 (its
// frame offset; the high word is 0), else of sValue.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Calls::LoadRegisterWord(int iRegister, SIRReference sValue, int iWord, int32 iHiddenResult)
{
	static const char*	aszLoads[IR816_ARGUMENT_REGISTERS] = { "lda", "ldx", "ldy" };
	static const char*	aszZeros[IR816_ARGUMENT_REGISTERS] = { "lda 0", "ldx 0", "ldy 0" };

	if ((iHiddenResult >= 0) && (iWord == 0))
	{
		//The hidden result pointer is the first argument: its low word is A's.
		mpcBackend->Line("tdc");
		mpcBackend->Line("clc");
		mpcBackend->Line("adc ", iHiddenResult);
	}
	else if (iHiddenResult >= 0)
	{
		mpcBackend->Line(aszZeros[iRegister]);
	}
	else
	{
		mpcBackend->mcSelector.Operand(aszLoads[iRegister], sValue, iWord);
	}
}


