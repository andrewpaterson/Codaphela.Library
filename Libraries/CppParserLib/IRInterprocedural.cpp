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
#include "BaseLib/Chars.h"
#include "IRInliner.h"
#include "IROptimiser.h"
#include "IRInterprocedural.h"


//////////////////////////////////////////////////////////////////////////
//
// The index of the first arg instruction of the call at uiCall in pcBlock (the
// arg instructions straight before it).
//
//////////////////////////////////////////////////////////////////////////
static size IRFirstArgument(CIRBlock* pcBlock, size uiCall)
{
	size	uiFirst;

	for (uiFirst = uiCall; (uiFirst > 0) && (pcBlock->masInstructions.Get(uiFirst - 1)->uiOpcode == IRO_Arg); uiFirst--)
	{
	}
	return uiFirst;
}


//////////////////////////////////////////////////////////////////////////
//
// The argument for the callee's parameter uiParameter of the call at psSite,
// or none.
//
//////////////////////////////////////////////////////////////////////////
static SIRReference IRCallArgument(SIRCallSite* psSite, size uiParameter)
{
	size	uiFirst;

	uiFirst = IRFirstArgument(psSite->pcBlock, psSite->uiCall);
	if (uiFirst + uiParameter >= psSite->uiCall)
	{
		return IRNone();
	}
	return psSite->pcBlock->masInstructions.Get(uiFirst + uiParameter)->asArguments[0];
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an instruction's result is worked out from its operands so
// simply that a constant operand makes it a constant the back end does
// better with (a count worked out from a parameter, say 61 - iFraction).
//
//////////////////////////////////////////////////////////////////////////
static bool IRSpecialiseFollows(uint16 uiOpcode)
{
	switch (uiOpcode)
	{
	case IRO_Add:
	case IRO_Sub:
	case IRO_Mul:
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Not:
	case IRO_Neg:
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
	case IRO_ZExt:
	case IRO_SExt:
	case IRO_Trunc:
	case IRO_Copy:
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every use of sOld (an operand, a phi's argument, a branch's or return's
// value) in pcFunction is sNew.
//
//////////////////////////////////////////////////////////////////////////
static void IRReplaceUses(CIRFunction* pcFunction, SIRReference sOld, SIRReference sNew)
{
	size			uiBlock;
	size			ui;
	size			uj;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;

	for (uiBlock = 0; uiBlock < pcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = pcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
		{
			psPhi = pcBlock->masPhis.Get(ui);
			for (uj = 0; uj < psPhi->asArguments.NumElements(); uj++)
			{
				if (psPhi->asArguments.Get(uj)->sValue.Equals(sOld))
				{
					psPhi->asArguments.Get(uj)->sValue = sNew;
				}
			}
		}
		for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcBlock->masInstructions.Get(ui);
			for (uj = 0; uj < IR_INSTRUCTION_ARGUMENTS; uj++)
			{
				if (psInstruction->asArguments[uj].Equals(sOld))
				{
					psInstruction->asArguments[uj] = sNew;
				}
			}
		}
		if (pcBlock->msJump.sArgument.Equals(sOld))
		{
			pcBlock->msJump.sArgument = sNew;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether sValue is a temporary paucFlags has set.
//
//////////////////////////////////////////////////////////////////////////
static bool IRIsFlagged(CArrayTemplate<uint8>* paucFlags, SIRReference sValue)
{
	return sValue.IsTemporary() && (sValue.Index() < paucFlags->NumElements()) && *paucFlags->Get(sValue.Index());
}


//////////////////////////////////////////////////////////////////////////
//
// Sets the flag of sValue if it is a temporary.
//
//////////////////////////////////////////////////////////////////////////
static void IRSetFlag(CArrayTemplate<uint8>* paucFlags, SIRReference sValue)
{
	if (sValue.IsTemporary() && (sValue.Index() < paucFlags->NumElements()))
	{
		*paucFlags->Get(sValue.Index()) = 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Sets the flags of what is worked out from a value already flagged in
// pcFunction (IRSpecialiseFollows, and phis), until no more are.
//
//////////////////////////////////////////////////////////////////////////
static void IRFlagDerived(CIRFunction* pcFunction, CArrayTemplate<uint8>* paucDerived)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	size			uiBlock;
	size			ui;
	size			uj;
	bool			bChanged;

	do
	{
		bChanged = false;
		for (uiBlock = 0; uiBlock < pcFunction->NumBlocks(); uiBlock++)
		{
			pcBlock = pcFunction->GetBlock(uiBlock);
			for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
			{
				psPhi = pcBlock->masPhis.Get(ui);
				if (!psPhi->sResult.IsTemporary() || IRIsFlagged(paucDerived, psPhi->sResult))
				{
					continue;
				}
				for (uj = 0; uj < psPhi->asArguments.NumElements(); uj++)
				{
					if (IRIsFlagged(paucDerived, psPhi->asArguments.Get(uj)->sValue))
					{
						IRSetFlag(paucDerived, psPhi->sResult);
						bChanged = true;
						break;
					}
				}
			}
			for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
			{
				psInstruction = pcBlock->masInstructions.Get(ui);
				if (!IRSpecialiseFollows(psInstruction->uiOpcode) || !psInstruction->sResult.IsTemporary() || IRIsFlagged(paucDerived, psInstruction->sResult))
				{
					continue;
				}
				if (IRIsFlagged(paucDerived, psInstruction->asArguments[0]) || IRIsFlagged(paucDerived, psInstruction->asArguments[1]))
				{
					IRSetFlag(paucDerived, psInstruction->sResult);
					bChanged = true;
				}
			}
		}
	}
	while (bChanged);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether sValue is the function's symbol.
//
//////////////////////////////////////////////////////////////////////////
static bool IRIsSymbol(SIRReference sValue, uint32 uiSymbol)
{
	return sValue.IsSymbol() && (sValue.Index() == uiSymbol);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRInterprocedural::Init(CIRModule* pcModule, CIRUnitFunctions* pcUnit)
{
	size	ui;
	bool	bFalse;

	mpcModule = pcModule;
	mpcUnit = pcUnit;
	bFalse = false;
	mabChanged.Init();
	for (ui = 0; ui < mpcUnit->NumUnitFunctions(); ui++)
	{
		mabChanged.Add(&bFalse);
	}
	mabCallsNoWrites.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRInterprocedural::Kill(void)
{
	mabCallsNoWrites.Kill();
	mabChanged.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the unit function's code must be written again.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterprocedural::IsChanged(size uiFunction)
{
	return *mabChanged.Get(uiFunction);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the unit function calls a function that writes no memory, which is
// worth optimising again.  Those added after MarkNoWrites weren't looked at.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterprocedural::CallsNoWrites(size uiFunction)
{
	return (uiFunction < mabCallsNoWrites.NumElements()) && *mabCallsNoWrites.Get(uiFunction);
}


//////////////////////////////////////////////////////////////////////////
//
// pcFunction optimised again (a constant folded through, what only made an
// argument gone).
//
//////////////////////////////////////////////////////////////////////////
void CIRInterprocedural::Optimise(CIRFunction* pcFunction)
{
	CIROptimiser	cOptimiser;

	mpcUnit->InitOptimiser(&cOptimiser);
	cOptimiser.Optimise(pcFunction);
	cOptimiser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFunction may call itself: through the calls of the module's
// functions (calls of functions in other units are taken not to).
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterprocedural::IsRecursive(CIRModule* pcModule, CIRFunction* pcFunction)
{
	CArrayTemplate<CIRFunction*>	apcToDo;
	CArrayTemplate<CIRFunction*>	apcSeen;
	CIRFunction*					pcCaller;
	CIRFunction*					pcCallee;
	CIRBlock*						pcBlock;
	SIRInstruction*					psInstruction;
	size							ui;
	size							uiBlock;
	size							uiInstruction;
	bool							bSeen;
	bool							bRecursive;

	apcToDo.Init();
	apcSeen.Init();
	apcToDo.Add(&pcFunction);
	bRecursive = false;
	while ((apcToDo.NumElements() > 0) && !bRecursive)
	{
		pcCaller = *apcToDo.Tail();
		apcToDo.RemoveTail();
		for (uiBlock = 0; (uiBlock < pcCaller->NumBlocks()) && !bRecursive; uiBlock++)
		{
			pcBlock = pcCaller->GetBlock(uiBlock);
			for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
			{
				psInstruction = pcBlock->masInstructions.Get(uiInstruction);
				if ((psInstruction->uiOpcode != IRO_Call) || !psInstruction->asArguments[0].IsSymbol())
				{
					continue;
				}
				pcCallee = pcModule->FindFunction(psInstruction->asArguments[0].Index());
				if (pcCallee == pcFunction)
				{
					bRecursive = true;
					break;
				}
				bSeen = (pcCallee == NULL);
				for (ui = 0; (ui < apcSeen.NumElements()) && !bSeen; ui++)
				{
					bSeen = *apcSeen.Get(ui) == pcCallee;
				}
				if (!bSeen)
				{
					apcSeen.Add(&pcCallee);
					apcToDo.Add(&pcCallee);
				}
			}
		}
	}
	apcSeen.Kill();
	apcToDo.Kill();
	return bRecursive;
}


//////////////////////////////////////////////////////////////////////////
//
// Mod/ref summaries (before inlining): which of the unit's functions write
// no memory their callers can see (CIROptimiser::WritesMemory), marked on
// their symbols (CIRModule::SetNoWrites) so the optimiser keeps what it
// knows is in memory across calls to them.  Each is taken to write nothing
// until it is seen to (a store outside its frame, a call to one that
// writes), again until none changes, so functions that call each other and
// store nothing are marked too.  CallsNoWrites then gives those that call
// one of them.  Functions of other units are taken to write.
//
//////////////////////////////////////////////////////////////////////////
void CIRInterprocedural::MarkNoWrites(void)
{
	CIROptimiser	cOptimiser;
	CIRFunction*	pcFunction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	size			ui;
	size			uiBlock;
	size			uiInstruction;
	bool			bChanged;
	bool			bCalls;

	for (ui = 0; ui < mpcUnit->NumUnitFunctions(); ui++)
	{
		pcFunction = mpcUnit->GetUnitFunction(ui);
		if (pcFunction)
		{
			mpcModule->SetNoWrites(pcFunction->muiSymbol, true);
		}
	}

	mpcUnit->InitOptimiser(&cOptimiser);
	do
	{
		bChanged = false;
		for (ui = 0; ui < mpcUnit->NumUnitFunctions(); ui++)
		{
			pcFunction = mpcUnit->GetUnitFunction(ui);
			if (pcFunction && mpcModule->IsNoWrites(pcFunction->muiSymbol) && cOptimiser.WritesMemory(pcFunction))
			{
				mpcModule->SetNoWrites(pcFunction->muiSymbol, false);
				bChanged = true;
			}
		}
	}
	while (bChanged);
	cOptimiser.Kill();

	for (ui = 0; ui < mpcUnit->NumUnitFunctions(); ui++)
	{
		pcFunction = mpcUnit->GetUnitFunction(ui);
		bCalls = false;
		for (uiBlock = 0; pcFunction && (uiBlock < pcFunction->NumBlocks()) && !bCalls; uiBlock++)
		{
			pcBlock = pcFunction->GetBlock(uiBlock);
			for (uiInstruction = 0; (uiInstruction < pcBlock->masInstructions.NumElements()) && !bCalls; uiInstruction++)
			{
				psInstruction = pcBlock->masInstructions.Get(uiInstruction);
				bCalls = (psInstruction->uiOpcode == IRO_Call) && psInstruction->asArguments[0].IsSymbol() && mpcModule->IsNoWrites(psInstruction->asArguments[0].Index());
			}
		}
		mabCallsNoWrites.Add(&bCalls);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Interprocedural constant propagation (before inlining).  A function of
// the unit that only the unit calls (OnlyTheUnitCalls, its address never
// taken) whose parameter is a constant at every call is specialised for
// it: with one value the parameter's uses are the constant; with up to
// IR_SPECIALISE_VALUES values each gets a copy of the function (named
// with the parameter and the value) its calls go to, and the function goes
// when nothing else names it.  Only parameters worth it are: those that
// are (or something worked out from them is) a shift's count, a multiply's
// or a divide's operand, or an argument of a specialisable parameter of
// another (StrongParameters).  The copies' own calls then have constants
// (softfloat.cpp's Add(..., iFraction) calls RoundPack(..., iFraction)), so
// it goes round again, IR_SPECIALISE_ROUNDS times at most.
//
//////////////////////////////////////////////////////////////////////////
void CIRInterprocedural::SpecialiseConstantArguments(void)
{
	CArrayTemplate<uint32>	auiStrong;
	CIRFunction*			pcFunction;
	size					ui;
	int						iRound;
	bool					bChanged;
	bool					bAny;
	uint32					uiStrong;
	uint32					uiZero;

	uiZero = 0;
	auiStrong.Init();
	for (iRound = 0; iRound < IR_SPECIALISE_ROUNDS; iRound++)
	{
		//Which parameters are worth it, to a fixed point (a parameter passed on is as good as the one it is passed to).
		auiStrong.FakeSetUsedElements(0);
		for (ui = 0; ui < mpcUnit->NumUnitFunctions(); ui++)
		{
			auiStrong.Add(&uiZero);
		}
		do
		{
			bChanged = false;
			for (ui = 0; ui < auiStrong.NumElements(); ui++)
			{
				pcFunction = mpcUnit->GetUnitFunction(ui);
				if (pcFunction)
				{
					uiStrong = StrongParameters(pcFunction, &auiStrong);
					if (uiStrong != *auiStrong.Get(ui))
					{
						*auiStrong.Get(ui) = uiStrong;
						bChanged = true;
					}
				}
			}
		}
		while (bChanged);

		bAny = false;
		for (ui = 0; ui < auiStrong.NumElements(); ui++)
		{
			if (*auiStrong.Get(ui) != 0)
			{
				bAny |= SpecialiseCallee(ui, *auiStrong.Get(ui));
			}
		}
		if (!bAny)
		{
			break;
		}
	}
	auiStrong.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The unit function whose symbol is uiSymbol, as its index, or -1.
//
//////////////////////////////////////////////////////////////////////////
int CIRInterprocedural::FunctionIndex(uint32 uiSymbol)
{
	CIRFunction*	pcFunction;
	size			ui;

	for (ui = 0; ui < mpcUnit->NumUnitFunctions(); ui++)
	{
		pcFunction = mpcUnit->GetUnitFunction(ui);
		if (pcFunction && (pcFunction->muiSymbol == uiSymbol))
		{
			return (int)ui;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The parameters of pcFunction (a bit each, the first IR_PARAMETER_BITS)
// worth specialising for a constant (SpecialiseConstantArguments): a value
// worked out from one (IRFlagDerived) counts (IsStrong).
//
//////////////////////////////////////////////////////////////////////////
uint32 CIRInterprocedural::StrongParameters(CIRFunction* pcFunction, CArrayTemplate<uint32>* pauiStrong)
{
	CArrayTemplate<uint8>	aucDerived;
	SIRReference			sParameter;
	size					uiParameter;
	size					ui;
	uint32					uiStrong;
	uint8					uiZero;

	uiStrong = 0;
	uiZero = 0;
	aucDerived.Init();
	for (uiParameter = 0; (uiParameter < pcFunction->masParameters.NumElements()) && (uiParameter < IR_PARAMETER_BITS); uiParameter++)
	{
		sParameter = *pcFunction->masParameters.Get(uiParameter);
		if (!sParameter.IsTemporary())
		{
			continue;
		}
		aucDerived.FakeSetUsedElements(0);
		for (ui = 0; ui < pcFunction->NumTemporaries(); ui++)
		{
			aucDerived.Add(&uiZero);
		}
		IRSetFlag(&aucDerived, sParameter);
		IRFlagDerived(pcFunction, &aucDerived);
		if (IsStrong(pcFunction, &aucDerived, pauiStrong))
		{
			uiStrong |= (uint32)1 << uiParameter;
		}
	}
	aucDerived.Kill();
	return uiStrong;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a value in paucDerived is a shift's count, a multiply's or divide's
// operand, or an argument for a parameter of a unit function that is worth it
// (pauiStrong, per unit function).
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterprocedural::IsStrong(CIRFunction* pcFunction, CArrayTemplate<uint8>* paucDerived, CArrayTemplate<uint32>* pauiStrong)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	size			uiBlock;
	size			ui;
	size			uj;
	size			uiFirst;
	int				iCallee;

	for (uiBlock = 0; uiBlock < pcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = pcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcBlock->masInstructions.Get(ui);
			switch (psInstruction->uiOpcode)
			{
			case IRO_Shl:
			case IRO_LShr:
			case IRO_AShr:
				if (IRIsFlagged(paucDerived, psInstruction->asArguments[1]))
				{
					return true;
				}
				break;
			case IRO_Mul:
			case IRO_UDiv:
			case IRO_SDiv:
			case IRO_URem:
			case IRO_SRem:
				if (IRIsFlagged(paucDerived, psInstruction->asArguments[0]) || IRIsFlagged(paucDerived, psInstruction->asArguments[1]))
				{
					return true;
				}
				break;
			case IRO_Call:
				iCallee = psInstruction->asArguments[0].IsSymbol() ? FunctionIndex(psInstruction->asArguments[0].Index()) : -1;
				if ((iCallee < 0) || ((size)iCallee >= pauiStrong->NumElements()))
				{
					break;
				}
				uiFirst = IRFirstArgument(pcBlock, ui);
				for (uj = uiFirst; (uj < ui) && (uj - uiFirst < IR_PARAMETER_BITS); uj++)
				{
					if (IRIsFlagged(paucDerived, pcBlock->masInstructions.Get(uj)->asArguments[0]) && (*pauiStrong->Get((size)iCallee) & ((uint32)1 << (uj - uiFirst))))
					{
						return true;
					}
				}
				break;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFunction has a body and takes a fixed number of arguments, and
// only its unit can call it.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterprocedural::OnlyTheUnitCalls(CIRFunction* pcFunction)
{
	return pcFunction && (pcFunction->NumBlocks() != 0) && !(pcFunction->muiAttributes & IRA_Variadic) &&
		   ((pcFunction->muiAttributes & IRA_Local) || mpcModule->IsLocal(pcFunction->muiSymbol));
}


//////////////////////////////////////////////////////////////////////////
//
// The calls of the function uiSymbol in the unit's functions.  False if
// there are none, or it is named otherwise (its address taken).
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterprocedural::FindCalls(uint32 uiSymbol, CArrayIRCallSites* pasSites)
{
	SIRCallSite		sSite;
	CIRFunction*	pcFunction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	size			uiFunction;
	size			uiBlock;
	size			ui;
	size			uj;

	pasSites->FakeSetUsedElements(0);
	for (uiFunction = 0; uiFunction < mpcUnit->NumUnitFunctions(); uiFunction++)
	{
		pcFunction = mpcUnit->GetUnitFunction(uiFunction);
		for (uiBlock = 0; pcFunction && (uiBlock < pcFunction->NumBlocks()); uiBlock++)
		{
			pcBlock = pcFunction->GetBlock(uiBlock);
			for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
			{
				psInstruction = pcBlock->masInstructions.Get(ui);
				if (IRIsSymbol(psInstruction->asArguments[1], uiSymbol))
				{
					return false;
				}
				if (IRIsSymbol(psInstruction->asArguments[0], uiSymbol))
				{
					if (psInstruction->uiOpcode != IRO_Call)
					{
						return false;
					}
					sSite.uiFunction = uiFunction;
					sSite.pcBlock = pcBlock;
					sSite.uiCall = ui;
					pasSites->Add(&sSite);
				}
			}
			for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
			{
				psPhi = pcBlock->masPhis.Get(ui);
				for (uj = 0; uj < psPhi->asArguments.NumElements(); uj++)
				{
					if (IRIsSymbol(psPhi->asArguments.Get(uj)->sValue, uiSymbol))
					{
						return false;
					}
				}
			}
			if (IRIsSymbol(pcBlock->msJump.sArgument, uiSymbol))
			{
				return false;
			}
		}
	}
	return pasSites->NumElements() != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The constant passed for uiParameter at psSite, which ConstantArguments
// found is one.
//
//////////////////////////////////////////////////////////////////////////
uint64 CIRInterprocedural::SiteConstant(SIRCallSite* psSite, size uiParameter)
{
	return mpcUnit->GetUnitFunction(psSite->uiFunction)->GetConstant(IRCallArgument(psSite, uiParameter))->ulliBits;
}


//////////////////////////////////////////////////////////////////////////
//
// The values passed for uiParameter at the calls.  False if one isn't a
// constant.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterprocedural::ConstantArguments(CArrayIRCallSites* pasSites, size uiParameter, CArrayTemplate<uint64>* paulliValues)
{
	uint64	ulliValue;
	size	ui;
	size	uj;

	paulliValues->FakeSetUsedElements(0);
	for (ui = 0; ui < pasSites->NumElements(); ui++)
	{
		if (!IRCallArgument(pasSites->Get(ui), uiParameter).IsConstant())
		{
			return false;
		}
		ulliValue = SiteConstant(pasSites->Get(ui), uiParameter);
		for (uj = 0; (uj < paulliValues->NumElements()) && (*paulliValues->Get(uj) != ulliValue); uj++)
		{
		}
		if (uj == paulliValues->NumElements())
		{
			paulliValues->Add(&ulliValue);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Specialises the unit function uiCallee for the first of its strong
// parameters (uiStrong) that has a constant at every call
// (SpecialiseConstantArguments).  True if it did.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterprocedural::SpecialiseCallee(size uiCallee, uint32 uiStrong)
{
	CArrayIRCallSites		asSites;
	CArrayTemplate<uint64>	aulliValues;
	CIRFunction*			pcCallee;
	SIRReference			sParameter;
	size					uiParameter;
	bool					bDone;

	pcCallee = mpcUnit->GetUnitFunction(uiCallee);
	if (!OnlyTheUnitCalls(pcCallee) || (CIRInliner::NumInstructions(pcCallee) > IR_SPECIALISE_INSTRUCTIONS))
	{
		return false;
	}
	asSites.Init();
	aulliValues.Init();
	bDone = false;
	if (FindCalls(pcCallee->muiSymbol, &asSites))
	{
		for (uiParameter = 0; (uiParameter < pcCallee->masParameters.NumElements()) && (uiParameter < IR_PARAMETER_BITS) && !bDone; uiParameter++)
		{
			if (!(uiStrong & ((uint32)1 << uiParameter)) || !ConstantArguments(&asSites, uiParameter, &aulliValues) || (aulliValues.NumElements() > IR_SPECIALISE_VALUES))
			{
				continue;
			}
			bDone = true;
			if (aulliValues.NumElements() == 1)
			{
				//One value: the parameter is that constant (the calls still pass it).
				sParameter = *pcCallee->masParameters.Get(uiParameter);
				IRReplaceUses(pcCallee, sParameter, pcCallee->AddConstant(pcCallee->GetTemporaryType(sParameter), *aulliValues.Get(0)));
				Optimise(pcCallee);
				*mabChanged.Get(uiCallee) = true;
			}
			else
			{
				CopyForValues(uiCallee, uiParameter, &asSites, &aulliValues);
			}
		}
	}
	aulliValues.Kill();
	asSites.Kill();
	return bDone;
}


//////////////////////////////////////////////////////////////////////////
//
// A copy of the unit function uiCallee for each value of its parameter (a
// function of that name is kept, not written again), and the calls with those
// values go to them, the copies' own too (the function may call itself).  It
// goes if nothing calls it then and nothing else names it
// (CIRUnitFunctions::RemoveSpecialised).
//
//////////////////////////////////////////////////////////////////////////
void CIRInterprocedural::CopyForValues(size uiCallee, size uiParameter, CArrayIRCallSites* pasSites, CArrayTemplate<uint64>* paulliValues)
{
	CArrayTemplate<uint32>	auiCopies;
	CIRFunction*			pcCallee;
	CIRFunction*			pcCopy;
	SIRCallSite*			psSite;
	SIRReference			sParameter;
	SIRType					sType;
	CChars					szName;
	uint64					ulliValue;
	size					ui;
	size					uj;
	size					uiFunctions;
	bool					bTrue;

	bTrue = true;
	pcCallee = mpcUnit->GetUnitFunction(uiCallee);
	sParameter = *pcCallee->masParameters.Get(uiParameter);
	sType = pcCallee->GetTemporaryType(sParameter);
	auiCopies.Init();
	for (ui = 0; ui < paulliValues->NumElements(); ui++)
	{
		ulliValue = *paulliValues->Get(ui);
		szName.Init(mpcModule->GetSymbolName(pcCallee->muiSymbol));
		szName.Append("__K");
		szName.Append((int)uiParameter);
		szName.Append('_');
		if ((int64)ulliValue < 0)
		{
			szName.Append('m');
			szName.Append((int64)(0 - ulliValue));
		}
		else
		{
			szName.Append((int64)ulliValue);
		}
		uiFunctions = mpcModule->NumFunctions();
		pcCopy = mpcModule->CopyFunction(pcCallee, szName.Text());
		szName.Kill();
		auiCopies.Add(&pcCopy->muiSymbol);
		if (mpcModule->NumFunctions() != uiFunctions)
		{
			IRReplaceUses(pcCopy, sParameter, pcCopy->AddConstant(sType, ulliValue));
			Optimise(pcCopy);
			mpcUnit->AddSpecialised(uiCallee, pcCopy);
			mabChanged.Add(&bTrue);
		}
	}

	FindCalls(pcCallee->muiSymbol, pasSites);
	for (ui = 0; ui < pasSites->NumElements(); ui++)
	{
		psSite = pasSites->Get(ui);
		if (!IRCallArgument(psSite, uiParameter).IsConstant())
		{
			continue;
		}
		ulliValue = SiteConstant(psSite, uiParameter);
		for (uj = 0; uj < paulliValues->NumElements(); uj++)
		{
			if (*paulliValues->Get(uj) == ulliValue)
			{
				psSite->pcBlock->masInstructions.Get(psSite->uiCall)->asArguments[0] = IRReference(IRRK_Symbol, *auiCopies.Get(uj));
				*mabChanged.Get(psSite->uiFunction) = true;
				break;
			}
		}
	}
	auiCopies.Kill();

	//Its name was only in calls (FindCalls found it wasn't taken), so no calls means nothing in the IR names it.
	if (!FindCalls(pcCallee->muiSymbol, pasSites) && mpcUnit->RemoveSpecialised(uiCallee))
	{
		*mabChanged.Get(uiCallee) = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The parameters (the first IR_PARAMETER_BITS) pcFunction never reads, as
// bits: no use of its temporary.  None if anything takes a parameter's
// address (argaddr): an object passed by value in pieces is reached through
// its first's.  A member function's this stays (IRA_ThisCall: the 65C64
// passes it in X).
//
//////////////////////////////////////////////////////////////////////////
uint32 CIRInterprocedural::DeadParameters(CIRFunction* pcFunction)
{
	CArrayTemplate<uint8>	aucUsed;
	CIRBlock*				pcBlock;
	SIRInstruction*			psInstruction;
	SIRPhi*					psPhi;
	SIRReference			sValue;
	size					uiBlock;
	size					ui;
	size					uj;
	size					uiParameters;
	uint32					uiDead;

	uiParameters = pcFunction->masParameters.NumElements();
	if (uiParameters > IR_PARAMETER_BITS)
	{
		uiParameters = IR_PARAMETER_BITS;
	}

	//Which temporaries are read.
	aucUsed.Init();
	aucUsed.GrowToAtLeastNumElements(pcFunction->NumTemporaries(), true, 0);
	for (uiBlock = 0; uiBlock < pcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = pcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masInstructions.NumElements(); ui++)
		{
			psInstruction = pcBlock->masInstructions.Get(ui);
			if (psInstruction->uiOpcode == IRO_ArgAddress)
			{
				aucUsed.Kill();
				return 0;
			}
			IRSetFlag(&aucUsed, psInstruction->asArguments[0]);
			IRSetFlag(&aucUsed, psInstruction->asArguments[1]);
		}
		for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
		{
			psPhi = pcBlock->masPhis.Get(ui);
			for (uj = 0; uj < psPhi->asArguments.NumElements(); uj++)
			{
				IRSetFlag(&aucUsed, psPhi->asArguments.Get(uj)->sValue);
			}
		}
		IRSetFlag(&aucUsed, pcBlock->msJump.sArgument);
	}

	uiDead = 0;
	for (ui = (pcFunction->muiAttributes & IRA_ThisCall) ? 1 : 0; ui < uiParameters; ui++)
	{
		sValue = *pcFunction->masParameters.Get(ui);
		if (sValue.IsTemporary() && (sValue.Index() < aucUsed.NumElements()) && !IRIsFlagged(&aucUsed, sValue))
		{
			uiDead |= (uint32)1 << ui;
		}
	}
	aucUsed.Kill();
	return uiDead;
}


//////////////////////////////////////////////////////////////////////////
//
// Dead argument removal (after SpecialiseConstantArguments): a function of
// the unit that only the unit calls (OnlyTheUnitCalls, its address never
// taken) loses the parameters it never reads (DeadParameters), and its
// calls their arguments, so they aren't worked out, loaded or pushed (a
// parameter specialised for its one value is one such).  The callee and its
// callers are optimised again (what only made an argument goes) and their
// code is to be written again (IsChanged).
//
//////////////////////////////////////////////////////////////////////////
void CIRInterprocedural::RemoveDeadArguments(void)
{
	CArrayIRCallSites		asSites;
	CArrayTemplate<bool>	abOptimise;
	CIRFunction*			pcCallee;
	SIRCallSite*			psSite;
	SIRInstruction*			psCall;
	uint32					uiDead;
	size					uiCallee;
	size					ui;
	bool					bFalse;
	bool					bAllArguments;

	bFalse = false;
	abOptimise.Init();
	for (ui = 0; ui < mpcUnit->NumUnitFunctions(); ui++)
	{
		abOptimise.Add(&bFalse);
	}
	asSites.Init();
	for (uiCallee = 0; uiCallee < mpcUnit->NumUnitFunctions(); uiCallee++)
	{
		pcCallee = mpcUnit->GetUnitFunction(uiCallee);
		if (!OnlyTheUnitCalls(pcCallee))
		{
			continue;
		}
		uiDead = DeadParameters(pcCallee);
		if ((uiDead == 0) || !FindCalls(pcCallee->muiSymbol, &asSites))
		{
			continue;
		}

		//Each call must have an argument per parameter.
		bAllArguments = true;
		for (ui = 0; (ui < asSites.NumElements()) && bAllArguments; ui++)
		{
			psSite = asSites.Get(ui);
			psCall = psSite->pcBlock->masInstructions.Get(psSite->uiCall);
			bAllArguments = !(psCall->uiFlags & IRF_Variadic) && ((psSite->uiCall - IRFirstArgument(psSite->pcBlock, psSite->uiCall)) == pcCallee->masParameters.NumElements());
		}
		if (bAllArguments)
		{
			RemoveCalleeArguments(pcCallee, uiDead, &asSites, &abOptimise);
			*abOptimise.Get(uiCallee) = true;
		}
	}
	asSites.Kill();

	for (ui = 0; ui < abOptimise.NumElements(); ui++)
	{
		if (*abOptimise.Get(ui))
		{
			Optimise(mpcUnit->GetUnitFunction(ui));
			*mabChanged.Get(ui) = true;
		}
	}
	abOptimise.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The parameters uiDead of pcCallee, and the calls' arguments for them:
// last call first (the sites before a removal keep their places), last
// parameter first.
//
//////////////////////////////////////////////////////////////////////////
void CIRInterprocedural::RemoveCalleeArguments(CIRFunction* pcCallee, uint32 uiDead, CArrayIRCallSites* pasSites, CArrayTemplate<bool>* pabOptimise)
{
	SIRCallSite*	psSite;
	size			uiParameters;
	size			uiFirst;
	size			ui;
	size			uj;

	uiParameters = pcCallee->masParameters.NumElements();
	for (ui = pasSites->NumElements(); ui > 0; ui--)
	{
		psSite = pasSites->Get(ui - 1);
		uiFirst = psSite->uiCall - uiParameters;
		for (uj = uiParameters; uj > 0; uj--)
		{
			if ((uj <= IR_PARAMETER_BITS) && (uiDead & ((uint32)1 << (uj - 1))))
			{
				psSite->pcBlock->masInstructions.RemoveAt(uiFirst + uj - 1, true);
			}
		}
		*pabOptimise->Get(psSite->uiFunction) = true;
	}
	for (uj = uiParameters; uj > 0; uj--)
	{
		if ((uj <= IR_PARAMETER_BITS) && (uiDead & ((uint32)1 << (uj - 1))))
		{
			pcCallee->masParameters.RemoveAt(uj - 1, true);
		}
	}
}
