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
	mbThrownInCarry = false;
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
// and X for 32 bits) goes to its place.  bKeepCarry: the carry is the call's thrown and the block's brif reads it, so
// up to IR816_CARRY_PULL_BYTES are pulled (moving S adds, which loses the carry).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Calls::CallerCleanUp(int32 iBytes, SIRInstruction* psInstruction, bool bKeepCarry)
{
	bool	bResult;
	int		iWords;
	int		i;

	bResult = !psInstruction->sResult.IsNone() && !mpcBackend->IsHiddenResult(psInstruction->sType);
	iWords = bResult ? mpcBackend->Words(psInstruction->sType) : 0;
	if ((iBytes == 2) || (iBytes == 4) || (bKeepCarry && (iBytes != 0)))
	{
		//Pulled into Y, which keeps A and X.
		for (i = 0; i < iBytes; i += 2)
		{
			mpcBackend->Line(IR816M_Ply);
		}
	}
	else if (iBytes != 0)
	{
		if (bResult)
		{
			mpcBackend->Line(IR816M_Tay);
		}
		mpcBackend->Line(IR816M_Tsc);
		mpcBackend->Line(IR816M_Clc);
		mpcBackend->Line(IR816M_Adc, (int64)(iBytes));
		mpcBackend->Line(IR816M_Tcs);
		if (bResult)
		{
			mpcBackend->Line(IR816M_Tya);
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
			mpcBackend->mcSelector.Frame(IR816M_Stx, mpcBackend->mcPlaces.OffsetOf(psInstruction->sResult) + 2);
		}
		else
		{
			//Past the direct page Frame goes through X: the high word waits in the scratch word.
			mpcBackend->mcSelector.Frame(IR816M_Stx, IR816_SCRATCH_RESULT + 2);
			mpcBackend->mcSelector.Store(psInstruction->sResult, 0);
			mpcBackend->mcSelector.Frame(IR816M_Lda, IR816_SCRATCH_RESULT + 2);
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
	SIRInstruction*			psThrown;
	size					uiIndex;
	int32					iThrown;

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
		sz.Append(mpcBackend->mpcModule->GetSymbolName(sCallee.Index()));
		if (iRegisters > 0)
		{
			mpcBackend->AppendRegisterSuffix(&sz, &asArguments, IR816_REGISTER_LETTERS);
		}
		mpcBackend->Line(IR816M_Jsl, sz.Text());
	}
	else
	{
		iReturn = mpcBackend->NewLabel();
		iCallee = mpcBackend->mcPlaces.OffsetOf(sCallee);
		mpcBackend->Line(IR816M_Phk);
		sz.Append("L");
		sz.Append(iReturn);
		sz.Append(" - 1");
		mpcBackend->Line(IR816M_Per, sz.Text());
		if (IR816PointerOnDirectPage(iCallee))
		{
			//The target's bank and high byte pushed as one word and a byte's room below them (phb: any byte), then the
			//target - 1 written over that byte and the high byte (which the - 1 may have borrowed from): no change of
			//A's width for the bank's byte.
			mpcBackend->mcSelector.Frame(IR816M_Pei, iCallee + 1);
			mpcBackend->Line(IR816M_Phb);
			mpcBackend->mcSelector.Frame(IR816M_Lda, iCallee);
			mpcBackend->Line(IR816M_Dec, "A");
			mpcBackend->Line(IR816M_Sta, "[S + 1]");
		}
		else
		{
			mpcBackend->Line(IR816M_Sep, "m");
			mpcBackend->mcSelector.Frame(IR816M_Lda, iCallee + 2);
			mpcBackend->Line(IR816M_Pha);
			mpcBackend->Line(IR816M_Rep, "m");
			mpcBackend->mcSelector.Frame(IR816M_Lda, iCallee);
			mpcBackend->Line(IR816M_Dec, "A");
			mpcBackend->Line(IR816M_Pha);
		}
		mpcBackend->Line(IR816M_Rtl);
		mpcBackend->Label(iReturn);
	}
	sz.Kill();
	asArguments.Kill();

	//A thrown comes next (the verifier: straight after its call): the callee returned with the carry set if it threw.
	//When the block's brif is all that reads it and nothing between here and the brif changes the carry, it stays in
	//the carry and the brif is a bcs or bcc (CIRW65C816Backend::BranchWhen).  Else it is made a word now, in a scratch
	//word, before the arguments are removed, and Thrown puts it in its place.
	uiIndex = (size)(psInstruction - pcBlock->masInstructions.Get(0));
	psThrown = pcBlock->masInstructions.SafeGet(uiIndex + 1);
	mbThrownInCarry = false;
	if (psThrown && (psThrown->uiOpcode == IRO_Thrown))
	{
		mbThrownInCarry = (uiIndex + 2 == pcBlock->masInstructions.NumElements()) && (pcBlock->msJump.uiKind == IRJ_Branch) && psThrown->sResult.IsTemporary() &&
			pcBlock->msJump.sArgument.Equals(psThrown->sResult) && (psThrown->sResult.Index() < mpcBackend->masTemporaries.NumElements()) &&
			(mpcBackend->masTemporaries.Get(psThrown->sResult.Index())->iUses == 1) && (iBytes <= IR816_CARRY_PULL_BYTES) && ResultKeepsCarry(psInstruction);
		if (!mbThrownInCarry)
		{
			iThrown = mpcBackend->NewLabel();
			mpcBackend->Line(IR816M_Ldy, "0");
			mpcBackend->Branch(IR816M_Bcc, iThrown);
			mpcBackend->Line(IR816M_Iny);
			mpcBackend->Label(iThrown);
			mpcBackend->mcSelector.Frame(IR816M_Sty, IR816_SCRATCH_COUNT);
		}
	}
	CallerCleanUp(iBytes, psInstruction, mbThrownInCarry);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether putting the call's result in its place (CallerCleanUp) leaves the carry as it is: a result on the direct
// page is stored with sta and stx.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Calls::ResultKeepsCarry(SIRInstruction* psInstruction)
{
	if (psInstruction->sResult.IsNone() || mpcBackend->IsHiddenResult(psInstruction->sType))
	{
		return true;
	}
	return psInstruction->sResult.IsTemporary() && IR816OnDirectPage(mpcBackend->mcPlaces.OffsetOf(psInstruction->sResult) + 2 * mpcBackend->Words(psInstruction->sType));
}


//////////////////////////////////////////////////////////////////////////
//
// A thrown: nothing when the call left it in the carry for the brif (Call), else the word Call made of it goes to its
// place.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Calls::Thrown(SIRInstruction* psInstruction)
{
	if (mbThrownInCarry)
	{
		return;
	}
	mpcBackend->mcSelector.Frame(IR816M_Lda, IR816_SCRATCH_COUNT);
	mpcBackend->mcSelector.Store(psInstruction->sResult, 0);
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
			mpcBackend->Line(IR816M_Tdc);
			mpcBackend->Line(IR816M_Clc);
			mpcBackend->Line(IR816M_Adc, (int64)(aiHidden[i]));
			mpcBackend->Line(IR816M_Pha);
		}
		else if (aiHidden[i] >= 0)
		{
			mpcBackend->Line(IR816M_Pea, "0");
		}
		else
		{
			mpcBackend->mcSelector.PushWord(asValues[i], aiWords[i]);
		}
	}
	mpcBackend->Line(IR816M_Pla);
	if (iRegisters > 1)
	{
		mpcBackend->Line(IR816M_Plx);
	}
	if (iRegisters > 2)
	{
		mpcBackend->Line(IR816M_Ply);
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
	static const uint16	auiLoads[IR816_ARGUMENT_REGISTERS] = { IR816M_Lda, IR816M_Ldx, IR816M_Ldy };

	if ((iHiddenResult >= 0) && (iWord == 0))
	{
		//The hidden result pointer is the first argument: its low word is A's.
		mpcBackend->Line(IR816M_Tdc);
		mpcBackend->Line(IR816M_Clc);
		mpcBackend->Line(IR816M_Adc, (int64)(iHiddenResult));
	}
	else if (iHiddenResult >= 0)
	{
		mpcBackend->Line(auiLoads[iRegister], (int64)0);
	}
	else
	{
		mpcBackend->mcSelector.Operand(auiLoads[iRegister], sValue, iWord);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the function only calls another with its own arguments and
// returns what that returns (a wrapper): one block of args, a call of a
// symbol and a return of its result (or of nothing).  Each of the callee's
// arguments must already be where the callee wants it: the hidden result
// pointer and each parameter in the same register or in the same place
// among the pushed words (the pushed words the same, in the same order).
// A constant the callee takes in a register is loaded into it.  One
// argument in A (or A and X) may be a parameter in the same registers plus
// (or minus) a constant that leaves its high word alone (a member's address: 'this'
// moved to it), added before the jump.  Then pszJump gets that add, those
// loads and a jml to the callee: it returns to this function's caller, the
// pushed words where its caller left them.  False, with nothing in pszJump,
// for any other function.
//
//////////////////////////////////////////////////////////////////////////
bool CIRW65C816Calls::Forwarder(CChars* pszJump)
{
	static const char*		aszLoads[IR816_ARGUMENT_REGISTERS] = { "lda", "ldx", "ldy" };
	CIRBlock*				pcBlock;
	SIRInstruction*			psCall;
	SIRInstruction*			psArgument;
	SIRReference			sCallee;
	CArrayIRRegisterArguments	asCallee;
	CArrayTemplate<int>		aiSource;		//Per callee argument: -1 the hidden result pointer, -2 a constant, else the parameter's index.
	CArrayTemplate<int>		aiPushed;		//The callee's pushed arguments' sources, in order.
	CArrayTemplate<SIRInstruction*>	apsArguments;	//The block's args, in order.
	SIRInstruction*			psInstruction;
	SIRReference			sAdjusted;		//The temporary that is a parameter plus a constant, or none.
	size					uiAdjustedFrom;	//The parameter it adds to.
	uint64					ulliAdjust;
	size					uiAdjustedArgument;	//The callee's argument it is, or ARRAY_ELEMENT_NOT_FOUND.
	CChars					szAdjust;
	int32					iLabel;
	SIRRegisterArgument*			psPlaced;
	SIRRegisterArgument*			psOwn;
	size					ui;
	size					uj;
	size					uiFirst;
	size					uiOwnFirst;
	size					uiPushed;
	int						iSource;
	int						iRegisters;
	int						i;
	uint64					ulliBits;
	bool					bResult;
	CChars					szLoads;

	if ((mpcBackend->mpcFunction->NumBlocks() != 1) || (mpcBackend->mpcFunction->muiAttributes & IRA_Variadic))
	{
		return false;
	}
	pcBlock = mpcBackend->mpcFunction->GetBlock(0);
	if ((pcBlock->masPhis.NumElements() != 0) || (pcBlock->masInstructions.NumElements() == 0) || (pcBlock->msJump.uiKind != IRJ_Return))
	{
		return false;
	}
	psCall = pcBlock->masInstructions.Get(pcBlock->masInstructions.NumElements() - 1);
	sCallee = psCall->asArguments[0];
	if ((psCall->uiOpcode != IRO_Call) || !sCallee.IsSymbol() || (sCallee.Index() == mpcBackend->mpcFunction->muiSymbol) || (psCall->uiFlags & IRF_Variadic))
	{
		return false;
	}
	if (mpcBackend->mpcOptions->mbExceptions && (mpcBackend->mpcModule->FindFunction(sCallee.Index()) == NULL))
	{
		//A jump to the callee returns with the carry as the callee left it, which is only known to be clear (nothing
		//thrown) for a function compiled here: another unit's may be assembly, which leaves it as it falls.
		return false;
	}

	//It returns the call's result, of its own type, or nothing.
	if (mpcBackend->mpcFunction->msReturnType.IsNone())
	{
		if (!pcBlock->msJump.sArgument.IsNone() || (!psCall->sResult.IsNone() && mpcBackend->IsHiddenResult(psCall->sType)))
		{
			return false;
		}
	}
	else if (psCall->sResult.IsNone() || !pcBlock->msJump.sArgument.Equals(psCall->sResult) || !psCall->sType.Equals(mpcBackend->mpcFunction->msReturnType))
	{
		return false;
	}

	//The args, and at most one parameter plus a constant ahead of the arg that passes it.
	apsArguments.Init(CIRW65C816Backend::GetScratch());
	sAdjusted.uiValue = 0;
	uiAdjustedFrom = 0;
	ulliAdjust = 0;
	bResult = true;
	for (ui = 0; bResult && (ui + 1 < pcBlock->masInstructions.NumElements()); ui++)
	{
		psInstruction = pcBlock->masInstructions.Get(ui);
		if (psInstruction->uiOpcode == IRO_Arg)
		{
			apsArguments.Add(&psInstruction);
			continue;
		}
		bResult = ((psInstruction->uiOpcode == IRO_PtrAdd) || (psInstruction->uiOpcode == IRO_Add) || (psInstruction->uiOpcode == IRO_Sub)) &&
				  sAdjusted.IsNone() && psInstruction->sResult.IsTemporary() && psInstruction->asArguments[1].IsConstant();
		for (uj = 0; bResult && (uj < mpcBackend->mpcFunction->masParameters.NumElements()); uj++)
		{
			if (psInstruction->asArguments[0].Equals(*mpcBackend->mpcFunction->masParameters.Get(uj)))
			{
				sAdjusted = psInstruction->sResult;
				uiAdjustedFrom = uj;
				ulliAdjust = mpcBackend->mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits;
				if (psInstruction->uiOpcode == IRO_Sub)
				{
					//x - c is x + -c (at the argument's width).
					ulliAdjust = 0 - ulliAdjust;
				}
				break;
			}
		}
		bResult = bResult && !sAdjusted.IsNone();
	}
	if (!bResult)
	{
		apsArguments.Kill();
		return false;
	}

	//The callee's arguments as Call places them, and where each comes from.
	asCallee.Init(CIRW65C816Backend::GetScratch());
	aiSource.Init(CIRW65C816Backend::GetScratch());
	uiAdjustedArgument = ARRAY_ELEMENT_NOT_FOUND;
	if (!psCall->sResult.IsNone() && mpcBackend->IsHiddenResult(psCall->sType))
	{
		psPlaced = asCallee.Add();
		psPlaced->iUnits = 2;
		psPlaced->iRegister = -1;
		iSource = -1;
		aiSource.Add(&iSource);
	}
	for (ui = 0; bResult && (ui < apsArguments.NumElements()); ui++)
	{
		psArgument = *apsArguments.Get(ui);
		psPlaced = asCallee.Add();
		psPlaced->iUnits = mpcBackend->Words(psArgument->sType);
		psPlaced->iRegister = -1;
		iSource = -3;
		if (psArgument->asArguments[0].IsConstant())
		{
			iSource = -2;
		}
		for (uj = 0; uj < mpcBackend->mpcFunction->masParameters.NumElements(); uj++)
		{
			if (psArgument->asArguments[0].Equals(*mpcBackend->mpcFunction->masParameters.Get(uj)) &&
				(mpcBackend->Words(mpcBackend->mpcFunction->GetTemporaryType(*mpcBackend->mpcFunction->masParameters.Get(uj))) == psPlaced->iUnits))
			{
				iSource = (int)uj;
			}
		}
		if (!sAdjusted.IsNone() && psArgument->asArguments[0].Equals(sAdjusted))
		{
			if ((uiAdjustedArgument != ARRAY_ELEMENT_NOT_FOUND) || (mpcBackend->Words(mpcBackend->mpcFunction->GetTemporaryType(*mpcBackend->mpcFunction->masParameters.Get(uiAdjustedFrom))) != psPlaced->iUnits))
			{
				bResult = false;
				break;
			}
			iSource = (int)uiAdjustedFrom;
			uiAdjustedArgument = asCallee.NumElements() - 1;
		}
		bResult = iSource != -3;
		aiSource.Add(&iSource);
	}
	iRegisters = (bResult && mpcBackend->IsRegisterCall(sCallee)) ? mpcBackend->AssignRegisters(&asCallee, IR816_ARGUMENT_REGISTERS, IR816_ARGUMENT_REGISTERS) : 0;

	//The pushed words line up: the callee's pushed arguments are this function's pushed ones, in order.
	uiOwnFirst = mpcBackend->IsHiddenResult(mpcBackend->mpcFunction->msReturnType) ? 1 : 0;
	uiFirst = asCallee.NumElements() - apsArguments.NumElements();
	aiPushed.Init(CIRW65C816Backend::GetScratch());
	for (ui = 0; bResult && (ui < asCallee.NumElements()); ui++)
	{
		if (asCallee.Get(ui)->iRegister < 0)
		{
			if ((*aiSource.Get(ui) == -2) || (ui == uiAdjustedArgument))
			{
				bResult = false;
			}
			aiPushed.Add(aiSource.Get(ui));
		}
	}
	uiPushed = 0;
	for (ui = 0; bResult && (ui < mpcBackend->masArguments.NumElements()); ui++)
	{
		psOwn = mpcBackend->masArguments.Get(ui);
		if (psOwn->iRegister >= 0)
		{
			continue;
		}
		iSource = (ui < uiOwnFirst) ? -1 : (int)(ui - uiOwnFirst);
		bResult = (uiPushed < aiPushed.NumElements()) && (*aiPushed.Get(uiPushed) == iSource);
		uiPushed++;
	}
	bResult = bResult && (uiPushed == aiPushed.NumElements());

	//The register ones: in the same register here, or a constant loaded.
	szLoads.Init(CIRW65C816Backend::GetScratch());
	for (ui = 0; bResult && (ui < asCallee.NumElements()); ui++)
	{
		psPlaced = asCallee.Get(ui);
		if (psPlaced->iRegister < 0)
		{
			continue;
		}
		iSource = *aiSource.Get(ui);
		if (iSource == -2)
		{
			ulliBits = mpcBackend->mpcFunction->GetConstant((*apsArguments.Get(ui - uiFirst))->asArguments[0])->ulliBits;
			for (i = 0; i < psPlaced->iUnits; i++)
			{
				szLoads.Append("\t\t");
				szLoads.Append(aszLoads[psPlaced->iRegister + i]);
				szLoads.Append(' ');
				szLoads.Append((int)((ulliBits >> (16 * i)) & 0xFFFF));
				szLoads.AppendNewLine();
			}
			continue;
		}
		if ((iSource == -1) && (uiOwnFirst == 0))
		{
			bResult = false;
			break;
		}
		psOwn = mpcBackend->masArguments.Get((iSource == -1) ? 0 : (uiOwnFirst + (size)iSource));
		bResult = (psOwn->iRegister == psPlaced->iRegister) && (psOwn->iUnits == psPlaced->iUnits);
	}

	//The adjusted argument is in A (and X): the constant, at the argument's width, added to its low word and the
	//carry into X.  One that would change X otherwise (a pointer moved back) isn't a jump.
	szAdjust.Init(CIRW65C816Backend::GetScratch());
	if (bResult && (uiAdjustedArgument != ARRAY_ELEMENT_NOT_FOUND))
	{
		psPlaced = asCallee.Get(uiAdjustedArgument);
		ulliAdjust &= (psPlaced->iUnits == 1) ? 0xFFFFULL : 0xFFFFFFFFULL;
		bResult = (psPlaced->iRegister == 0) && (psPlaced->iUnits <= 2) && ((ulliAdjust >> 16) == 0);
		if (bResult && ((ulliAdjust & 0xFFFF) != 0))
		{
			szAdjust.Append("\t\tclc\n\t\tadc ");
			szAdjust.Append((int)(ulliAdjust & 0xFFFF));
			szAdjust.AppendNewLine();
			if (psPlaced->iUnits == 2)
			{
				iLabel = mpcBackend->NewLabel();
				szAdjust.Append("\t\tbcc L");
				szAdjust.Append(iLabel);
				szAdjust.Append("\n\t\tinx\nL");
				szAdjust.Append(iLabel);
				szAdjust.Append(':');
				szAdjust.AppendNewLine();
			}
		}
	}

	if (bResult)
	{
		pszJump->Append(szAdjust);
		pszJump->Append(szLoads);
		pszJump->Append("\t\tjml ");
		pszJump->Append(mpcBackend->mpcModule->GetSymbolName(sCallee.Index()));
		if (iRegisters > 0)
		{
			mpcBackend->AppendRegisterSuffix(pszJump, &asCallee, IR816_REGISTER_LETTERS);
		}
		pszJump->AppendNewLine();
	}
	szAdjust.Kill();
	szLoads.Kill();
	aiPushed.Kill();
	aiSource.Kill();
	asCallee.Kill();
	apsArguments.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A wrapper (Forwarder): its entry, then the jump (pszJump) instead of a
// frame and body.  It has no frame (FunctionArguments' empty one, which
// AlignFrame leaves).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Calls::GenerateForwarder(const char* szLabel, CChars* pszJump)
{
	Entry(szLabel);
	mpcBackend->mpszOut->Append(*pszJump);
}


//////////////////////////////////////////////////////////////////////////
//
// A function's label and widths.  A register function's plain label is
// the stub for callers that push everything (RegisterStub), then comes the
// body's own label (visible to other objects when the plain one is).
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Calls::Entry(const char* szLabel)
{
	CChars	szBody;

	mpcBackend->mpszOut->Append(szLabel);
	mpcBackend->mpszOut->Append(':');
	mpcBackend->mpszOut->AppendNewLine();
	mpcBackend->Line(".width (A, X), 16");
	mpcBackend->Line(".relax");
	if (mpcBackend->msFrame.iRegisterWords == 0)
	{
		return;
	}
	szBody.Init(szLabel);
	mpcBackend->AppendRegisterSuffix(&szBody, &mpcBackend->masArguments, IR816_REGISTER_LETTERS);
	RegisterStub(&szBody);
	if (!(mpcBackend->mpcFunction->muiAttributes & IRA_Local) && !mpcBackend->mpcModule->IsLocal(mpcBackend->mpcFunction->muiSymbol))
	{
		mpcBackend->mpszOut->Append((mpcBackend->mpcFunction->muiAttributes & IRA_Weak) ? "\t\t.weak " : "\t\t.global ");
		mpcBackend->mpszOut->Append(szBody);
		mpcBackend->mpszOut->AppendNewLine();
	}
	mpcBackend->mpszOut->Append(szBody);
	mpcBackend->mpszOut->Append(':');
	mpcBackend->mpszOut->AppendNewLine();
	szBody.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A register function's plain label, for callers that push every argument
// (function pointers, vtables, thunks, assembly): on entry argument word k
// is at S + 4 + 2k (after the jsl's return address).  The body wants its
// pushed words just above its return address (S + 4 on), in order, so they
// are moved down there (over the register words, read first) and it goes
// straight on to the body; the words left above them are the caller's to
// remove, as they always are.  Y's and X's words are loaded through A, then
// A's (kept in Y while the words move when its place is moved over).  When
// all three registers hold arguments and A's place is moved over there is
// nowhere to keep it: the pushed words are pushed again instead, the body
// is called, they are removed (A kept in Y, X kept) and it returns.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Calls::RegisterStub(CChars* pszBodyLabel)
{
	int				aiWordOf[IR816_ARGUMENT_REGISTERS];
	int				iWords;
	int				iStacked;
	int				iMoved;
	int				i;
	size			ui;
	SIRRegisterArgument*	psArgument;
	bool			bKeepA;

	//Each register's word, the pushed words, and how many of those aren't already where the body wants them.
	iWords = 0;
	iStacked = 0;
	iMoved = 0;
	for (ui = 0; ui < mpcBackend->masArguments.NumElements(); ui++)
	{
		psArgument = mpcBackend->masArguments.Get(ui);
		for (i = 0; i < psArgument->iUnits; i++)
		{
			if (psArgument->iRegister >= 0)
			{
				aiWordOf[psArgument->iRegister + i] = iWords;
			}
			else
			{
				if (iWords != iStacked)
				{
					iMoved++;
				}
				iStacked++;
			}
			iWords++;
		}
	}

	//A's word is moved over when its place is one the pushed words go to.
	bKeepA = (iMoved > 0) && (aiWordOf[0] < iStacked);
	if (bKeepA && (mpcBackend->msFrame.iRegisterWords == IR816_ARGUMENT_REGISTERS))
	{
		RegisterStubCall(pszBodyLabel, aiWordOf, iWords, iStacked);
		return;
	}

	//Y's and X's words, and A's into Y when the moves go over it.
	RegisterStubLoad(aiWordOf, mpcBackend->msFrame.iRegisterWords - 1, 1, 0);
	if (bKeepA)
	{
		RegisterStubLoad(aiWordOf, 0, 0, 0);
		mpcBackend->Line(IR816M_Tay);
	}

	//The pushed words down to S + 4 on, first first: each goes to a lower place than any still to be read.
	iStacked = 0;
	iWords = 0;
	for (ui = 0; ui < mpcBackend->masArguments.NumElements(); ui++)
	{
		psArgument = mpcBackend->masArguments.Get(ui);
		for (i = 0; i < psArgument->iUnits; i++)
		{
			if (psArgument->iRegister < 0)
			{
				if (iWords != iStacked)
				{
					StackLine(IR816M_Lda, 4 + 2 * iWords);
					StackLine(IR816M_Sta, 4 + 2 * iStacked);
				}
				iStacked++;
			}
			iWords++;
		}
	}

	//A's word, then on to the body (its label is next).
	if (bKeepA)
	{
		mpcBackend->Line(IR816M_Tya);
	}
	else
	{
		RegisterStubLoad(aiWordOf, 0, 0, 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Loads the register words iHigh down to iLow (2 Y, 1 X, 0 A) from the
// plain label's arguments, iPushed words further from S than on entry: Y's
// and X's through A.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Calls::RegisterStubLoad(int* aiWordOf, int iHigh, int iLow, int iPushed)
{
	int		k;

	for (k = iHigh; k >= iLow; k--)
	{
		StackLine(IR816M_Lda, 4 + 2 * aiWordOf[k] + 2 * iPushed);
		if (k == 2)
		{
			mpcBackend->Line(IR816M_Tay);
		}
		else if (k == 1)
		{
			mpcBackend->Line(IR816M_Tax);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// RegisterStub when the pushed words can't be moved down (all three
// registers hold arguments and A's place is moved over): they are pushed
// again, last first (each push moves the stack's words 2 further from S),
// the registers are loaded, the body is called, they are removed (A kept
// in Y, X kept) and it returns.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Calls::RegisterStubCall(CChars* pszBodyLabel, int* aiWordOf, int iWords, int iStacked)
{
	CChars			sz;
	int				iPushed;
	int				iWord;
	int				i;
	int32			iThrown;
	size			ui;
	SIRRegisterArgument*	psArgument;

	iPushed = 0;
	iWord = iWords;
	for (ui = mpcBackend->masArguments.NumElements(); ui > 0; ui--)
	{
		psArgument = mpcBackend->masArguments.Get(ui - 1);
		for (i = psArgument->iUnits - 1; i >= 0; i--)
		{
			iWord--;
			if (psArgument->iRegister < 0)
			{
				StackLine(IR816M_Lda, 4 + 2 * iWord + 2 * iPushed);
				mpcBackend->Line(IR816M_Pha);
				iPushed++;
			}
		}
	}

	RegisterStubLoad(aiWordOf, mpcBackend->msFrame.iRegisterWords - 1, 0, iStacked);

	sz.Init(CIRW65C816Backend::GetScratch());
	sz.Append(pszBodyLabel);
	mpcBackend->Line(IR816M_Jsl, sz.Text());
	iThrown = -1;
	if (mpcBackend->mpcOptions->mbExceptions && !mpcBackend->mpcModule->IsNoThrow(mpcBackend->mpcFunction->muiSymbol))
	{
		//Removing the pushed words loses the carry: a body that threw leaves by an exit of its own.
		iThrown = mpcBackend->NewLabel();
		mpcBackend->Branch(IR816M_Bcs, iThrown);
	}
	mpcBackend->Line(IR816M_Tay);
	mpcBackend->Line(IR816M_Tsc);
	mpcBackend->Line(IR816M_Clc);
	mpcBackend->Line(IR816M_Adc, (int64)(2 * iStacked));
	mpcBackend->Line(IR816M_Tcs);
	mpcBackend->Line(IR816M_Tya);
	mpcBackend->Line(IR816M_Rtl);
	if (iThrown >= 0)
	{
		mpcBackend->Label(iThrown);
		mpcBackend->Line(IR816M_Tsc);
		mpcBackend->Line(IR816M_Clc);
		mpcBackend->Line(IR816M_Adc, (int64)(2 * iStacked));
		mpcBackend->Line(IR816M_Tcs);
		mpcBackend->Line(IR816M_Sec);
		mpcBackend->Line(IR816M_Rtl);
	}
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// szInstruction [S + iOffset]: a stack relative load or store.
//
//////////////////////////////////////////////////////////////////////////
void CIRW65C816Calls::StackLine(uint16 uiMnemonic, int iOffset)
{
	CChars	sz;

	sz.Init(CIRW65C816Backend::GetScratch());
	sz.Append("[S + ");
	sz.Append(iOffset);
	sz.Append("]");
	mpcBackend->Line(uiMnemonic, sz.Text());
	sz.Kill();
}
