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
#include "BaseLib/StringHelper.h"
#include "IR65Backend.h"


//A class of temporaries sharing a home and how much they are used, for the order homes are given in.
struct SIR65Weight
{
	int32	iLeader;
	uint32	uiWeight;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int CompareIR65Weights(const void* pv1, const void* pv2)
{
	SIR65Weight*	ps1;
	SIR65Weight*	ps2;

	ps1 = (SIR65Weight*)pv1;
	ps2 = (SIR65Weight*)pv2;
	if (ps1->uiWeight != ps2->uiWeight)
	{
		return (ps1->uiWeight > ps2->uiWeight) ? -1 : 1;
	}
	return ps1->iLeader - ps2->iLeader;
}


//What the lowering of an opcode does (IR65OpcodeFlags).
#define IR65F_TAKES_A		0x01	//Takes its first operand from A when the instruction before left it there.
#define IR65F_TAKES_B		0x02	//Takes its second operand from A (copying or swapping it) the same way.
#define IR65F_COMMUTES		0x04
#define IR65F_CALLS			0x08	//Changes X, Y and [$00 + W] to [$7F + W].
#define IR65F_CHANGES_XY	0x10	//Calls the runtime: changes X and Y.
#define IR65F_CHANGES_Y		0x20	//mul and div: change Y.
#define IR65F_NO_DEFER		0x40	//Loads into registers the deferred load's address may use.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint8 IR65OpcodeFlags(uint16 uiOpcode)
{
	if (IsIRIntegerComparison(uiOpcode))
	{
		return IR65F_TAKES_A | IR65F_TAKES_B | (((uiOpcode == IRO_Eq) || (uiOpcode == IRO_Ne)) ? IR65F_COMMUTES : 0);
	}
	switch (uiOpcode)
	{
	case IRO_Add:
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_PtrAdd:
		return IR65F_TAKES_A | IR65F_TAKES_B | IR65F_COMMUTES;
	case IRO_Mul:
		return IR65F_TAKES_A | IR65F_TAKES_B | IR65F_COMMUTES | IR65F_CHANGES_Y | IR65F_NO_DEFER;
	case IRO_Sub:
	case IRO_Shl:
	case IRO_LShr:
		return IR65F_TAKES_A | IR65F_TAKES_B;
	case IRO_AShr:
		return IR65F_TAKES_A | IR65F_TAKES_B | IR65F_NO_DEFER;
	case IRO_SDiv:
	case IRO_SRem:
		return IR65F_TAKES_A | IR65F_TAKES_B | IR65F_CHANGES_XY | IR65F_NO_DEFER;
	case IRO_UDiv:
	case IRO_URem:
		return IR65F_TAKES_A | IR65F_TAKES_B | IR65F_CHANGES_Y | IR65F_NO_DEFER;
	case IRO_Neg:
	case IRO_Not:
	case IRO_Copy:
	case IRO_SExt:
	case IRO_ZExt:
	case IRO_Trunc:
	case IRO_PtrToI:
	case IRO_IToPtr:
	case IRO_SpaceCast:
	case IRO_Store:
		return IR65F_TAKES_A;
	case IRO_Call:
		return IR65F_CALLS;
	case IRO_FAdd:
	case IRO_FSub:
	case IRO_FMul:
	case IRO_FDiv:
	case IRO_SIToF:
	case IRO_UIToF:
	case IRO_FToSI:
	case IRO_FToUI:
	case IRO_FExt:
	case IRO_FTrunc:
	case IRO_FEq:
	case IRO_FNe:
	case IRO_FLt:
	case IRO_FLe:
	case IRO_FGt:
	case IRO_FGe:
	case IRO_FOrdered:
	case IRO_FUnordered:
		return IR65F_CHANGES_XY;
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// piLabels is the counter labels are numbered from (the generator's), and
// paszRuntime the list the soft float routines' labels are added to.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Init(CIRModule* pcModule, int* piLabels, CArrayChars* paszRuntime)
{
	mpcModule = pcModule;
	mpiLabels = piLabels;
	mpaszRuntime = paszRuntime;
	mbSignedDivide = false;
	mbSignedDivide64 = false;
	mpcFunction = NULL;
	mpszOut = NULL;
	mpcErrors = NULL;
	mcDominators.Init();
	mcLiveness.Init();
	masPlaces.Init();
	mauiAddressed.Init();
	maiClasses.Init();
	maiLeaders.Init();
	masHomes.Init();
	maiCalls.Init();
	maiChangesXY.Init();
	maiChangesY.Init();
	maiBlockLabels.Init();
	mauiUses.Init();
	masMoves.Init();
	masStubs.Init();
	muiSaved = 0;
	miFrameBytes = 0;
	miArguments = 0;
	miThisOffset = -1;
	miReturnLabel = 0;
	mbResult = true;
	msInA = IRNone();
	msAvailable = IRNone();
	msDeferred = IRNone();
	mszDeferred.Init();
	mpsWritten = NULL;
	mpsFlags = NULL;
	mpcFlags = NULL;
	mbFlagsKnown = false;
	mbRightImmediate = false;
	mbFrameless = false;
	mcFrame = 'V';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Kill(void)
{
	size	ui;

	mszDeferred.Kill();
	masStubs.Kill();
	masMoves.Kill();
	mauiUses.Kill();
	maiBlockLabels.Kill();
	maiChangesY.Kill();
	maiChangesXY.Kill();
	maiCalls.Kill();
	for (ui = 0; ui < masHomes.NumElements(); ui++)
	{
		masHomes.Get(ui)->asRanges.Kill();
	}
	masHomes.Kill();
	maiLeaders.Kill();
	maiClasses.Kill();
	mauiAddressed.Kill();
	masPlaces.Kill();
	mcLiveness.Kill();
	mcDominators.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// True if a function needs R_DivideSigned (or R_DivideSigned64), which the
// generator adds to the unit.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::UsesSignedDivide(void)
{
	return mbSignedDivide;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::UsesSignedDivide64(void)
{
	return mbSignedDivide64;
}


//////////////////////////////////////////////////////////////////////////
//
// Every function (each a text section of its own) and global (a data
// section) of the module, for CAsm65Assembler::AssembleObject.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::GenerateModule(CChars* pszDest, CCPPErrors* pcErrors)
{
	size			ui;
	CIRFunction*	pcFunction;
	char*			szName;
	bool			bResult;

	bResult = true;
	for (ui = 0; ui < mpcModule->NumFunctions(); ui++)
	{
		pcFunction = mpcModule->GetFunction(ui);
		szName = mpcModule->GetSymbolName(pcFunction->muiSymbol);
		pszDest->Append("\t\t.section text.");
		pszDest->Append(szName);
		pszDest->AppendNewLine();
		Binding(szName, pcFunction->muiAttributes, pszDest);
		bResult &= GenerateFunction(pcFunction, szName, pszDest, pcErrors);
	}
	for (ui = 0; ui < mpcModule->NumGlobals(); ui++)
	{
		GenerateGlobal(mpcModule->GetGlobal(ui), pszDest);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// .global or .weak, or nothing for the unit's own label.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Binding(const char* szName, uint32 uiAttributes, CChars* pszDest)
{
	if (uiAttributes & IRA_Local)
	{
		return;
	}
	pszDest->Append((uiAttributes & IRA_Weak) ? "\t\t.weak " : "\t\t.global ");
	pszDest->Append(szName);
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// A global's data section, alignment, binding, label and items.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::GenerateGlobal(CIRGlobal* pcGlobal, CChars* pszDest)
{
	size			ui;
	size			uiByte;
	SIRDataItem*	psItem;
	char*			szName;
	int				iBytes;

	szName = mpcModule->GetSymbolName(pcGlobal->muiSymbol);
	pszDest->Append("\t\t.section data.");
	pszDest->Append(szName);
	pszDest->AppendNewLine();
	if (pcGlobal->muiAlign > 1)
	{
		pszDest->Append("\t\t.align ");
		pszDest->Append(pcGlobal->muiAlign);
		pszDest->AppendNewLine();
	}
	Binding(szName, pcGlobal->muiAttributes, pszDest);
	pszDest->Append(szName);
	pszDest->Append(':');
	pszDest->AppendNewLine();

	for (ui = 0; ui < pcGlobal->masItems.NumElements(); ui++)
	{
		psItem = pcGlobal->masItems.Get(ui);
		switch (psItem->uiKind)
		{
		case IRDK_Value:
		case IRDK_Symbol:
			iBytes = mpcModule->GetSize(psItem->sType);
			pszDest->Append((iBytes == 1) ? "\t\t.byte " : (iBytes == 2) ? "\t\t.word " : (iBytes == 4) ? "\t\t.long " : "\t\t.quad ");
			if (psItem->uiKind == IRDK_Symbol)
			{
				pszDest->Append(mpcModule->GetSymbolName((uint32)psItem->ulliValue));
				if (psItem->lliOffset != 0)
				{
					pszDest->Append(psItem->lliOffset > 0 ? " + " : " - ");
					pszDest->Append(psItem->lliOffset > 0 ? psItem->lliOffset : -psItem->lliOffset);
				}
			}
			else
			{
				pszDest->Append(IRSignExtend(psItem->sType, psItem->ulliValue));
			}
			pszDest->AppendNewLine();
			break;
		case IRDK_Bytes:
			for (uiByte = 0; uiByte < psItem->uiLength; uiByte++)
			{
				pszDest->Append(((uiByte % 16) == 0) ? "\t\t.byte " : ", ");
				pszDest->Append((int32)*pcGlobal->macBytes.Get(psItem->uiStart + uiByte));
				if (((uiByte % 16) == 15) || (uiByte + 1 == psItem->uiLength))
				{
					pszDest->AppendNewLine();
				}
			}
			break;
		case IRDK_Zero:
			if (psItem->ulliValue > 0)
			{
				pszDest->Append("\t\t.space ");
				pszDest->Append(psItem->ulliValue);
				pszDest->AppendNewLine();
			}
			break;
		}
	}
	pszDest->AppendNewLine();
}

//////////////////////////////////////////////////////////////////////////
//
// Puts the blocks in reverse post order, a brif's else visited first so its
// then block comes straight after it (a loop's body after its test, which
// RotateLoops then moves to the bottom).  Blocks nothing reaches go last.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::OrderBlocks(void)
{
	CArrayIRBlocks	apcOrder;
	size			ui;

	mcDominators.Compute(mpcFunction, true);
	apcOrder.Init();
	for (ui = 0; ui < mcDominators.NumReachable(); ui++)
	{
		apcOrder.Add(mcDominators.GetReversePostOrder(ui));
	}
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		if (!mcDominators.IsReachable(mpcFunction->GetBlock(ui)))
		{
			apcOrder.Add(mpcFunction->GetBlock(ui));
		}
	}
	mpcFunction->SetBlockOrder(&apcOrder);
	mpcFunction->ComputePredecessors();
	apcOrder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Moves a loop's test to after its body: a block that branches into the
// block after it or out of the loop, and that the last block of the loop
// jumps back to, goes after that block.  The loop is entered by a bra to
// the test, and each time round takes one branch.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::RotateLoops(void)
{
	CArrayIRBlocks	apcOrder;
	CArrayIRBlocks	apcHeads;
	CArrayIRBlocks	apcLatches;
	size			ui;
	size			uiPredecessor;
	size			uiHead;
	size			uiLatch;
	CIRBlock*		pcHead;
	CIRBlock*		pcLatch;
	CIRBlock*		pcPredecessor;
	CIRBlock*		pcOutside;
	SIRJump*		psJump;

	mcDominators.Compute(mpcFunction);
	apcHeads.Init();
	apcLatches.Init();
	for (ui = 1; ui + 1 < mpcFunction->NumBlocks(); ui++)
	{
		pcHead = mpcFunction->GetBlock(ui);
		psJump = &pcHead->msJump;
		if ((psJump->uiKind != IRJ_Branch) || !mcDominators.IsReachable(pcHead))
		{
			continue;
		}
		if (psJump->pcThen == mpcFunction->GetBlock(ui + 1))
		{
			pcOutside = psJump->pcElse;
		}
		else if (psJump->pcElse == mpcFunction->GetBlock(ui + 1))
		{
			pcOutside = psJump->pcThen;
		}
		else
		{
			continue;
		}

		pcLatch = NULL;
		for (uiPredecessor = 0; uiPredecessor < pcHead->mapcPredecessors.NumElements(); uiPredecessor++)
		{
			pcPredecessor = pcHead->mapcPredecessors.GetPtr(uiPredecessor);
			if ((pcPredecessor->muiIndex > ui) && mcDominators.IsBackEdge(pcPredecessor, pcHead) &&
				((pcLatch == NULL) || (pcPredecessor->muiIndex > pcLatch->muiIndex)))
			{
				pcLatch = pcPredecessor;
			}
		}
		if ((pcLatch == NULL) || (pcLatch->msJump.uiKind != IRJ_Jump) || ((pcOutside->muiIndex > ui) && (pcOutside->muiIndex <= pcLatch->muiIndex)))
		{
			continue;
		}
		apcHeads.Add(pcHead);
		apcLatches.Add(pcLatch);
	}

	if (apcHeads.NumElements() > 0)
	{
		apcOrder.Init();
		apcOrder.Copy(&mpcFunction->mapcBlocks);
		for (ui = 0; ui < apcHeads.NumElements(); ui++)
		{
			pcHead = apcHeads.GetPtr(ui);
			uiHead = apcOrder.Find(pcHead);
			apcOrder.RemoveAt(uiHead, true);
			uiLatch = apcOrder.Find(apcLatches.GetPtr(ui));
			apcOrder.InsertAt(&pcHead, uiLatch + 1);
		}
		mpcFunction->SetBlockOrder(&apcOrder);
		apcOrder.Kill();
	}
	apcLatches.Kill();
	apcHeads.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Moves a load used once, later in its block, to just before its use when
// nothing between writes memory (a store, a call, a volatile load) or
// pushes arguments, so the use can take it from A or as a memory operand.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::SinkLoads(void)
{
	size			ui;
	size			uiLoad;
	size			uiUse;
	CIRBlock*		pcBlock;
	SIRInstruction*	psLoad;
	SIRInstruction*	psUse;
	SIRInstruction	sLoad;
	SIRReference	sResult;
	bool			bBlocked;

	mpcFunction->CountUses(&mauiUses);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiLoad = pcBlock->masInstructions.NumElements(); uiLoad > 0; uiLoad--)
		{
			psLoad = pcBlock->masInstructions.Get(uiLoad - 1);
			sResult = psLoad->sResult;
			if ((psLoad->uiOpcode != IRO_Load) || (psLoad->uiFlags & IRF_Volatile) || !sResult.IsTemporary() || (*mauiUses.Get(sResult.Index()) != 1))
			{
				continue;
			}
			bBlocked = false;
			for (uiUse = uiLoad; uiUse < pcBlock->masInstructions.NumElements(); uiUse++)
			{
				psUse = pcBlock->masInstructions.Get(uiUse);
				if (psUse->asArguments[0].Equals(sResult) || psUse->asArguments[1].Equals(sResult))
				{
					break;
				}
				if ((psUse->uiOpcode == IRO_Store) || (psUse->uiOpcode == IRO_Call) || (psUse->uiOpcode == IRO_Arg) || ((psUse->uiOpcode == IRO_Load) && (psUse->uiFlags & IRF_Volatile)))
				{
					bBlocked = true;
					break;
				}
			}
			if (bBlocked || (uiUse >= pcBlock->masInstructions.NumElements()) || (uiUse == uiLoad) ||
				(pcBlock->masInstructions.Get(uiUse)->uiOpcode == IRO_Arg) || (pcBlock->masInstructions.Get(uiUse)->uiOpcode == IRO_Call))
			{
				continue;
			}
			sLoad = *psLoad;
			pcBlock->masInstructions.RemoveAt(uiLoad - 1, true);
			pcBlock->masInstructions.InsertAt(&sLoad, uiUse - 1);
		}
		SinkToEnd(pcBlock);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Moves an operation whose result only a successor's phi takes (the next
// time round a loop, say: p + 1) to the end of its block, before the
// comparison the block's brif may take.  Its operands' other uses come
// first, so an operand that dies there can share its home (add Y, 1).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::SinkToEnd(CIRBlock* pcBlock)
{
	size			uiInstruction;
	size			uiUse;
	size			uiEnd;
	SIRInstruction*	psInstruction;
	SIRInstruction*	psUse;
	SIRInstruction	sMoved;
	SIRReference	sResult;
	bool			bUsed;
	bool			bOperandLater;

	uiEnd = pcBlock->masInstructions.NumElements();
	psInstruction = pcBlock->masInstructions.SafeGet(uiEnd - 1);
	if (psInstruction && psInstruction->sResult.Equals(pcBlock->msJump.sArgument))
	{
		uiEnd--;
	}
	for (uiInstruction = uiEnd; uiInstruction > 0; uiInstruction--)
	{
		psInstruction = pcBlock->masInstructions.Get(uiInstruction - 1);
		sResult = psInstruction->sResult;
		switch (psInstruction->uiOpcode)
		{
		case IRO_Add:
		case IRO_Sub:
		case IRO_PtrAdd:
			break;
		default:
			continue;
		}
		if (!sResult.IsTemporary() || (*mauiUses.Get(sResult.Index()) != 1) || pcBlock->msJump.sArgument.Equals(sResult))
		{
			continue;
		}
		//Only worth it when an operand is used again later in the block.
		bUsed = false;
		bOperandLater = false;
		for (uiUse = uiInstruction; uiUse < pcBlock->masInstructions.NumElements(); uiUse++)
		{
			psUse = pcBlock->masInstructions.Get(uiUse);
			if (psUse->asArguments[0].Equals(sResult) || psUse->asArguments[1].Equals(sResult))
			{
				bUsed = true;
				break;
			}
			bOperandLater |= (psInstruction->asArguments[0].IsTemporary() && (psUse->asArguments[0].Equals(psInstruction->asArguments[0]) || psUse->asArguments[1].Equals(psInstruction->asArguments[0]))) ||
							 (psInstruction->asArguments[1].IsTemporary() && (psUse->asArguments[0].Equals(psInstruction->asArguments[1]) || psUse->asArguments[1].Equals(psInstruction->asArguments[1])));
		}
		if (bUsed || !bOperandLater || (uiInstruction == uiEnd))
		{
			continue;
		}
		sMoved = *psInstruction;
		pcBlock->masInstructions.RemoveAt(uiInstruction - 1, true);
		pcBlock->masInstructions.InsertAt(&sMoved, uiEnd - 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if psInstruction is a ptradd of a constant (which a load or store
// can fold into its address: [8 + X]).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::ConstantPtrAdd(SIRInstruction* psInstruction)
{
	return (psInstruction->uiOpcode == IRO_PtrAdd) && psInstruction->sResult.IsTemporary() && psInstruction->asArguments[1].IsConstant() && !IsWide(mpcFunction->TypeOf(psInstruction->asArguments[1]));
}


//////////////////////////////////////////////////////////////////////////
//
// Gives each load and store whose address is a ptradd of a constant a ptradd
// of its own just before it (unless the one it uses is there and used only
// by it), so MarkFolded can fold it into the address.  A ptradd left unused
// isn't written.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::CopyAddresses(void)
{
	CArrayIRInstructions	asNew;
	CArrayTemplate<int32>	aiDefinitions;	//Per temporary: its constant ptradd in asDefinitions, -1 for none.
	CArrayIRInstructions	asDefinitions;
	size					ui;
	size					uiInstruction;
	size					uiArgument;
	CIRBlock*				pcBlock;
	SIRInstruction*			psInstruction;
	SIRInstruction*			psLast;
	SIRInstruction			sCopy;
	SIRReference			sAddress;
	int32					iDefinition;
	bool					bChanged;

	//SinkLoads counted the uses (moving loads doesn't change them).
	aiDefinitions.Init();
	aiDefinitions.GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, -1);
	asDefinitions.Init();
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (ConstantPtrAdd(psInstruction))
			{
				*aiDefinitions.Get(psInstruction->sResult.Index()) = (int32)asDefinitions.NumElements();
				asDefinitions.Add(psInstruction);
			}
		}
	}

	if (asDefinitions.NumElements() > 0)
	{
		asNew.Init();
		for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
		{
			pcBlock = mpcFunction->GetBlock(ui);
			asNew.FakeSetUsedElements(0);
			bChanged = false;
			for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
			{
				sCopy = *pcBlock->masInstructions.Get(uiInstruction);
				uiArgument = (sCopy.uiOpcode == IRO_Load) ? 0 : 1;
				sAddress = IRAddressOf(&sCopy);
				if (sAddress.IsTemporary() && (sAddress.Index() < aiDefinitions.NumElements()))
				{
					iDefinition = *aiDefinitions.Get(sAddress.Index());
					psLast = asNew.SafeGet(asNew.NumElements() - 1);
					if ((iDefinition >= 0) && !(psLast && psLast->sResult.Equals(sAddress) && (*mauiUses.Get(sAddress.Index()) == 1)))
					{
						psInstruction = asNew.Add(asDefinitions.Get(iDefinition));
						psInstruction->sResult = mpcFunction->AddTemporary(psInstruction->sType);
						sCopy.asArguments[uiArgument] = psInstruction->sResult;
						bChanged = true;
					}
				}
				asNew.Add(&sCopy);
			}
			if (bChanged)
			{
				pcBlock->masInstructions.Copy(&asNew);
			}
		}
		asNew.Kill();
	}
	asDefinitions.Kill();
	aiDefinitions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A ptradd of a constant just before the load or store that alone uses it
// becomes part of the load or store's address.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::MarkFolded(void)
{
	size			ui;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRInstruction*	psPrevious;
	SIRReference	sAddress;
	SIR65Place*		psPlace;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 1; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			sAddress = IRAddressOf(psInstruction);
			if (sAddress.IsNone())
			{
				continue;
			}
			psPrevious = pcBlock->masInstructions.Get(uiInstruction - 1);
			if (ConstantPtrAdd(psPrevious) && psPrevious->sResult.Equals(sAddress) && (*mauiUses.Get(sAddress.Index()) == 1))
			{
				psPlace = PlaceOf(sAddress);
				psPlace->uiKind = IR65P_Folded;
				psPlace->iOffset = (int32)mpcFunction->GetSigned(psPrevious->asArguments[1]);
				psPlace->sBase = psPrevious->asArguments[0];
				psPlace->bInA = false;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The function's label, frame, blocks and return, appended to pszDest
// (see Prologue and Epilogue).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::GenerateFunction(CIRFunction* pcFunction, const char* szLabel, CChars* pszDest, CCPPErrors* pcErrors)
{
	size		ui;
	CIRBlock*	pcBlock;
	int32		iLabel;

	mpcFunction = pcFunction;
	mpszOut = pszDest;
	mpcErrors = pcErrors;
	mbResult = true;

	pcFunction->Renumber();
	pcFunction->ComputePredecessors();
	OrderBlocks();
	RotateLoops();
	SinkLoads();
	CopyAddresses();
	mpcFunction->CountUses(&mauiUses);
	mcDominators.Compute(pcFunction);
	mcLiveness.Compute(pcFunction, &mcDominators);
	Layout();
	Allocate();

	//A leaf function with nothing in the frame doesn't set V up: its arguments are found from S, which only moves
	//around mul and div (when nothing is read from the stack).  Otherwise V points at the frame.
	mbFrameless = (miFrameBytes == 0) && (muiSaved == 0) && (miThisOffset < 0) && IsLeaf();
	mcFrame = mbFrameless ? 'S' : 'V';
	miArguments = mbFrameless ? (1 + 4) : (1 + miFrameBytes + 8);

	maiBlockLabels.FakeSetUsedElements(0);
	for (ui = 0; ui < pcFunction->NumBlocks(); ui++)
	{
		iLabel = NewLabel();
		maiBlockLabels.Add(&iLabel);
	}
	miReturnLabel = NewLabel();

	Prologue(szLabel);
	mpsFlags = NULL;
	mpcFlags = NULL;
	for (ui = 0; ui < pcFunction->NumBlocks(); ui++)
	{
		pcBlock = pcFunction->GetBlock(ui);
		if (!Block(pcBlock, (ui + 1 < pcFunction->NumBlocks()) ? pcFunction->GetBlock(ui + 1) : NULL))
		{
			mbResult = false;
		}
	}
	Epilogue();
	return mbResult;
}


//////////////////////////////////////////////////////////////////////////
//
//	label:
//			psh V
//			sub S, frame
//			cpy V, S
//			psh [n + W]			; each [$80 + W] to [$FF + W] unit used
//			...					; parameters copied to their homes
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Prologue(const char* szLabel)
{
	size			ui;
	int32			iUnit;
	SIRReference	sParameter;
	SIRType			sType;
	SIR65Place*		psPlace;
	SIR65Place		sArrival;

	mpszOut->Append(szLabel);
	mpszOut->Append(':');
	mpszOut->AppendNewLine();
	if (!mbFrameless)
	{
		Line("psh V")->AppendNewLine();
		if (miFrameBytes > 0)
		{
			Line("sub S, ")->Append(miFrameBytes)->AppendNewLine();
		}
		Line("cpy V, S")->AppendNewLine();
	}
	for (iUnit = 0; iUnit < IR65_PSEUDO_UNITS; iUnit++)
	{
		if (muiSaved & (1u << iUnit))
		{
			Line("psh [")->Append(128 + 4 * iUnit)->Append(" + W]")->AppendNewLine();
		}
	}

	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		sParameter = *mpcFunction->masParameters.Get(ui);
		sType = mpcFunction->GetTemporaryType(sParameter);
		psPlace = PlaceOf(sParameter);
		if ((ui == 0) && (mpcFunction->muiAttributes & IRA_ThisCall))
		{
			//this, in X.
			if (miThisOffset >= 0)
			{
				Line("cpy [")->Append(1 + miThisOffset)->Append(" + V], X")->AppendNewLine();
			}
			if ((psPlace->uiKind == IR65P_Register) && (psPlace->iOffset == IR65R_Y))
			{
				Line("cpy Y, X")->AppendNewLine();
			}
			else if ((psPlace->uiKind == IR65P_Pseudo) || (psPlace->uiKind == IR65P_Frame))
			{
				StorePlace(psPlace, sType, "X");
			}
			continue;
		}
		if ((psPlace->uiKind == IR65P_Register) || (psPlace->uiKind == IR65P_Pseudo) || (psPlace->uiKind == IR65P_Frame))
		{
			ArrivalPlace(ui, &sArrival);
			if (psPlace->uiKind == IR65P_Register)
			{
				LoadPlace(&sArrival, sType, RegisterName(psPlace), IR65E_Zero);
			}
			else
			{
				LoadPlace(&sArrival, sType, "A", IR65E_Zero);
				StorePlace(psPlace, sType, "A");
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//	Lreturn:
//			pul [n + W]			; in reverse
//			add S, frame
//			pul V
//			ret
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Epilogue(void)
{
	int32		iUnit;
	size		ui;
	SIR65Stub*	psStub;

	Label(miReturnLabel);
	for (iUnit = IR65_PSEUDO_UNITS - 1; iUnit >= 0; iUnit--)
	{
		if (muiSaved & (1u << iUnit))
		{
			Line("pul [")->Append(128 + 4 * iUnit)->Append(" + W]")->AppendNewLine();
		}
	}
	if (!mbFrameless)
	{
		if (miFrameBytes > 0)
		{
			Line("add S, ")->Append(miFrameBytes)->AppendNewLine();
		}
		Line("pul V")->AppendNewLine();
	}
	Line("ret")->AppendNewLine();

	//Edges whose copies were moved out of the way.
	for (ui = 0; ui < masStubs.NumElements(); ui++)
	{
		psStub = masStubs.Get(ui);
		Label(psStub->iLabel);
		Edge(psStub->pcFrom, psStub->pcTo);
		Branch("bra ", *maiBlockLabels.Get(psStub->pcTo->muiIndex));
	}
	masStubs.FakeSetUsedElements(0);
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Error(const char* szMessage, const char* szDetail)
{
	CChars	szError;

	szError.Init("@");
	szError.Append(mpcModule->GetSymbolName(mpcFunction->muiSymbol));
	szError.Append(": ");
	szError.Append(szMessage);
	if (mpcErrors)
	{
		mpcErrors->Add(NULL, -1, -1, -1, szError.Text(), szDetail);
	}
	szError.Kill();
	mbResult = false;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Starts an instruction line.  The caller appends the rest and the new line.
//
//////////////////////////////////////////////////////////////////////////
CChars* CIR65Backend::Line(const char* szText)
{
	mpszOut->Append("\t\t");
	return mpszOut->Append(szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CIR65Backend::NewLabel(void)
{
	(*mpiLabels)++;
	return *mpiLabels;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Label(int32 iLabel)
{
	mpszOut->Append('L');
	mpszOut->Append(iLabel);
	mpszOut->Append(':');
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// szBranch is the instruction and its flag ("bras z, "), or "bra ".
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Branch(const char* szBranch, int32 iLabel)
{
	Line(szBranch)->Append('L')->Append(iLabel)->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Calls a soft float routine (_ and its name) and removes iBytes of
// arguments.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::CallRuntime(const char* szName, int iBytes)
{
	CChars	szLabel;

	szLabel.Init("_");
	szLabel.Append(szName);
	if (mpaszRuntime)
	{
		mpaszRuntime->AddIfUnique(szLabel);
	}
	Line("jsr ")->Append(szLabel)->AppendNewLine();
	szLabel.Kill();
	if (iBytes > 0)
	{
		Line("add S, ")->Append(iBytes)->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Bytes a value of sType takes as an argument or in memory.
//
//////////////////////////////////////////////////////////////////////////
int CIR65Backend::ValueBytes(SIRType sType)
{
	return mpcModule->GetSize(sType);
}


//////////////////////////////////////////////////////////////////////////
//
// 64 bit values are worked on with the registers 64 bits wide.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IsWide(SIRType sType)
{
	return sType.Bits() == 64;
}


//////////////////////////////////////////////////////////////////////////
//
// i8 and i16 (and i1), held zero extended.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IsNarrow(SIRType sType)
{
	int	iBits;

	iBits = sType.IntegerBits();
	return (iBits > 0) && (iBits < 32);
}


//////////////////////////////////////////////////////////////////////////
//
// iBytes more of the frame, at least 4 and a multiple of 4.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR65Backend::AddFrame(int32 iBytes)
{
	int32	iOffset;

	iOffset = miFrameBytes;
	miFrameBytes += (iBytes < 4) ? 4 : ((iBytes + 3) & ~3);
	return iOffset;
}


//////////////////////////////////////////////////////////////////////////
//
// The comparison a block's brif alone uses, which branches without making
// its i1; NULL for none.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIR65Backend::FusedComparison(CIRBlock* pcBlock)
{
	SIRInstruction*		psLast;
	const SIROpcode*	psOpcode;

	if (pcBlock->msJump.uiKind != IRJ_Branch)
	{
		return NULL;
	}
	psLast = pcBlock->masInstructions.SafeGet(pcBlock->masInstructions.NumElements() - 1);
	if (psLast == NULL)
	{
		return NULL;
	}
	psOpcode = GetIROpcode(psLast->uiOpcode);
	if (psOpcode && (psOpcode->uiResult == IRRS_I1) && psLast->sResult.Equals(pcBlock->msJump.sArgument) && (*mauiUses.Get(psLast->sResult.Index()) == 1))
	{
		return psLast;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A result is left in A, and not kept, when the next instruction (or the
// block's jump) is its only use and takes it from A.  A comparison its
// block's brif fuses has no result either.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::MarkInA(void)
{
	size			ui;
	size			uiInstruction;
	size			uiNext;
	size			uiNumInstructions;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRInstruction*	psNext;
	SIRInstruction*	psFused;
	SIRReference	sResult;
	bool			bInA;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		psFused = FusedComparison(pcBlock);
		uiNumInstructions = pcBlock->masInstructions.NumElements();
		if (psFused)
		{
			masPlaces.Get(psFused->sResult.Index())->bInA = true;
			uiNumInstructions--;
		}
		for (uiInstruction = 0; uiInstruction < uiNumInstructions; uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			sResult = psInstruction->sResult;
			if (!sResult.IsTemporary() || (*mauiUses.Get(sResult.Index()) != 1) || Skipped(psInstruction))
			{
				continue;
			}

			//The next instruction that writes something.
			uiNext = NextWritten(pcBlock, uiInstruction, uiNumInstructions);
			if (uiNext < uiNumInstructions)
			{
				psNext = pcBlock->masInstructions.Get(uiNext);
				bInA = TakesInA(psNext, sResult) || TakesInB(psNext, sResult) || ((psNext->uiOpcode == IRO_Load) && psNext->asArguments[0].Equals(sResult)) || FoldedBase(psNext, sResult);
			}
			else
			{
				bInA = JumpTakesInA(pcBlock, psFused, sResult) || (psFused && TakesInB(psFused, sResult)) || (psFused && FoldedBase(psFused, sResult));
			}
			masPlaces.Get(sResult.Index())->bInA = bInA;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if psNext is a load or store whose address is a folded ptradd of
// sValue.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::FoldedBase(SIRInstruction* psNext, SIRReference sValue)
{
	SIRReference	sAddress;

	sAddress = IRAddressOf(psNext);
	return sAddress.IsTemporary() && (PlaceOf(sAddress)->uiKind == IR65P_Folded) && PlaceOf(sAddress)->sBase.Equals(sValue);
}


//////////////////////////////////////////////////////////////////////////
//
// Where a parameter the caller pushed starts, from the first argument.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR65Backend::ParameterOffset(size uiParameter)
{
	size	ui;
	int32	iOffset;

	iOffset = 0;
	for (ui = 0; ui < uiParameter; ui++)
	{
		if ((ui == 0) && (mpcFunction->muiAttributes & IRA_ThisCall))
		{
			continue;
		}
		iOffset += ValueBytes(mpcFunction->GetTemporaryType(*mpcFunction->masParameters.Get(ui)));
	}
	return iOffset;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the caller pushed a parameter.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::ArrivalPlace(size uiParameter, SIR65Place* psPlace)
{
	psPlace->uiKind = IR65P_Argument;
	psPlace->iOffset = ParameterOffset(uiParameter);
	psPlace->iBytes = ValueBytes(mpcFunction->GetTemporaryType(*mpcFunction->masParameters.Get(uiParameter)));
	psPlace->bInA = false;
}


//////////////////////////////////////////////////////////////////////////
//
// Slots get frame space (their temporary is its address) and an argaddr is
// where the caller pushed its parameter (a thiscall's this gets a frame
// place, stored from X, for it).  Results left in A are marked.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Layout(void)
{
	size			ui;
	size			uiInstruction;
	int32			iParameter;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIR65Place*		psPlace;

	masPlaces.FakeSetUsedElements(0);
	masPlaces.GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, 0);
	mauiAddressed.FakeSetUsedElements(0);
	mauiAddressed.GrowToAtLeastNumElements(mpcFunction->masParameters.NumElements(), true, 0);
	miFrameBytes = 0;
	miThisOffset = -1;
	MarkFolded();
	MarkInA();

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (!psInstruction->sResult.IsTemporary())
			{
				continue;
			}
			psPlace = masPlaces.Get(psInstruction->sResult.Index());
			if (psInstruction->uiOpcode == IRO_Slot)
			{
				psPlace->uiKind = IR65P_FrameAddress;
				psPlace->iBytes = (int32)mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits;
				psPlace->iOffset = AddFrame(psPlace->iBytes);
			}
			else if (psInstruction->uiOpcode == IRO_ArgAddress)
			{
				iParameter = (int32)mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits;
				if ((size)iParameter >= mpcFunction->masParameters.NumElements())
				{
					Error("argaddr of a parameter the function doesn't have.");
				}
				else if ((iParameter == 0) && (mpcFunction->muiAttributes & IRA_ThisCall))
				{
					if (miThisOffset < 0)
					{
						miThisOffset = AddFrame(4);
					}
					psPlace->uiKind = IR65P_FrameAddress;
					psPlace->iOffset = miThisOffset;
				}
				else
				{
					psPlace->uiKind = IR65P_ArgumentAddress;
					psPlace->iOffset = ParameterOffset((size)iParameter);
					*mauiAddressed.Get((size)iParameter) = 1;
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if a temporary needs a home: it is used, isn't an address and isn't
// left in A.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::NeedsHome(int32 iTemporary)
{
	SIR65Place*		psPlace;
	SIRReference	sTemporary;

	psPlace = masPlaces.Get(iTemporary);
	sTemporary = IRReference(IRRK_Temporary, (uint32)iTemporary);
	return (psPlace->uiKind == IR65P_None) && !psPlace->bInA && (*mauiUses.Get(iTemporary) > 0) && (mcLiveness.NumRanges(sTemporary) > 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Gives every temporary that needs one a home.  Phis and the values they
// take share a class (and a home) where they don't interfere; classes are
// given homes most weighted first (a use in a loop counts 8 times more): X
// or Y if nothing that changes them is done while the class is live, then
// a low pseudo register if no call is made.  A parameter used at most once,
// or that gets none of those, stays where it was pushed.  Then a kept
// pseudo register if the class weighs 8 or more (it costs a push and a
// pull), then the frame.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Allocate(void)
{
	CArrayTemplate<SIR65Weight>	asWeights;
	SIR65Weight*				psWeight;
	SIR65Home*					psHome;
	size						ui;
	int32						iTemporary;
	int32						iMember;
	int32						iNumTemporaries;

	for (ui = 0; ui < masHomes.NumElements(); ui++)
	{
		masHomes.Get(ui)->asRanges.Kill();
	}
	masHomes.FakeSetUsedElements(0);
	for (ui = 0; ui < IR65_HOME_FRAME; ui++)
	{
		psHome = masHomes.Add();
		psHome->asRanges.Init();
		psHome->iOffset = -1;
	}
	muiSaved = 0;
	FindChanges();

	iNumTemporaries = (int32)mpcFunction->NumTemporaries();
	maiClasses.FakeSetUsedElements(0);
	maiLeaders.FakeSetUsedElements(0);
	for (iTemporary = 0; iTemporary < iNumTemporaries; iTemporary++)
	{
		iMember = -1;
		maiClasses.Add(&iMember);
		maiLeaders.Add(&iTemporary);
	}
	Coalesce();

	asWeights.Init();
	for (iTemporary = 0; iTemporary < iNumTemporaries; iTemporary++)
	{
		if (!NeedsHome(iTemporary) || (Find(iTemporary) != iTemporary))
		{
			continue;
		}
		psWeight = asWeights.Add();
		psWeight->iLeader = iTemporary;
		psWeight->uiWeight = 0;
		for (iMember = iTemporary; iMember >= 0; iMember = *maiClasses.Get(iMember))
		{
			psWeight->uiWeight += mcLiveness.GetWeight(IRReference(IRRK_Temporary, (uint32)iMember));
		}
	}
	asWeights.QuickSort(CompareIR65Weights);
	for (ui = 0; ui < asWeights.NumElements(); ui++)
	{
		psWeight = asWeights.Get(ui);
		AllocateClass(psWeight->iLeader, psWeight->uiWeight);
	}
	asWeights.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The positions of instructions that change X and Y (and the low pseudo
// registers) behind the lowering's back: calls (and the runtime routines
// that are called), mul and div.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::FindChanges(void)
{
	size			ui;
	size			uiInstruction;
	CIRBlock*		pcBlock;
	int32			iPosition;
	uint8			uiFlags;

	maiCalls.FakeSetUsedElements(0);
	maiChangesXY.FakeSetUsedElements(0);
	maiChangesY.FakeSetUsedElements(0);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			iPosition = mcLiveness.Position(pcBlock, uiInstruction);
			uiFlags = IR65OpcodeFlags(pcBlock->masInstructions.Get(uiInstruction)->uiOpcode);
			if (uiFlags & IR65F_CALLS)
			{
				maiCalls.Add(&iPosition);
			}
			else if (uiFlags & IR65F_CHANGES_XY)
			{
				maiChangesXY.Add(&iPosition);
			}
			else if (uiFlags & IR65F_CHANGES_Y)
			{
				maiChangesY.Add(&iPosition);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Puts each phi in the class of each value it takes that needs a home, if
// they don't interfere.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Coalesce(void)
{
	size			ui;
	size			uiPhi;
	size			uiArgument;
	CIRBlock*		pcBlock;
	SIRPhi*			psPhi;
	SIRReference	sValue;
	int32			iPhi;
	int32			iValue;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiPhi = 0; uiPhi < pcBlock->masPhis.NumElements(); uiPhi++)
		{
			psPhi = pcBlock->masPhis.Get(uiPhi);
			if (!NeedsHome((int32)psPhi->sResult.Index()))
			{
				continue;
			}
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				sValue = psPhi->asArguments.Get(uiArgument)->sValue;
				if (!sValue.IsTemporary() || !NeedsHome((int32)sValue.Index()))
				{
					continue;
				}
				iPhi = Find((int32)psPhi->sResult.Index());
				iValue = Find((int32)sValue.Index());
				if ((iPhi != iValue) && !Interferes(iPhi, iValue))
				{
					Join(iPhi, iValue);
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The first temporary of iTemporary's class.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR65Backend::Find(int32 iTemporary)
{
	return *maiLeaders.Get(iTemporary);
}


//////////////////////////////////////////////////////////////////////////
//
// True if any temporary of one class is live where one of the other is.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Interferes(int32 iLeader1, int32 iLeader2)
{
	int32			iMember1;
	int32			iMember2;
	SIRReference	sMember1;
	SIRReference	sMember2;

	for (iMember1 = iLeader1; iMember1 >= 0; iMember1 = *maiClasses.Get(iMember1))
	{
		sMember1 = IRReference(IRRK_Temporary, (uint32)iMember1);
		for (iMember2 = iLeader2; iMember2 >= 0; iMember2 = *maiClasses.Get(iMember2))
		{
			sMember2 = IRReference(IRRK_Temporary, (uint32)iMember2);
			if (CIRLiveness::Intersect(mcLiveness.GetRanges(sMember1), mcLiveness.NumRanges(sMember1), mcLiveness.GetRanges(sMember2), mcLiveness.NumRanges(sMember2)))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// iLeader2's class joins the end of iLeader1's.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Join(int32 iLeader1, int32 iLeader2)
{
	int32	iMember;

	for (iMember = iLeader1; *maiClasses.Get(iMember) >= 0; iMember = *maiClasses.Get(iMember))
	{
	}
	*maiClasses.Get(iMember) = iLeader2;
	for (iMember = iLeader2; iMember >= 0; iMember = *maiClasses.Get(iMember))
	{
		*maiLeaders.Get(iMember) = iLeader1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if the class is live across an instruction at one of paiPositions
// (sorted): live both where it reads and where it writes.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Crosses(int32 iLeader, CArrayTemplate<int32>* paiPositions)
{
	int32			iMember;
	SIRReference	sMember;
	SIRRange*		psRanges;
	size			uiNumRanges;
	size			ui;
	size			uiLow;
	size			uiHigh;
	size			uiMiddle;

	if (paiPositions->NumElements() == 0)
	{
		return false;
	}
	for (iMember = iLeader; iMember >= 0; iMember = *maiClasses.Get(iMember))
	{
		sMember = IRReference(IRRK_Temporary, (uint32)iMember);
		psRanges = mcLiveness.GetRanges(sMember);
		uiNumRanges = mcLiveness.NumRanges(sMember);
		for (ui = 0; ui < uiNumRanges; ui++)
		{
			uiLow = 0;
			uiHigh = paiPositions->NumElements();
			while (uiLow < uiHigh)
			{
				uiMiddle = (uiLow + uiHigh) / 2;
				if (*paiPositions->Get(uiMiddle) < psRanges[ui].iFrom)
				{
					uiLow = uiMiddle + 1;
				}
				else
				{
					uiHigh = uiMiddle;
				}
			}
			if ((uiLow < paiPositions->NumElements()) && (*paiPositions->Get(uiLow) + 1 < psRanges[ui].iTo))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if none of the class's ranges overlap those given the home already.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::HomeFree(int32 iHome, int32 iLeader)
{
	CArrayIRRanges*	pasRanges;
	int32			iMember;
	SIRReference	sMember;
	SIRRange*		psRanges;
	size			uiNumRanges;
	size			ui;

	pasRanges = &masHomes.Get(iHome)->asRanges;
	for (iMember = iLeader; iMember >= 0; iMember = *maiClasses.Get(iMember))
	{
		sMember = IRReference(IRRK_Temporary, (uint32)iMember);
		psRanges = mcLiveness.GetRanges(sMember);
		uiNumRanges = mcLiveness.NumRanges(sMember);
		for (ui = 0; ui < uiNumRanges; ui++)
		{
			if (FindRange(pasRanges, &psRanges[ui]) < 0)
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Where psRange goes in the sorted pasRanges, or -1 if it overlaps one.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR65Backend::FindRange(CArrayIRRanges* pasRanges, SIRRange* psRange)
{
	size	uiLow;
	size	uiHigh;
	size	uiMiddle;

	uiLow = 0;
	uiHigh = pasRanges->NumElements();
	while (uiLow < uiHigh)
	{
		uiMiddle = (uiLow + uiHigh) / 2;
		if (pasRanges->Get(uiMiddle)->iTo <= psRange->iFrom)
		{
			uiLow = uiMiddle + 1;
		}
		else
		{
			uiHigh = uiMiddle;
		}
	}
	if ((uiLow < pasRanges->NumElements()) && (pasRanges->Get(uiLow)->iFrom < psRange->iTo))
	{
		return -1;
	}
	return (int32)uiLow;
}


//////////////////////////////////////////////////////////////////////////
//
// Gives a class's ranges to a home (HomeFree must be true).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::TakeHome(int32 iHome, int32 iLeader)
{
	CArrayIRRanges*	pasRanges;
	int32			iMember;
	SIRReference	sMember;
	SIRRange*		psRanges;
	size			uiNumRanges;
	size			ui;
	int32			iIndex;

	pasRanges = &masHomes.Get(iHome)->asRanges;
	for (iMember = iLeader; iMember >= 0; iMember = *maiClasses.Get(iMember))
	{
		sMember = IRReference(IRRK_Temporary, (uint32)iMember);
		psRanges = mcLiveness.GetRanges(sMember);
		uiNumRanges = mcLiveness.NumRanges(sMember);
		for (ui = 0; ui < uiNumRanges; ui++)
		{
			iIndex = FindRange(pasRanges, &psRanges[ui]);
			if (iIndex < 0)
			{
				Error("Home given twice.");
				return;
			}
			pasRanges->InsertAt(&psRanges[ui], (size)iIndex);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Sets the home of each temporary of a class.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::PlaceClass(int32 iLeader, uint8 uiKind, int32 iOffset, int32 iBytes)
{
	int32		iMember;
	SIR65Place*	psPlace;

	for (iMember = iLeader; iMember >= 0; iMember = *maiClasses.Get(iMember))
	{
		psPlace = masPlaces.Get(iMember);
		psPlace->uiKind = uiKind;
		psPlace->iOffset = iOffset;
		psPlace->iBytes = iBytes;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if iUnits homes from iHome are free for the class; takes them.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::TakeHomes(int32 iHome, int32 iUnits, int32 iLeader)
{
	int32	i;

	for (i = 0; i < iUnits; i++)
	{
		if (!HomeFree(iHome + i, iLeader))
		{
			return false;
		}
	}
	for (i = 0; i < iUnits; i++)
	{
		TakeHome(iHome + i, iLeader);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// See Allocate.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::AllocateClass(int32 iLeader, uint32 uiWeight)
{
	SIRType		sType;
	bool		bCall;
	bool		bChangesXY;
	bool		bChangesY;
	int32		iParameter;
	int32		iUnits;
	int32		iUnit;
	int32		iHome;
	int32		iOffset;
	size		ui;
	SIR65Home*	psHome;
	SIR65Place	sArrival;

	sType = mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)iLeader));
	bCall = Crosses(iLeader, &maiCalls);
	bChangesXY = bCall || Crosses(iLeader, &maiChangesXY);
	bChangesY = bChangesXY || Crosses(iLeader, &maiChangesY);
	iUnits = IsWide(sType) ? 2 : 1;

	//A parameter alone in its class (not a thiscall's this, which comes in X), whose arrival place nothing stores to
	//through an argaddr.
	iParameter = -1;
	if (*maiClasses.Get(iLeader) < 0)
	{
		for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
		{
			if ((mpcFunction->masParameters.Get(ui)->Index() == (uint32)iLeader) && !((ui == 0) && (mpcFunction->muiAttributes & IRA_ThisCall)) && !*mauiAddressed.Get(ui))
			{
				iParameter = (int32)ui;
				break;
			}
		}
	}

	//A parameter used at most once isn't worth copying.
	if ((iParameter < 0) || (uiWeight > 2))
	{
		if ((iUnits == 1) && !bChangesXY && TakeHomes(IR65R_X, 1, iLeader))
		{
			PlaceClass(iLeader, IR65P_Register, IR65R_X, 4);
			return;
		}
		if ((iUnits == 1) && !bChangesY && TakeHomes(IR65R_Y, 1, iLeader))
		{
			PlaceClass(iLeader, IR65P_Register, IR65R_Y, 4);
			return;
		}
		iUnit = bCall ? -1 : FindUnits(IR65_HOME_PSEUDO, iUnits, iLeader);
		if (iUnit >= 0)
		{
			PlaceClass(iLeader, IR65P_Pseudo, 4 * iUnit, 4 * iUnits);
			return;
		}
	}
	if (iParameter >= 0)
	{
		ArrivalPlace((size)iParameter, &sArrival);
		PlaceClass(iLeader, sArrival.uiKind, sArrival.iOffset, sArrival.iBytes);
		return;
	}
	iUnit = (uiWeight >= 8) ? FindUnits(IR65_HOME_SAVED, iUnits, iLeader) : -1;
	if (iUnit >= 0)
	{
		muiSaved |= ((1u << iUnits) - 1) << iUnit;
		PlaceClass(iLeader, IR65P_Pseudo, 128 + 4 * iUnit, 4 * iUnits);
		return;
	}

	for (iHome = IR65_HOME_FRAME; iHome + iUnits <= (int32)masHomes.NumElements(); iHome++)
	{
		if ((iUnits == 2) && (masHomes.Get(iHome + 1)->iOffset != masHomes.Get(iHome)->iOffset + 4))
		{
			continue;
		}
		if (TakeHomes(iHome, iUnits, iLeader))
		{
			PlaceClass(iLeader, IR65P_Frame, masHomes.Get(iHome)->iOffset, 4 * iUnits);
			return;
		}
	}
	iOffset = AddFrame(4 * iUnits);
	iHome = (int32)masHomes.NumElements();
	for (iUnit = 0; iUnit < iUnits; iUnit++)
	{
		psHome = masHomes.Add();
		psHome->asRanges.Init();
		psHome->iOffset = iOffset + 4 * iUnit;
	}
	TakeHomes(iHome, iUnits, iLeader);
	PlaceClass(iLeader, IR65P_Frame, iOffset, 4 * iUnits);
}


//////////////////////////////////////////////////////////////////////////
//
// The first of iUnits pseudo register units from iFirstHome (the low or the
// kept ones) free for the class, taken; -1 if none are.
//
//////////////////////////////////////////////////////////////////////////
int32 CIR65Backend::FindUnits(int32 iFirstHome, int32 iUnits, int32 iLeader)
{
	int32	iUnit;

	for (iUnit = 0; iUnit + iUnits <= IR65_PSEUDO_UNITS; iUnit++)
	{
		if (TakeHomes(iFirstHome + iUnit, iUnits, iLeader))
		{
			return iUnit;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIR65Place* CIR65Backend::PlaceOf(SIRReference sValue)
{
	return masPlaces.Get(sValue.Index());
}


//////////////////////////////////////////////////////////////////////////
//
// True if the place holds a value: a register, a pseudo register, the
// frame or a parameter where it was pushed.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IsHome(SIR65Place* psPlace)
{
	return (psPlace->uiKind == IR65P_Register) || (psPlace->uiKind == IR65P_Pseudo) || (psPlace->uiKind == IR65P_Frame) || (psPlace->uiKind == IR65P_Argument);
}


//////////////////////////////////////////////////////////////////////////
//
// True if a home is 4 bytes of memory: a pseudo register, the frame or a
// 4 byte parameter where it was pushed.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IsMemoryHome(SIR65Place* psPlace)
{
	return (psPlace->uiKind == IR65P_Pseudo) || (psPlace->uiKind == IR65P_Frame) || ((psPlace->uiKind == IR65P_Argument) && (psPlace->iBytes == 4));
}


//////////////////////////////////////////////////////////////////////////
//
// True if two temporaries have the same home.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::SamePlace(SIRReference sValue1, SIRReference sValue2)
{
	SIR65Place*	psPlace1;
	SIR65Place*	psPlace2;

	if (!sValue1.IsTemporary() || !sValue2.IsTemporary())
	{
		return false;
	}
	psPlace1 = PlaceOf(sValue1);
	psPlace2 = PlaceOf(sValue2);
	return IsHome(psPlace1) && (psPlace1->uiKind == psPlace2->uiKind) && (psPlace1->iOffset == psPlace2->iOffset) && (psPlace1->iBytes == psPlace2->iBytes);
}


//////////////////////////////////////////////////////////////////////////
//
// True if writing one home changes the other.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Overlaps(SIR65Place* psPlace1, SIR65Place* psPlace2)
{
	if (!IsHome(psPlace1) || (psPlace1->uiKind != psPlace2->uiKind))
	{
		return false;
	}
	if (psPlace1->uiKind == IR65P_Register)
	{
		return psPlace1->iOffset == psPlace2->iOffset;
	}
	return (psPlace1->iOffset < psPlace2->iOffset + psPlace2->iBytes) && (psPlace2->iOffset < psPlace1->iOffset + psPlace1->iBytes);
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue is a load the instruction before deferred (see Defer).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IsDeferred(SIRReference sValue)
{
	return sValue.IsTemporary() && msDeferred.Equals(sValue);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the instruction before left sValue in A.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IsInA(SIRReference sValue)
{
	return sValue.IsTemporary() && msAvailable.Equals(sValue);
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue's home is a register.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::InRegister(SIRReference sValue)
{
	return sValue.IsTemporary() && (PlaceOf(sValue)->uiKind == IR65P_Register);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CIR65Backend::RegisterName(SIR65Place* psPlace)
{
	return (psPlace->iOffset == IR65R_X) ? "X" : "Y";
}


//////////////////////////////////////////////////////////////////////////
//
// True if a memory operand addresses the frame (or the arguments) with V or
// S, which add, sub, cmp, shl and shr can't do.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IsFrameOperand(CChars* pszOperand)
{
	return (pszOperand->Find("V]") != (size)-1) || (pszOperand->Find("S]") != (size)-1);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the function calls nothing (not even the runtime), so nothing is
// pushed while it runs except around mul and div.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IsLeaf(void)
{
	size	ui;
	size	uiInstruction;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		for (uiInstruction = 0; uiInstruction < mpcFunction->GetBlock(ui)->masInstructions.NumElements(); uiInstruction++)
		{
			if (IR65OpcodeFlags(mpcFunction->GetBlock(ui)->masInstructions.Get(uiInstruction)->uiOpcode) & (IR65F_CALLS | IR65F_CHANGES_XY))
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A home as an operand: X, [n + W] or [n + V] ([n + S] in a function
// without a frame).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::AppendPlace(CChars* psz, SIR65Place* psPlace)
{
	switch (psPlace->uiKind)
	{
	case IR65P_Register:
		psz->Append(RegisterName(psPlace));
		break;
	case IR65P_Pseudo:
		psz->Append('[')->Append(psPlace->iOffset)->Append(" + W]");
		break;
	case IR65P_Frame:
	case IR65P_FrameAddress:
		psz->Append('[')->Append(1 + psPlace->iOffset)->Append(" + ")->Append(mcFrame)->Append(']');
		break;
	case IR65P_Argument:
	case IR65P_ArgumentAddress:
		psz->Append('[')->Append(miArguments + psPlace->iOffset)->Append(" + ")->Append(mcFrame)->Append(']');
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A constant as an immediate, extended to 32 bits as eExtend says.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::AppendImmediate(CChars* psz, SIRReference sConstant, EIR65Extend eExtend)
{
	SIRConstant*	psConstant;
	int64			lliValue;

	psConstant = mpcFunction->GetConstant(sConstant);
	lliValue = (eExtend == IR65E_Sign) ? IRSignExtend(psConstant->sType, psConstant->ulliBits) : (int64)psConstant->ulliBits;
	psz->Append((int64)(int32)lliValue);
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue can be the second operand of an instruction as it is:
// a 32 bit (or narrower) constant, a symbol's address, A if the
// instruction before left it there, its register or pseudo register, or
// its frame place unless the instruction is bArithmetic (add, sub, cmp,
// shl and shr can't address memory with V).  Appends the operand.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Operand(SIRReference sValue, CChars* pszOperand, bool bArithmetic, EIR65Extend eExtend)
{
	SIRType		sType;
	SIR65Place*	psPlace;

	if (IsDeferred(sValue))
	{
		//add, sub, cmp, shl and shr can't address memory with V.
		if (bArithmetic && IsFrameOperand(&mszDeferred))
		{
			return false;
		}
		pszOperand->Append(mszDeferred);
		return true;
	}
	sType = mpcFunction->TypeOf(sValue);
	if (IsWide(sType))
	{
		return false;
	}
	if (sValue.IsConstant())
	{
		AppendImmediate(pszOperand, sValue, eExtend);
		return true;
	}
	if (sValue.IsSymbol())
	{
		pszOperand->Append(mpcModule->GetSymbolName(sValue.Index()));
		return true;
	}
	if (!sValue.IsTemporary() || ((eExtend == IR65E_Sign) && IsNarrow(sType)))
	{
		return false;
	}
	if (IsInA(sValue))
	{
		pszOperand->Append('A');
		return true;
	}
	psPlace = PlaceOf(sValue);
	switch (psPlace->uiKind)
	{
	case IR65P_Register:
	case IR65P_Pseudo:
		AppendPlace(pszOperand, psPlace);
		return true;
	case IR65P_Frame:
		if (bArithmetic)
		{
			return false;
		}
		AppendPlace(pszOperand, psPlace);
		return true;
	case IR65P_Argument:
		if (bArithmetic || (psPlace->iBytes != 4))
		{
			return false;
		}
		AppendPlace(pszOperand, psPlace);
		return true;
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue is a constant from iMin to iMax (as held: narrow values
// zero extended), which an instruction can use with memory.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::SmallImmediate(SIRReference sValue, int iMin, int iMax)
{
	SIRConstant*	psConstant;
	int64			lliValue;

	if (!sValue.IsConstant())
	{
		return false;
	}
	psConstant = mpcFunction->GetConstant(sValue);
	if (IsWide(psConstant->sType))
	{
		return false;
	}
	lliValue = IsNarrow(psConstant->sType) ? (int64)psConstant->ulliBits : (int64)(int32)psConstant->ulliBits;
	return (lliValue >= iMin) && (lliValue <= iMax);
}


//////////////////////////////////////////////////////////////////////////
//
// The register to work out an instruction's result in: its own if it is
// kept in X or Y, otherwise A.
//
//////////////////////////////////////////////////////////////////////////
const char* CIR65Backend::WorkRegister(SIRInstruction* psInstruction)
{
	SIR65Place*	psPlace;

	if (!psInstruction->sResult.IsTemporary() || IsWide(psInstruction->sType))
	{
		return "A";
	}
	psPlace = PlaceOf(psInstruction->sResult);
	return (psPlace->uiKind == IR65P_Register) ? RegisterName(psPlace) : "A";
}


//////////////////////////////////////////////////////////////////////////
//
// Sign extends a narrow integer held zero extended (flip and subtract the
// sign bit), when eExtend says so.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Extend(const char* szRegister, SIRType sType, EIR65Extend eExtend)
{
	int		iBits;
	int64	lliSign;

	iBits = sType.IntegerBits();
	if ((eExtend == IR65E_Sign) && (iBits >= 1) && (iBits < 32))
	{
		lliSign = (int64)1 << (iBits - 1);
		Line("xor ")->Append(szRegister)->Append(", ")->Append(lliSign)->AppendNewLine();
		Line("sub ")->Append(szRegister)->Append(", ")->Append(lliSign)->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps the low bits of a narrow integer, zero extended.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Truncate(const char* szRegister, SIRType sType)
{
	if (IsNarrow(sType))
	{
		Line("and ")->Append(szRegister)->Append(", ")->Append((((int64)1) << sType.IntegerBits()) - 1)->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::LoadConstant(const char* szRegister, SIRType sType, uint64 ulliBits, EIR65Extend eExtend)
{
	int64	lliValue;

	if (IsWide(sType))
	{
		Line("wth ")->Append(szRegister)->Append(", 64")->AppendNewLine();
		Line("cpy ")->Append(szRegister)->Append(", ")->Append((int64)ulliBits)->AppendNewLine();
		Line("wth ")->Append(szRegister)->Append(", 32")->AppendNewLine();
		return;
	}
	lliValue = (eExtend == IR65E_Sign) ? IRSignExtend(sType, ulliBits) : (int64)ulliBits;
	Line("cpy ")->Append(szRegister)->Append(", ")->Append((int64)(int32)lliValue)->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Loads a home into a register, narrow integers extended to 32 bits as
// eExtend says, 64 bit values with the register 64 bits wide (it is left 32
// bits wide, holding all 64).  A parameter pushed narrow is read at its own
// width and widened.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::LoadPlace(SIR65Place* psPlace, SIRType sType, const char* szRegister, EIR65Extend eExtend)
{
	CChars*	psz;

	if (psPlace->uiKind == IR65P_Register)
	{
		if (StringCompare(RegisterName(psPlace), szRegister) != 0)
		{
			Line("cpy ")->Append(szRegister)->Append(", ")->Append(RegisterName(psPlace))->AppendNewLine();
		}
		Extend(szRegister, sType, eExtend);
		return;
	}
	if (IsWide(sType))
	{
		Line("wth ")->Append(szRegister)->Append(", 64")->AppendNewLine();
		psz = Line("cpy ")->Append(szRegister)->Append(", ");
		AppendPlace(psz, psPlace);
		psz->AppendNewLine();
		Line("wth ")->Append(szRegister)->Append(", 32")->AppendNewLine();
		return;
	}
	if ((psPlace->uiKind == IR65P_Argument) && (psPlace->iBytes < 4))
	{
		Line("wth ")->Append(szRegister)->Append(", ")->Append(8 * psPlace->iBytes)->AppendNewLine();
		psz = Line("cpy ")->Append(szRegister)->Append(", ");
		AppendPlace(psz, psPlace);
		psz->AppendNewLine();
		Line("wthx ")->Append(szRegister)->Append(((eExtend == IR65E_Sign) && IsNarrow(sType)) ? ", 32:s" : ", 32")->AppendNewLine();
		return;
	}
	psz = Line("cpy ")->Append(szRegister)->Append(", ");
	AppendPlace(psz, psPlace);
	psz->AppendNewLine();
	Extend(szRegister, sType, eExtend);
}


//////////////////////////////////////////////////////////////////////////
//
// Stores a register to a home.  Narrow values are already zero extended.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::StorePlace(SIR65Place* psPlace, SIRType sType, const char* szRegister)
{
	CChars*	psz;

	if (psPlace->uiKind == IR65P_Register)
	{
		if (StringCompare(RegisterName(psPlace), szRegister) != 0)
		{
			Line("cpy ")->Append(RegisterName(psPlace))->Append(", ")->Append(szRegister)->AppendNewLine();
		}
		return;
	}
	if (IsWide(sType))
	{
		Line("wth ")->Append(szRegister)->Append(", 64")->AppendNewLine();
	}
	psz = Line("cpy ");
	AppendPlace(psz, psPlace);
	psz->Append(", ")->Append(szRegister)->AppendNewLine();
	if (IsWide(sType))
	{
		Line("wth ")->Append(szRegister)->Append(", 32")->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Loads a value into a register.  Only a result's own register is loaded
// in place (its operand dies there, so extending it is fine).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::LoadValue(SIRReference sValue, const char* szRegister, EIR65Extend eExtend)
{
	SIR65Place*	psPlace;
	bool		bA;

	bA = StringCompare(szRegister, "A") == 0;
	if (IsDeferred(sValue))
	{
		if (bA)
		{
			msAvailable = IRNone();
		}
		Line("cpy ")->Append(szRegister)->Append(", ")->Append(mszDeferred)->AppendNewLine();
		return;
	}
	if (IsInA(sValue))
	{
		if (!bA && IsWide(mpcFunction->TypeOf(sValue)))
		{
			Line("wtth (A, ")->Append(szRegister)->Append("), 64")->AppendNewLine();
			Line("cpy ")->Append(szRegister)->Append(", A")->AppendNewLine();
			Line("wtth (A, ")->Append(szRegister)->Append("), 32")->AppendNewLine();
			return;
		}
		if (!bA)
		{
			Line("cpy ")->Append(szRegister)->Append(", A")->AppendNewLine();
		}
		else if ((eExtend == IR65E_Sign) && IsNarrow(mpcFunction->TypeOf(sValue)))
		{
			msAvailable = IRNone();
		}
		Extend(szRegister, mpcFunction->TypeOf(sValue), eExtend);
		return;
	}
	if (bA)
	{
		msAvailable = IRNone();
	}

	switch (sValue.Kind())
	{
	case IRRK_Constant:
		LoadConstant(szRegister, mpcFunction->GetConstant(sValue)->sType, mpcFunction->GetConstant(sValue)->ulliBits, eExtend);
		return;
	case IRRK_Symbol:
		Line("cpy ")->Append(szRegister)->Append(", ")->Append(mpcModule->GetSymbolName(sValue.Index()))->AppendNewLine();
		return;
	case IRRK_Temporary:
		break;
	default:
		Error("Operand can't be loaded.");
		return;
	}

	psPlace = PlaceOf(sValue);
	switch (psPlace->uiKind)
	{
	case IR65P_FrameAddress:
	case IR65P_ArgumentAddress:
		Line("cpy ")->Append(szRegister)->Append(", ")->Append(mcFrame)->AppendNewLine();
		Line("add ")->Append(szRegister)->Append(", ")->Append(((psPlace->uiKind == IR65P_ArgumentAddress) ? miArguments : 1) + psPlace->iOffset)->AppendNewLine();
		return;
	case IR65P_Register:
	case IR65P_Pseudo:
	case IR65P_Frame:
	case IR65P_Argument:
		LoadPlace(psPlace, mpcFunction->GetTemporaryType(sValue), szRegister, eExtend);
		return;
	default:
		Error("Temporary isn't where it was left.");
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps a result worked out in szRegister (truncated if narrow, unless
// bExtended says it is already) in its home, or in A for the next
// instruction.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::StoreResult(SIRReference sResult, const char* szRegister, bool bExtended)
{
	SIR65Place*	psPlace;
	SIRType		sType;
	bool		bA;

	if (!sResult.IsTemporary())
	{
		return;
	}
	psPlace = PlaceOf(sResult);
	sType = mpcFunction->GetTemporaryType(sResult);
	bA = StringCompare(szRegister, "A") == 0;
	if (psPlace->bInA)
	{
		if (!bA)
		{
			Line("cpy A, ")->Append(szRegister)->AppendNewLine();
		}
		if (!bExtended)
		{
			Truncate("A", sType);
		}
		msInA = sResult;
		return;
	}
	if (psPlace->uiKind == IR65P_None)
	{
		//Not used.
		return;
	}
	if ((psPlace->uiKind != IR65P_Register) && (psPlace->uiKind != IR65P_Pseudo) && (psPlace->uiKind != IR65P_Frame))
	{
		Error("Result has no home.");
		return;
	}
	if (!bExtended)
	{
		Truncate(szRegister, sType);
	}
	StorePlace(psPlace, sType, szRegister);
	if (bA)
	{
		msInA = sResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The memory operand for an address: [symbol], [n + V] for a slot or an
// argument, [X] for an address kept in a register, [[n + W]] or [[n + V]]
// for one kept in memory, otherwise [B].  szWidened is the register the
// access narrows or widens (an address in it is copied to B first).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::MemoryOperand(SIRReference sAddress, CChars* pszOperand, const char* szWidened)
{
	SIR65Place*	psPlace;

	if (sAddress.IsSymbol())
	{
		pszOperand->Append('[')->Append(mpcModule->GetSymbolName(sAddress.Index()))->Append(']');
		return;
	}
	if (sAddress.IsConstant())
	{
		pszOperand->Append('[')->Append((int64)(uint32)mpcFunction->GetConstant(sAddress)->ulliBits)->Append(']');
		return;
	}
	if (sAddress.IsTemporary() && (PlaceOf(sAddress)->uiKind == IR65P_Folded))
	{
		FoldedOperand(PlaceOf(sAddress), pszOperand, szWidened);
		return;
	}
	if (sAddress.IsTemporary())
	{
		if (IsInA(sAddress) && !InRegister(sAddress))
		{
			if ((szWidened == NULL) || (StringCompare(szWidened, "A") != 0))
			{
				pszOperand->Append("[A]");
				return;
			}
		}
		else
		{
			psPlace = PlaceOf(sAddress);
			switch (psPlace->uiKind)
			{
			case IR65P_FrameAddress:
			case IR65P_ArgumentAddress:
				AppendPlace(pszOperand, psPlace);
				return;
			case IR65P_Register:
				if ((szWidened == NULL) || (StringCompare(szWidened, RegisterName(psPlace)) != 0))
				{
					pszOperand->Append('[')->Append(RegisterName(psPlace))->Append(']');
					return;
				}
				break;
			default:
				if (IsMemoryHome(psPlace))
				{
					pszOperand->Append('[');
					AppendPlace(pszOperand, psPlace);
					pszOperand->Append(']');
					return;
				}
				break;
			}
		}
	}
	LoadValue(sAddress, "B", IR65E_Zero);
	pszOperand->Append("[B]");
}


//////////////////////////////////////////////////////////////////////////
//
// A folded ptradd's address: [symbol + offset], [n + V] for a slot or an
// argument, [offset + R] with the base in a register (not szWidened) or B.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::FoldedOperand(SIR65Place* psFolded, CChars* pszOperand, const char* szWidened)
{
	SIRReference	sBase;
	SIR65Place*		psBase;
	SIR65Place		sAddress;
	const char*		szRegister;

	sBase = psFolded->sBase;
	if (sBase.IsSymbol())
	{
		pszOperand->Append('[')->Append(mpcModule->GetSymbolName(sBase.Index()));
		if (psFolded->iOffset != 0)
		{
			pszOperand->Append((psFolded->iOffset < 0) ? " - " : " + ")->Append((int64)((psFolded->iOffset < 0) ? -(int64)psFolded->iOffset : psFolded->iOffset));
		}
		pszOperand->Append(']');
		return;
	}
	if (sBase.IsConstant())
	{
		pszOperand->Append('[')->Append((int64)(uint32)(mpcFunction->GetConstant(sBase)->ulliBits + psFolded->iOffset))->Append(']');
		return;
	}

	szRegister = NULL;
	if (IsInA(sBase))
	{
		szRegister = "A";
	}
	else if (sBase.IsTemporary())
	{
		psBase = PlaceOf(sBase);
		if ((psBase->uiKind == IR65P_FrameAddress) || (psBase->uiKind == IR65P_ArgumentAddress))
		{
			sAddress = *psBase;
			sAddress.iOffset += psFolded->iOffset;
			AppendPlace(pszOperand, &sAddress);
			return;
		}
		if (psBase->uiKind == IR65P_Register)
		{
			szRegister = RegisterName(psBase);
		}
	}
	if ((szRegister == NULL) || (szWidened && (StringCompare(szRegister, szWidened) == 0)))
	{
		LoadValue(sBase, "B", IR65E_Zero);
		szRegister = "B";
	}
	pszOperand->Append('[')->Append(psFolded->iOffset)->Append(" + ")->Append(szRegister)->Append(']');
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes a value at the size of sType: straight from its register, memory
// or as an immediate when it can be, otherwise from A.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Push(SIRReference sValue, SIRType sType)
{
	int			iBytes;
	CChars		szOperand;
	SIR65Place*	psPlace;
	const char*	szRegister;

	iBytes = ValueBytes(sType);
	if (iBytes == 8)
	{
		LoadValue(sValue, "A", IR65E_Zero);
		Line("wth A, 64")->AppendNewLine();
		Line("psh A")->AppendNewLine();
		Line("wth A, 32")->AppendNewLine();
		return;
	}
	if (sValue.IsConstant())
	{
		szOperand.Init();
		AppendImmediate(&szOperand, sValue, IR65E_Zero);
		Line("psh ")->Append(szOperand)->Append(':')->Append(8 * iBytes)->AppendNewLine();
		szOperand.Kill();
		return;
	}

	szRegister = NULL;
	if (IsInA(sValue))
	{
		szRegister = "A";
	}
	else if (InRegister(sValue))
	{
		szRegister = RegisterName(PlaceOf(sValue));
	}
	else if ((iBytes == 4) && sValue.IsTemporary())
	{
		psPlace = PlaceOf(sValue);
		if (IsMemoryHome(psPlace))
		{
			szOperand.Init();
			AppendPlace(&szOperand, psPlace);
			Line("psh ")->Append(szOperand)->AppendNewLine();
			szOperand.Kill();
			return;
		}
	}
	if (szRegister == NULL)
	{
		LoadValue(sValue, "A", IR65E_Zero);
		szRegister = "A";
	}
	if (iBytes == 4)
	{
		Line("psh ")->Append(szRegister)->AppendNewLine();
		return;
	}
	Line("wth ")->Append(szRegister)->Append(", ")->Append(8 * iBytes)->AppendNewLine();
	Line("psh ")->Append(szRegister)->AppendNewLine();
	Line("wth ")->Append(szRegister)->Append(", 32")->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// mul and div count their steps in W, from 0, so W is kept on the stack.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Multiply(const char* szInstruction)
{
	Line("psh W")->AppendNewLine();
	Line("cpy W, 0")->AppendNewLine();
	Line(szInstruction)->AppendNewLine();
	Line("pul W")->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// True if going from pcFrom to pcTo copies values to pcTo's phis (a phi
// that shares its home with the value it takes needs no copy).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::HasMoves(CIRBlock* pcFrom, CIRBlock* pcTo)
{
	size			ui;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;

	for (ui = 0; ui < pcTo->masPhis.NumElements(); ui++)
	{
		psPhi = pcTo->masPhis.Get(ui);
		psArgument = pcTo->FindPhiArgument(psPhi, pcFrom);
		if (psArgument && (PlaceOf(psPhi->sResult)->uiKind != IR65P_None) && !SamePlace(psArgument->sValue, psPhi->sResult))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if an instruction writes nothing: a folded ptradd, slots, argaddrs,
// args (the call pushes them) and instructions without effects whose
// results aren't used.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Skipped(SIRInstruction* psInstruction)
{
	switch (psInstruction->uiOpcode)
	{
	case IRO_Slot:
	case IRO_ArgAddress:
	case IRO_Arg:
		return true;
	case IRO_Call:
	case IRO_Store:
		return false;
	case IRO_Load:
		if (psInstruction->uiFlags & IRF_Volatile)
		{
			return false;
		}
		break;
	}
	if (!psInstruction->sResult.IsTemporary())
	{
		return false;
	}
	return (PlaceOf(psInstruction->sResult)->uiKind == IR65P_Folded) || (*mauiUses.Get(psInstruction->sResult.Index()) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The first instruction after uiAfter (before uiEnd) that writes something
// (see Skipped); uiEnd if none does.
//
//////////////////////////////////////////////////////////////////////////
size CIR65Backend::NextWritten(CIRBlock* pcBlock, size uiAfter, size uiEnd)
{
	size	uiNext;

	for (uiNext = uiAfter + 1; (uiNext < uiEnd) && Skipped(pcBlock->masInstructions.Get(uiNext)); uiNext++)
	{
	}
	return uiNext;
}


//////////////////////////////////////////////////////////////////////////
//
// A block: its instructions and its jump.  A comparison used only by the
// block's brif branches directly.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Block(CIRBlock* pcBlock, CIRBlock* pcNext)
{
	size			ui;
	size			uiNumInstructions;
	SIRInstruction*	psFused;
	SIRInstruction*	psInstruction;
	SIRReference	sDeferred;
	bool			bResult;

	msAvailable = IRNone();
	msInA = IRNone();
	msDeferred = IRNone();
	mpsWritten = NULL;
	Label(*maiBlockLabels.Get(pcBlock->muiIndex));

	psFused = FusedComparison(pcBlock);
	uiNumInstructions = pcBlock->masInstructions.NumElements();
	if (psFused)
	{
		uiNumInstructions--;
	}
	mbFlagsKnown = (uiNumInstructions == 0) && FlagsKnown(pcBlock, psFused);

	bResult = true;
	for (ui = 0; ui < uiNumInstructions; ui++)
	{
		psInstruction = pcBlock->masInstructions.Get(ui);
		if (Skipped(psInstruction) || (psInstruction == mpsWritten))
		{
			//Writes nothing (or was written with the instruction before), so A keeps what it had.
			continue;
		}
		msAvailable = msInA;
		msInA = IRNone();
		sDeferred = msDeferred;
		bResult &= Instruction(pcBlock, ui);
		if (!sDeferred.IsNone())
		{
			//The instruction after a deferred load has used it.
			msDeferred = IRNone();
		}
	}
	msAvailable = msInA;
	msInA = IRNone();
	bResult &= Jump(pcBlock, pcNext, psFused);
	mbFlagsKnown = false;
	mpsFlags = (psFused && !psFused->sType.IsFloat()) ? psFused : NULL;
	mpcFlags = pcBlock;
	msAvailable = IRNone();
	msDeferred = IRNone();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the flags pcBlock is entered with already give psFused, an eq or
// ne whose block has nothing else in it: its only predecessor's jump
// compared the same operands (cmp leaves z set when they are equal) and
// nothing was copied on the way.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::FlagsKnown(CIRBlock* pcBlock, SIRInstruction* psFused)
{
	uint16	uiOpcode;

	if ((psFused == NULL) || (mpsFlags == NULL) || ((psFused->uiOpcode != IRO_Eq) && (psFused->uiOpcode != IRO_Ne)) || (pcBlock->masPhis.NumElements() != 0) ||
		(pcBlock->mapcPredecessors.NumElements() != 1) || (pcBlock->mapcPredecessors.GetPtr(0) != mpcFlags) || HasMoves(mpcFlags, pcBlock) || !psFused->sType.Equals(mpsFlags->sType))
	{
		return false;
	}

	//x > c and x <= c compare x with c + 1.
	uiOpcode = mpsFlags->uiOpcode;
	if (mpsFlags->asArguments[1].IsConstant() && ((uiOpcode == IRO_SGt) || (uiOpcode == IRO_SLe) || (uiOpcode == IRO_UGt) || (uiOpcode == IRO_ULe)))
	{
		return false;
	}
	return (psFused->asArguments[0].Equals(mpsFlags->asArguments[0]) && psFused->asArguments[1].Equals(mpsFlags->asArguments[1])) ||
		   (psFused->asArguments[0].Equals(mpsFlags->asArguments[1]) && psFused->asArguments[1].Equals(mpsFlags->asArguments[0]));
}


//////////////////////////////////////////////////////////////////////////
//
// The copies pcTo's phis need from pcFrom, as one parallel copy: a copy is
// made once nothing still to be copied reads its destination; when only
// cycles are left, one value is copied to B and read from there.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Edge(CIRBlock* pcFrom, CIRBlock* pcTo)
{
	size			ui;
	size			uiOther;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	SIR65Move*		psMove;
	SIR65Move*		psOther;
	SIR65Place*		psDestination;
	SIRReference	sCycle;
	bool			bBlocked;
	bool			bProgress;
	bool			bLeft;

	masMoves.FakeSetUsedElements(0);
	for (ui = 0; ui < pcTo->masPhis.NumElements(); ui++)
	{
		psPhi = pcTo->masPhis.Get(ui);
		psArgument = pcTo->FindPhiArgument(psPhi, pcFrom);
		if (psArgument == NULL)
		{
			Error("Phi has no argument for a predecessor.");
			continue;
		}
		if ((PlaceOf(psPhi->sResult)->uiKind == IR65P_None) || SamePlace(psArgument->sValue, psPhi->sResult))
		{
			continue;
		}
		psMove = masMoves.Add();
		psMove->sDestination = psPhi->sResult;
		psMove->sSource = psArgument->sValue;
		psMove->sType = psPhi->sType;
		psMove->bFromB = false;
	}

	for (;;)
	{
		bProgress = false;
		bLeft = false;
		for (ui = 0; ui < masMoves.NumElements(); ui++)
		{
			psMove = masMoves.Get(ui);
			if (psMove->sSource.IsNone())
			{
				continue;
			}
			bLeft = true;
			psDestination = PlaceOf(psMove->sDestination);
			bBlocked = false;
			for (uiOther = 0; uiOther < masMoves.NumElements(); uiOther++)
			{
				psOther = masMoves.Get(uiOther);
				if ((uiOther != ui) && !psOther->bFromB && psOther->sSource.IsTemporary() && Overlaps(PlaceOf(psOther->sSource), psDestination))
				{
					bBlocked = true;
					break;
				}
			}
			if (!bBlocked)
			{
				Move(psMove->sDestination, psMove->sSource, psMove->sType, psMove->bFromB);
				psMove->sSource = IRNone();
				bProgress = true;
			}
		}
		if (!bLeft)
		{
			break;
		}
		if (!bProgress)
		{
			//Only cycles: the value in the first copy's destination goes to B.
			for (ui = 0; ui < masMoves.NumElements(); ui++)
			{
				psMove = masMoves.Get(ui);
				if (!psMove->sSource.IsNone())
				{
					break;
				}
			}
			psDestination = PlaceOf(psMove->sDestination);
			sCycle = IRNone();
			for (uiOther = 0; uiOther < masMoves.NumElements(); uiOther++)
			{
				psOther = masMoves.Get(uiOther);
				if (!psOther->sSource.IsNone() && psOther->sSource.IsTemporary() && Overlaps(PlaceOf(psOther->sSource), psDestination))
				{
					if (sCycle.IsNone())
					{
						sCycle = psOther->sSource;
						LoadValue(sCycle, "B", IR65E_Zero);
					}
					if (SamePlace(psOther->sSource, sCycle))
					{
						psOther->bFromB = true;
					}
					else
					{
						Error("Phi copies overlap.");
						return;
					}
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// One phi copy: into a register, from a register or a small constant
// straight to memory, otherwise through A (or from B, for a cycle).
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Move(SIRReference sDestination, SIRReference sSource, SIRType sType, bool bFromB)
{
	SIR65Place*	psDestination;
	CChars*		psz;

	psDestination = PlaceOf(sDestination);
	if (bFromB)
	{
		StorePlace(psDestination, sType, "B");
	}
	else if (psDestination->uiKind == IR65P_Register)
	{
		LoadValue(sSource, RegisterName(psDestination), IR65E_Zero);
	}
	else if (InRegister(sSource) && !IsWide(sType))
	{
		StorePlace(psDestination, sType, RegisterName(PlaceOf(sSource)));
	}
	else if (SmallImmediate(sSource, -8, 8))
	{
		psz = Line("cpy ");
		AppendPlace(psz, psDestination);
		psz->Append(", ");
		AppendImmediate(psz, sSource, IR65E_Zero);
		psz->AppendNewLine();
	}
	else
	{
		LoadValue(sSource, "A", IR65E_Zero);
		StorePlace(psDestination, sType, "A");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Sets z from a value: cmp with 0 where it is kept, or in A.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::TestZero(SIRReference sValue)
{
	CChars*		psz;
	SIR65Place*	psPlace;

	if (sValue.IsTemporary() && !IsInA(sValue))
	{
		psPlace = PlaceOf(sValue);
		if ((psPlace->uiKind == IR65P_Register) || (psPlace->uiKind == IR65P_Pseudo))
		{
			psz = Line("cmp ");
			AppendPlace(psz, psPlace);
			psz->Append(", 0")->AppendNewLine();
			return;
		}
	}
	LoadValue(sValue, "A", IR65E_Zero);
	Line("cmp A, 0")->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// The block's jump.  A branch tests its condition (or does psFused's
// comparison) and goes to the else block, a target's phis getting their
// values on the way; the then block is fallen through to when it is next.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Jump(CIRBlock* pcBlock, CIRBlock* pcNext, SIRInstruction* psFused)
{
	SIRJump*	psJump;
	int32		iElse;
	bool		bResult;
	SIR65Stub*	psStub;

	psJump = &pcBlock->msJump;
	bResult = true;
	switch (psJump->uiKind)
	{
	case IRJ_Jump:
		Edge(pcBlock, psJump->pcThen);
		if (psJump->pcThen != pcNext)
		{
			Branch("bra ", *maiBlockLabels.Get(psJump->pcThen->muiIndex));
		}
		break;

	case IRJ_Branch:
		if ((psJump->pcElse == pcNext) && (psJump->pcThen != pcNext) && HasMoves(pcBlock, psJump->pcThen) && !HasMoves(pcBlock, psJump->pcElse))
		{
			//The then edge's copies go after the function's ret (a loop that goes round when the test is true).
			psStub = masStubs.Add();
			psStub->iLabel = NewLabel();
			psStub->pcFrom = pcBlock;
			psStub->pcTo = psJump->pcThen;
			if (psFused)
			{
				bResult = Compare(psFused, true, psStub->iLabel);
			}
			else
			{
				TestZero(psJump->sArgument);
				Branch("brac z, ", psStub->iLabel);
			}
			break;
		}
		if ((psJump->pcElse == pcNext) && (psJump->pcThen != pcNext) && !HasMoves(pcBlock, psJump->pcThen))
		{
			//Branch to the then block, fall through to the else block.
			if (psFused)
			{
				bResult = Compare(psFused, true, *maiBlockLabels.Get(psJump->pcThen->muiIndex));
			}
			else
			{
				TestZero(psJump->sArgument);
				Branch("brac z, ", *maiBlockLabels.Get(psJump->pcThen->muiIndex));
			}
			Edge(pcBlock, psJump->pcElse);
			break;
		}
		iElse = HasMoves(pcBlock, psJump->pcElse) ? NewLabel() : *maiBlockLabels.Get(psJump->pcElse->muiIndex);
		if (psFused)
		{
			bResult = Compare(psFused, false, iElse);
		}
		else
		{
			TestZero(psJump->sArgument);
			Branch("bras z, ", iElse);
		}
		Edge(pcBlock, psJump->pcThen);
		if ((psJump->pcThen != pcNext) || HasMoves(pcBlock, psJump->pcElse))
		{
			Branch("bra ", *maiBlockLabels.Get(psJump->pcThen->muiIndex));
		}
		if (HasMoves(pcBlock, psJump->pcElse))
		{
			Label(iElse);
			Edge(pcBlock, psJump->pcElse);
			Branch("bra ", *maiBlockLabels.Get(psJump->pcElse->muiIndex));
		}
		break;

	case IRJ_Return:
		if (!psJump->sType.IsNone())
		{
			LoadValue(psJump->sArgument, "A", IR65E_Zero);
		}
		if (pcNext != NULL)
		{
			Branch("bra ", miReturnLabel);
		}
		break;

	case IRJ_Unreachable:
		Line("stp")->AppendNewLine();
		break;

	default:
		return Error("Block has no jump.");
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Instruction(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	int32			iTrue;
	int32			iEnd;
	const char*		szWork;

	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	switch (psInstruction->uiOpcode)
	{
	case IRO_Add:
	case IRO_Sub:
	case IRO_And:
	case IRO_Or:
	case IRO_Xor:
	case IRO_Shl:
	case IRO_LShr:
		if (ReadModifyWrite(pcBlock, uiIndex))
		{
			return true;
		}
		return Integer(psInstruction);

	case IRO_Neg:
	case IRO_Not:
	case IRO_Copy:
	case IRO_PtrAdd:
		return Integer(psInstruction);

	case IRO_Mul:
		return MultiplyInstruction(psInstruction);

	case IRO_AShr:
		return ShiftSigned(psInstruction);

	case IRO_SDiv:
	case IRO_UDiv:
	case IRO_SRem:
	case IRO_URem:
		return Divide(psInstruction);

	case IRO_SExt:
	case IRO_ZExt:
	case IRO_Trunc:
	case IRO_PtrToI:
	case IRO_IToPtr:
	case IRO_SpaceCast:
		return Conversion(psInstruction);

	case IRO_Eq:
	case IRO_Ne:
	case IRO_SLt:
	case IRO_SLe:
	case IRO_SGt:
	case IRO_SGe:
	case IRO_ULt:
	case IRO_ULe:
	case IRO_UGt:
	case IRO_UGe:
	case IRO_FEq:
	case IRO_FNe:
	case IRO_FLt:
	case IRO_FLe:
	case IRO_FGt:
	case IRO_FGe:
	case IRO_FOrdered:
	case IRO_FUnordered:
		//The i1: 1 when the comparison is true.
		iTrue = NewLabel();
		iEnd = NewLabel();
		if (!Compare(psInstruction, true, iTrue))
		{
			return false;
		}
		szWork = WorkRegister(psInstruction);
		if (StringCompare(szWork, "A") == 0)
		{
			msAvailable = IRNone();
		}
		Line("cpy ")->Append(szWork)->Append(", 0")->AppendNewLine();
		Branch("bra ", iEnd);
		Label(iTrue);
		Line("cpy ")->Append(szWork)->Append(", 1")->AppendNewLine();
		Label(iEnd);
		StoreResult(psInstruction->sResult, szWork);
		return true;

	case IRO_FAdd:
	case IRO_FSub:
	case IRO_FMul:
	case IRO_FDiv:
	case IRO_FNeg:
		return FloatOperation(psInstruction);

	case IRO_SIToF:
	case IRO_UIToF:
	case IRO_FToSI:
	case IRO_FToUI:
	case IRO_FExt:
	case IRO_FTrunc:
		return FloatConversion(psInstruction);

	case IRO_Slot:
	case IRO_ArgAddress:
		return true;

	case IRO_Load:
		return Load(pcBlock, uiIndex);
	case IRO_Store:
		return Store(psInstruction);
	case IRO_Arg:
		return true;
	case IRO_Call:
		return Call(pcBlock, uiIndex);

	default:
		return Error("Opcode not supported by the 65C64 back end.", GetIROpcode(psInstruction->uiOpcode) ? GetIROpcode(psInstruction->uiOpcode)->szName : NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Commutes(uint16 uiOpcode)
{
	return (IR65OpcodeFlags(uiOpcode) & IR65F_COMMUTES) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A deferred load, an operation on it and a store of the result back to the
// same place is one instruction on memory: add [4 + X], 1 (the operand a
// register or -8 to 8, 1 to 8 for a shift).  The store is then not written.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::ReadModifyWrite(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	SIRInstruction*	psStore;
	SIRReference	sMemory;
	SIRReference	sOther;
	size			uiNext;
	CChars			szAddress;
	CChars			szOperand;
	char			cRegister;
	bool			bShift;
	bool			bResult;
	CChars*			psz;

	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	bShift = (psInstruction->uiOpcode == IRO_Shl) || (psInstruction->uiOpcode == IRO_LShr);
	if (IsNarrow(psInstruction->sType) || IsWide(psInstruction->sType) || !psInstruction->sResult.IsTemporary() || !PlaceOf(psInstruction->sResult)->bInA)
	{
		return false;
	}
	if (IsDeferred(psInstruction->asArguments[0]))
	{
		sMemory = psInstruction->asArguments[0];
		sOther = psInstruction->asArguments[1];
	}
	else if (IsDeferred(psInstruction->asArguments[1]) && Commutes(psInstruction->uiOpcode))
	{
		sMemory = psInstruction->asArguments[1];
		sOther = psInstruction->asArguments[0];
	}
	else
	{
		return false;
	}

	uiNext = NextWritten(pcBlock, uiIndex, pcBlock->masInstructions.NumElements());
	psStore = pcBlock->masInstructions.SafeGet(uiNext);
	if ((psStore == NULL) || (psStore->uiOpcode != IRO_Store) || !psStore->asArguments[0].Equals(psInstruction->sResult) || (psStore->uiFlags & IRF_Volatile) || !psStore->sType.Equals(psInstruction->sType))
	{
		return false;
	}

	szOperand.Init();
	if (IsInA(sOther))
	{
		szOperand.Append('A');
	}
	else if (InRegister(sOther) && !bShift)
	{
		//A shift of memory takes its amount from A or B.
		szOperand.Append(RegisterName(PlaceOf(sOther)));
	}
	else if (SmallImmediate(sOther, bShift ? 1 : -8, 8))
	{
		AppendImmediate(&szOperand, sOther, IR65E_Zero);
	}
	szAddress.Init();
	cRegister = '\0';
	bResult = !szOperand.Empty() && PureMemoryOperand(psStore->asArguments[1], &szAddress, &cRegister) && szAddress.Equals(mszDeferred.Text());
	if (bResult && ((psInstruction->uiOpcode == IRO_Add) || (psInstruction->uiOpcode == IRO_Sub) || bShift) && IsFrameOperand(&szAddress))
	{
		//add, sub, shl and shr can't address memory with V.
		bResult = false;
	}
	if (bResult)
	{
		switch (psInstruction->uiOpcode)
		{
		case IRO_Add:	psz = Line("add ");	break;
		case IRO_Sub:	psz = Line("sub ");	break;
		case IRO_And:	psz = Line("and ");	break;
		case IRO_Or:	psz = Line("or ");	break;
		case IRO_Xor:	psz = Line("xor ");	break;
		case IRO_Shl:	psz = Line("shl ");	break;
		default:		psz = Line("shr ");	break;
		}
		psz->Append(szAddress)->Append(", ")->Append(szOperand)->AppendNewLine();
		mpsWritten = psStore;
		msInA = msAvailable;
	}
	szAddress.Kill();
	szOperand.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// add, sub, and, or, xor, shl and lshr (and neg, not and copy), 32 bits or
// narrower.  The result is worked out where it is kept when its first
// operand is kept there too (add X, 1; add [8 + W], A), in its register
// (cpy X, [4 + W] / add X, B), or in A.  Narrow operands are held zero
// extended, which these don't mind; the result is truncated.  64 bit values
// work with A and B 64 bits wide.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Integer(SIRInstruction* psInstruction)
{
	uint16			uiOpcode;
	const char*		szOperation;
	const char*		szWork;
	bool			bArithmetic;
	bool			bShift;
	SIRReference	sLeft;
	SIRReference	sRight;
	SIRReference	sResult;
	SIR65Place*		psResult;
	CChars			szOperand;
	CChars*			psz;

	if (IsWide(psInstruction->sType))
	{
		return IntegerWide(psInstruction);
	}
	uiOpcode = psInstruction->uiOpcode;
	sLeft = psInstruction->asArguments[0];
	sRight = psInstruction->asArguments[1];
	sResult = psInstruction->sResult;
	bShift = (uiOpcode == IRO_Shl) || (uiOpcode == IRO_LShr);
	bArithmetic = (uiOpcode == IRO_Add) || (uiOpcode == IRO_Sub) || (uiOpcode == IRO_PtrAdd) || bShift;
	switch (uiOpcode)
	{
	case IRO_Add:
	case IRO_PtrAdd:
		szOperation = "add ";
		break;
	case IRO_Sub:
		szOperation = "sub ";
		break;
	case IRO_And:
		szOperation = "and ";
		break;
	case IRO_Or:
		szOperation = "or ";
		break;
	case IRO_Xor:
	case IRO_Not:
		szOperation = "xor ";
		break;
	case IRO_Shl:
		szOperation = "shl ";
		break;
	case IRO_LShr:
		szOperation = "shr ";
		break;
	case IRO_Neg:
		szOperation = "neg ";
		break;
	default:
		szOperation = NULL;
		break;
	}

	//Where the first operand is kept already (or the second, if they swap).
	if (Commutes(uiOpcode) && SamePlace(sRight, sResult) && !SamePlace(sLeft, sResult))
	{
		sLeft = psInstruction->asArguments[1];
		sRight = psInstruction->asArguments[0];
	}
	if (sResult.IsTemporary() && !IsNarrow(psInstruction->sType) && SamePlace(sLeft, sResult) && !SamePlace(sRight, sResult))
	{
		psResult = PlaceOf(sResult);
		szOperand.Init();
		if (uiOpcode == IRO_Copy)
		{
			szOperand.Kill();
			return true;
		}
		if (psResult->uiKind == IR65P_Register)
		{
			if (uiOpcode == IRO_Neg)
			{
				Line("neg ")->Append(RegisterName(psResult))->Append(", ")->Append(RegisterName(psResult))->AppendNewLine();
				szOperand.Kill();
				return true;
			}
			if (uiOpcode == IRO_Not)
			{
				szOperand.Append("-1");
			}
			if ((uiOpcode == IRO_Not) || Operand(sRight, &szOperand, bArithmetic, IR65E_Zero))
			{
				if (bShift && (szOperand.Text()[0] == '['))
				{
					//Only A and B shift by an amount in memory.
					LoadValue(sRight, "B", IR65E_Zero);
					szOperand.Clear();
					szOperand.Append('B');
				}
				Line(szOperation)->Append(RegisterName(psResult))->Append(", ")->Append(szOperand)->AppendNewLine();
				szOperand.Kill();
				return true;
			}
		}
		else if ((psResult->uiKind == IR65P_Pseudo) && (uiOpcode != IRO_Neg))
		{
			if (uiOpcode == IRO_Not)
			{
				szOperand.Append("-1");
			}
			else if (IsInA(sRight))
			{
				szOperand.Append('A');
			}
			else if (InRegister(sRight) && !bShift)
			{
				//A shift of memory takes its amount from A or B.
				szOperand.Append(RegisterName(PlaceOf(sRight)));
			}
			else if (SmallImmediate(sRight, bShift ? 1 : -8, 8))
			{
				AppendImmediate(&szOperand, sRight, IR65E_Zero);
			}
			if (!szOperand.Empty())
			{
				psz = Line(szOperation);
				AppendPlace(psz, psResult);
				psz->Append(", ")->Append(szOperand)->AppendNewLine();
				szOperand.Kill();
				return true;
			}
		}
		szOperand.Kill();
	}

	szWork = WorkRegister(psInstruction);
	if ((StringCompare(szWork, "A") != 0) && (IsInA(sLeft) || SamePlace(sRight, sResult)))
	{
		szWork = "A";
	}

	szOperand.Init();
	if ((StringCompare(szWork, "A") == 0) && IsInA(sRight) && !IsInA(sLeft))
	{
		//The second operand is in A.
		if (Commutes(uiOpcode))
		{
			if (!Operand(sLeft, &szOperand, bArithmetic, IR65E_Zero))
			{
				LoadValue(sLeft, "B", IR65E_Zero);
				szOperand.Append('B');
			}
		}
		else if ((uiOpcode == IRO_Sub) && Operand(sLeft, &szOperand, true, IR65E_Zero))
		{
			//x - A is -A + x.
			Line("neg A, A")->AppendNewLine();
			szOperation = "add ";
		}
		else
		{
			szOperand.Clear();
			Line("cpy B, A")->AppendNewLine();
			LoadValue(sLeft, "A", IR65E_Zero);
			szOperand.Append('B');
		}
	}
	else
	{
		LoadValue(sLeft, szWork, IR65E_Zero);
		if (!sRight.IsNone() && !Operand(sRight, &szOperand, bArithmetic, IR65E_Zero))
		{
			LoadValue(sRight, "B", IR65E_Zero);
			szOperand.Append('B');
		}
		else if (bShift && (szWork[0] != 'A') && (szOperand.Text()[0] == '['))
		{
			//Only A and B shift by an amount in memory.
			LoadValue(sRight, "B", IR65E_Zero);
			szOperand.Clear();
			szOperand.Append('B');
		}
	}

	if (uiOpcode == IRO_Neg)
	{
		Line("neg ")->Append(szWork)->Append(", ")->Append(szWork)->AppendNewLine();
	}
	else if (uiOpcode == IRO_Not)
	{
		Line("xor ")->Append(szWork)->Append(", -1")->AppendNewLine();
	}
	else if (szOperation)
	{
		Line(szOperation)->Append(szWork)->Append(", ")->Append(szOperand)->AppendNewLine();
	}
	szOperand.Kill();
	if (StringCompare(szWork, "A") == 0)
	{
		msAvailable = IRNone();
	}
	StoreResult(sResult, szWork);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 64 bit add, sub, and, or, xor, neg, not, copy, shl and lshr: A = A op B
// with both 64 bits wide.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IntegerWide(SIRInstruction* psInstruction)
{
	const char*	szInstruction;

	LoadOperands(psInstruction, IR65E_Zero, true);
	Line("wtth (A, B), 64")->AppendNewLine();
	switch (psInstruction->uiOpcode)
	{
	case IRO_Add:
	case IRO_PtrAdd:
		szInstruction = "add A, ";
		break;
	case IRO_Sub:
		szInstruction = "sub A, ";
		break;
	case IRO_And:
		szInstruction = "and A, ";
		break;
	case IRO_Or:
		szInstruction = "or A, ";
		break;
	case IRO_Xor:
		szInstruction = "xor A, ";
		break;
	case IRO_Shl:
		szInstruction = "shl A, ";
		break;
	case IRO_LShr:
		szInstruction = "shr A, ";
		break;
	case IRO_Neg:
		szInstruction = "neg A, A";
		break;
	case IRO_Not:
		szInstruction = "xor A, -1";
		break;
	default:
		szInstruction = NULL;
		break;
	}
	if (szInstruction)
	{
		Line(szInstruction);
		if (szInstruction[StrLen(szInstruction) - 1] == ' ')
		{
			if (mbRightImmediate)
			{
				AppendImmediate(mpszOut, psInstruction->asArguments[1], IR65E_Zero);
			}
			else
			{
				mpszOut->Append('B');
			}
		}
		mpszOut->AppendNewLine();
	}
	Line("wtth (A, B), 32")->AppendNewLine();
	msAvailable = IRNone();
	StoreResult(psInstruction->sResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A x B -> Y.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::MultiplyInstruction(SIRInstruction* psInstruction)
{
	bool	bWide;

	bWide = IsWide(psInstruction->sType);
	if (!bWide && IsInA(psInstruction->asArguments[1]) && !IsInA(psInstruction->asArguments[0]))
	{
		LoadValue(psInstruction->asArguments[0], "B", IR65E_Zero);
	}
	else
	{
		LoadOperands(psInstruction, IR65E_Zero, false);
	}
	if (bWide)
	{
		Line("wtth (A, B), 64")->AppendNewLine();
		Line("wth Y, 64")->AppendNewLine();
	}
	Multiply("mul");
	msAvailable = IRNone();
	if (bWide)
	{
		Line("cpy A, Y")->AppendNewLine();
		Line("wth Y, 32")->AppendNewLine();
		Line("wtth (A, B), 32")->AppendNewLine();
		StoreResult(psInstruction->sResult);
	}
	else
	{
		StoreResult(psInstruction->sResult, "Y");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// ashr: shr is logical, so a negative value is inverted, shifted and
// inverted back.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::ShiftSigned(SIRInstruction* psInstruction)
{
	bool	bWide;
	int32	iPositive;
	int32	iEnd;
	CChars	szAmount;

	bWide = IsWide(psInstruction->sType);
	LoadOperands(psInstruction, IR65E_Sign, true);
	szAmount.Init();
	if (mbRightImmediate)
	{
		AppendImmediate(&szAmount, psInstruction->asArguments[1], IR65E_Zero);
	}
	else
	{
		szAmount.Append('B');
	}
	if (bWide)
	{
		Line("wtth (A, B), 64")->AppendNewLine();
	}
	iPositive = NewLabel();
	iEnd = NewLabel();
	Line("cmp A, 0")->AppendNewLine();
	Branch("brac n, ", iPositive);
	Line("xor A, -1")->AppendNewLine();
	Line("shr A, ")->Append(szAmount)->AppendNewLine();
	Line("xor A, -1")->AppendNewLine();
	Branch("bra ", iEnd);
	Label(iPositive);
	Line("shr A, ")->Append(szAmount)->AppendNewLine();
	Label(iEnd);
	szAmount.Kill();
	if (bWide)
	{
		Line("wtth (A, B), 32")->AppendNewLine();
	}
	msAvailable = IRNone();
	StoreResult(psInstruction->sResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Unsigned: div (Y / A -> Y, the remainder in B, which starts at 0).
// Signed: R_DivideSigned (A / B -> A, the remainder in B).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Divide(SIRInstruction* psInstruction)
{
	bool			bWide;
	bool			bSigned;
	bool			bRemainder;
	SIRReference	sLeft;
	SIRReference	sRight;

	bWide = IsWide(psInstruction->sType);
	bSigned = (psInstruction->uiOpcode == IRO_SDiv) || (psInstruction->uiOpcode == IRO_SRem);
	bRemainder = (psInstruction->uiOpcode == IRO_SRem) || (psInstruction->uiOpcode == IRO_URem);
	sLeft = psInstruction->asArguments[0];
	sRight = psInstruction->asArguments[1];

	if (bSigned || bWide)
	{
		LoadOperands(psInstruction, bSigned ? IR65E_Sign : IR65E_Zero, false);
		if (bWide)
		{
			Line("wtth (A, B), 64")->AppendNewLine();
		}
		if (bSigned)
		{
			Line(bWide ? "jsr R_DivideSigned64" : "jsr R_DivideSigned")->AppendNewLine();
			if (bWide)
			{
				mbSignedDivide64 = true;
			}
			else
			{
				mbSignedDivide = true;
			}
		}
		else
		{
			Line("wth Y, 64")->AppendNewLine();
			Line("cpy Y, A")->AppendNewLine();
			Line("cpy A, B")->AppendNewLine();
			Line("cpy B, 0")->AppendNewLine();
			Multiply("div");
			Line("cpy A, Y")->AppendNewLine();
			Line("wth Y, 32")->AppendNewLine();
		}
		if (bRemainder)
		{
			Line("cpy A, B")->AppendNewLine();
		}
		if (bWide)
		{
			Line("wtth (A, B), 32")->AppendNewLine();
		}
		msAvailable = IRNone();
		StoreResult(psInstruction->sResult);
		return true;
	}

	//The dividend to Y and the divisor to A, in an order that loses neither.
	if (IsInA(sRight) && !IsInA(sLeft))
	{
		LoadValue(sLeft, "Y", IR65E_Zero);
	}
	else if (InRegister(sRight) && (PlaceOf(sRight)->iOffset == IR65R_Y) && !sLeft.Equals(sRight))
	{
		if (IsInA(sLeft))
		{
			Line("cpy B, A")->AppendNewLine();
			Line("cpy A, Y")->AppendNewLine();
			Line("cpy Y, B")->AppendNewLine();
		}
		else
		{
			LoadValue(sRight, "A", IR65E_Zero);
			LoadValue(sLeft, "Y", IR65E_Zero);
		}
	}
	else
	{
		LoadValue(sLeft, "Y", IR65E_Zero);
		LoadValue(sRight, "A", IR65E_Zero);
	}
	Line("cpy B, 0")->AppendNewLine();
	Multiply("div");
	msAvailable = IRNone();
	StoreResult(psInstruction->sResult, bRemainder ? "B" : "Y");
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// sext, zext, trunc and the pointer conversions.  Narrow values are kept zero
// extended, so a conversion is a load (extended), a widening to 64 bits if
// the result is wide, and a store (truncated).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Conversion(SIRInstruction* psInstruction)
{
	SIRType		sFrom;
	bool		bSigned;
	const char*	szWork;

	sFrom = mpcFunction->TypeOf(psInstruction->asArguments[0]);
	bSigned = psInstruction->uiOpcode == IRO_SExt;
	szWork = WorkRegister(psInstruction);
	LoadValue(psInstruction->asArguments[0], szWork, bSigned ? IR65E_Sign : IR65E_Zero);
	if (IsWide(psInstruction->sType) && !IsWide(sFrom))
	{
		Line(bSigned ? "wthx A, 64:s" : "wthx A, 64")->AppendNewLine();
		Line("wth A, 32")->AppendNewLine();
	}
	StoreResult(psInstruction->sResult, szWork);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Floating point is the soft float runtime's: the operands' bits are pushed
// (the right first, as arguments are) and the result is in A.  fneg flips the
// sign bit in line.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::FloatOperation(SIRInstruction* psInstruction)
{
	bool		bDouble;
	const char*	szOperation;
	CChars		szName;

	bDouble = psInstruction->sType.Is(IRTK_F64);
	if (psInstruction->uiOpcode == IRO_FNeg)
	{
		LoadValue(psInstruction->asArguments[0], "A", IR65E_Zero);
		if (bDouble)
		{
			Line("wth A, 64")->AppendNewLine();
			Line("xor A, -9223372036854775808")->AppendNewLine();
			Line("wth A, 32")->AppendNewLine();
		}
		else
		{
			Line("xor A, -2147483648")->AppendNewLine();
		}
		msAvailable = IRNone();
		StoreResult(psInstruction->sResult);
		return true;
	}

	switch (psInstruction->uiOpcode)
	{
	case IRO_FAdd:	szOperation = "Add";		break;
	case IRO_FSub:	szOperation = "Subtract";	break;
	case IRO_FMul:	szOperation = "Multiply";	break;
	default:		szOperation = "Divide";		break;
	}
	Push(psInstruction->asArguments[1], psInstruction->sType);
	Push(psInstruction->asArguments[0], psInstruction->sType);
	szName.Init(bDouble ? "SoftDouble" : "SoftFloat");
	szName.Append(szOperation);
	CallRuntime(szName.Text(), 2 * ValueBytes(psInstruction->sType));
	szName.Kill();
	msAvailable = IRNone();
	StoreResult(psInstruction->sResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Conversions to, from and between floats, by the soft float runtime.
// Integers narrower than 32 bits convert as ints.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::FloatConversion(SIRInstruction* psInstruction)
{
	SIRType	sFrom;
	SIRType	sTo;
	bool	bSigned;
	CChars	szName;

	sFrom = mpcFunction->TypeOf(psInstruction->asArguments[0]);
	sTo = psInstruction->sType;
	switch (psInstruction->uiOpcode)
	{
	case IRO_FExt:
	case IRO_FTrunc:
		Push(psInstruction->asArguments[0], sFrom);
		CallRuntime(sTo.Is(IRTK_F64) ? "SoftFloatToDouble" : "SoftDoubleToFloat", ValueBytes(sFrom));
		break;

	case IRO_SIToF:
	case IRO_UIToF:
		bSigned = psInstruction->uiOpcode == IRO_SIToF;
		szName.Init(sTo.Is(IRTK_F64) ? "SoftDoubleFrom" : "SoftFloatFrom");
		if (IsWide(sFrom))
		{
			szName.Append(bSigned ? "LongLong" : "UnsignedLongLong");
		}
		else
		{
			szName.Append((bSigned || (sFrom.IntegerBits() < 32)) ? "Int" : "Unsigned");
		}
		if (IsWide(sFrom))
		{
			Push(psInstruction->asArguments[0], sFrom);
		}
		else
		{
			//Narrow integers are pushed as ints, extended.
			LoadValue(psInstruction->asArguments[0], "A", bSigned ? IR65E_Sign : IR65E_Zero);
			Line("psh A")->AppendNewLine();
		}
		CallRuntime(szName.Text(), IsWide(sFrom) ? 8 : 4);
		szName.Kill();
		break;

	default:
		bSigned = psInstruction->uiOpcode == IRO_FToSI;
		szName.Init(sFrom.Is(IRTK_F64) ? "SoftDoubleTo" : "SoftFloatTo");
		if (IsWide(sTo))
		{
			szName.Append(bSigned ? "LongLong" : "UnsignedLongLong");
		}
		else
		{
			szName.Append((bSigned || (sTo.IntegerBits() < 32)) ? "Int" : "Unsigned");
		}
		Push(psInstruction->asArguments[0], sFrom);
		CallRuntime(szName.Text(), ValueBytes(sFrom));
		szName.Kill();
		break;
	}
	msAvailable = IRNone();
	StoreResult(psInstruction->sResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A 4 byte load left in A for the next instruction isn't written when its
// address needs no instructions and uses no register the next instruction
// changes before reading it: the next instruction reads memory itself
// (add A, [8 + X]; cpy Y, [[4 + W]]).  Not before mul, div or ashr.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Defer(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psLoad;
	SIRInstruction*	psNext;
	size			uiNext;
	const char*		szWork;
	char			cRegister;
	SIRReference	sResult;
	CChars			szOperand;

	psLoad = pcBlock->masInstructions.Get(uiIndex);
	sResult = psLoad->sResult;
	if (!msDeferred.IsNone() || !sResult.IsTemporary() || !PlaceOf(sResult)->bInA || (psLoad->uiFlags & IRF_Volatile) || (ValueBytes(psLoad->sType) != 4))
	{
		return false;
	}
	uiNext = NextWritten(pcBlock, uiIndex, pcBlock->masInstructions.NumElements());
	psNext = pcBlock->masInstructions.SafeGet(uiNext);
	if (psNext)
	{
		if (IR65OpcodeFlags(psNext->uiOpcode) & IR65F_NO_DEFER)
		{
			return false;
		}
		szWork = WorkRegister(psNext);
	}
	else
	{
		szWork = "A";
	}

	szOperand.Init();
	cRegister = '\0';
	if (!PureMemoryOperand(psLoad->asArguments[0], &szOperand, &cRegister) || (cRegister == szWork[0]))
	{
		szOperand.Kill();
		return false;
	}
	mszDeferred.Kill();
	mszDeferred.Init(&szOperand);
	szOperand.Kill();
	msDeferred = sResult;
	msInA = msAvailable;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An address's memory operand when it needs no instructions and doesn't use
// A or B; cRegister is set to X or Y if it uses one.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::PureMemoryOperand(SIRReference sAddress, CChars* pszOperand, char* pcRegister)
{
	SIR65Place*		psPlace;
	SIR65Place*		psBase;
	SIRReference	sBase;

	if (sAddress.IsSymbol() || sAddress.IsConstant())
	{
		MemoryOperand(sAddress, pszOperand, NULL);
		return true;
	}
	if (!sAddress.IsTemporary() || IsInA(sAddress))
	{
		return false;
	}
	psPlace = PlaceOf(sAddress);
	switch (psPlace->uiKind)
	{
	case IR65P_Folded:
		sBase = psPlace->sBase;
		if (sBase.IsTemporary())
		{
			if (IsInA(sBase))
			{
				return false;
			}
			psBase = PlaceOf(sBase);
			if (psBase->uiKind == IR65P_Register)
			{
				*pcRegister = RegisterName(psBase)[0];
			}
			else if ((psBase->uiKind != IR65P_FrameAddress) && (psBase->uiKind != IR65P_ArgumentAddress))
			{
				return false;
			}
		}
		FoldedOperand(psPlace, pszOperand, NULL);
		return true;
	case IR65P_Register:
		*pcRegister = RegisterName(psPlace)[0];
		MemoryOperand(sAddress, pszOperand, NULL);
		return true;
	case IR65P_FrameAddress:
	case IR65P_ArgumentAddress:
		MemoryOperand(sAddress, pszOperand, NULL);
		return true;
	default:
		if (!IsMemoryHome(psPlace))
		{
			return false;
		}
		MemoryOperand(sAddress, pszOperand, NULL);
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Into the result's register (or A): narrow values read at their width
// and widened, 8 byte values with A 64 bits wide.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Load(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	CChars			szOperand;
	int				iBytes;
	const char*		szWork;

	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	if (Defer(pcBlock, uiIndex))
	{
		return true;
	}
	iBytes = ValueBytes(psInstruction->sType);
	szWork = WorkRegister(psInstruction);
	szOperand.Init();
	MemoryOperand(psInstruction->asArguments[0], &szOperand, (iBytes == 4) ? NULL : szWork);
	if (StringCompare(szWork, "A") == 0)
	{
		msAvailable = IRNone();
	}
	if (iBytes == 8)
	{
		Line("wth A, 64")->AppendNewLine();
		Line("cpy A, ")->Append(szOperand)->AppendNewLine();
		Line("wth A, 32")->AppendNewLine();
	}
	else if (iBytes == 4)
	{
		Line("cpy ")->Append(szWork)->Append(", ")->Append(szOperand)->AppendNewLine();
	}
	else
	{
		Line("wth ")->Append(szWork)->Append(", ")->Append(8 * iBytes)->AppendNewLine();
		Line("cpy ")->Append(szWork)->Append(", ")->Append(szOperand)->AppendNewLine();
		Line("wthx ")->Append(szWork)->Append(", 32")->AppendNewLine();
	}
	szOperand.Kill();
	StoreResult(psInstruction->sResult, szWork, true);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// From the value's register (or A, or a small constant straight to a 4
// byte place): narrow values written at their width, 8 byte values with A
// 64 bits wide.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Store(SIRInstruction* psInstruction)
{
	CChars			szOperand;
	CChars			szValue;
	int				iBytes;
	const char*		szValueRegister;
	SIRReference	sValue;
	bool			bLoad;

	sValue = psInstruction->asArguments[0];
	iBytes = ValueBytes(psInstruction->sType);
	szOperand.Init();
	szValue.Init();
	szValueRegister = NULL;
	bLoad = false;
	if ((iBytes == 4) && SmallImmediate(sValue, -8, 8))
	{
		AppendImmediate(&szValue, sValue, IR65E_Zero);
	}
	else if ((iBytes != 8) && IsInA(sValue))
	{
		szValueRegister = "A";
	}
	else if ((iBytes != 8) && InRegister(sValue))
	{
		szValueRegister = RegisterName(PlaceOf(sValue));
	}
	else
	{
		szValueRegister = "A";
		bLoad = true;
	}

	//The address first, so it doesn't use A when the value is loaded into it afterwards.
	MemoryOperand(psInstruction->asArguments[1], &szOperand, (bLoad || (iBytes != 4)) ? szValueRegister : NULL);
	if (bLoad)
	{
		LoadValue(sValue, "A", IR65E_Zero);
	}

	if (szValueRegister == NULL)
	{
		Line("cpy ")->Append(szOperand)->Append(", ")->Append(szValue)->AppendNewLine();
	}
	else if (iBytes == 4)
	{
		Line("cpy ")->Append(szOperand)->Append(", ")->Append(szValueRegister)->AppendNewLine();
	}
	else if (iBytes == 8)
	{
		Line("wth A, 64")->AppendNewLine();
		Line("cpy ")->Append(szOperand)->Append(", A")->AppendNewLine();
		Line("wth A, 32")->AppendNewLine();
	}
	else
	{
		Line("wth ")->Append(szValueRegister)->Append(", ")->Append(8 * iBytes)->AppendNewLine();
		Line("cpy ")->Append(szOperand)->Append(", ")->Append(szValueRegister)->AppendNewLine();
		Line("wth ")->Append(szValueRegister)->Append(", 32")->AppendNewLine();
	}
	szValue.Kill();
	szOperand.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The arg instructions before the call are pushed last first, each at its
// own size (a thiscall's first goes in X instead), then jsr and the caller
// removes them.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Call(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psCall;
	SIRInstruction*	psArg;
	SIRReference	sCallee;
	size			uiFirst;
	size			ui;
	int				iBytes;
	bool			bThisCall;
	const char*		szCallee;

	psCall = pcBlock->masInstructions.Get(uiIndex);
	sCallee = psCall->asArguments[0];
	uiFirst = uiIndex;
	while ((uiFirst > 0) && (pcBlock->masInstructions.Get(uiFirst - 1)->uiOpcode == IRO_Arg))
	{
		uiFirst--;
	}
	bThisCall = (psCall->uiFlags & IRF_ThisCall) && (uiFirst < uiIndex);

	iBytes = 0;
	for (ui = uiIndex; ui > uiFirst + (bThisCall ? 1 : 0); ui--)
	{
		psArg = pcBlock->masInstructions.Get(ui - 1);
		Push(psArg->asArguments[0], psArg->sType);
		iBytes += ValueBytes(psArg->sType);
	}

	szCallee = NULL;
	if (sCallee.IsSymbol())
	{
		szCallee = mpcModule->GetSymbolName(sCallee.Index());
	}
	else if (InRegister(sCallee) && (PlaceOf(sCallee)->iOffset == IR65R_Y))
	{
		szCallee = "Y";
	}
	else
	{
		LoadValue(sCallee, "A", IR65E_Zero);
		szCallee = "A";
	}
	if (bThisCall)
	{
		LoadValue(pcBlock->masInstructions.Get(uiFirst)->asArguments[0], "X", IR65E_Zero);
	}
	Line("jsr ")->Append(szCallee)->AppendNewLine();
	if (iBytes > 0)
	{
		Line("add S, ")->Append(iBytes)->AppendNewLine();
	}
	msAvailable = IRNone();
	StoreResult(psCall->sResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Branches to iLabel when the comparison is bJumpIf.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Compare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel)
{
	if (psInstruction->sType.IsFloat())
	{
		return FloatCompare(psInstruction, bJumpIf, iLabel);
	}
	return IntegerCompare(psInstruction, bJumpIf, iLabel);
}


//////////////////////////////////////////////////////////////////////////
//
// cmp first, second for a comparison of 32 bits or less that needs no
// sign extending: the first operand where it is kept (X, [4 + W]) or in A
// or B, the second as an operand that can go with it, or the constant
// lliConstant when bConstant.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::CompareOperands(SIRReference sFirst, SIRReference sSecond, bool bConstant, int64 lliConstant)
{
	CChars	szLeft;
	CChars	szRight;
	bool	bSmall;

	szLeft.Init();
	szRight.Init();
	if (bConstant)
	{
		bSmall = (lliConstant >= -8) && (lliConstant <= 8);
	}
	else
	{
		bSmall = IsInA(sSecond) || InRegister(sSecond) || SmallImmediate(sSecond, -8, 8);
	}

	if (IsInA(sFirst))
	{
		szLeft.Append('A');
	}
	else if (InRegister(sFirst))
	{
		szLeft.Append(RegisterName(PlaceOf(sFirst)));
	}
	else if (bSmall && sFirst.IsTemporary() && (PlaceOf(sFirst)->uiKind == IR65P_Pseudo))
	{
		//Memory goes with a register or -8 to 8.
		AppendPlace(&szLeft, PlaceOf(sFirst));
	}
	else if (!bConstant && IsInA(sSecond))
	{
		LoadValue(sFirst, "B", IR65E_Zero);
		szLeft.Append('B');
	}
	else
	{
		LoadValue(sFirst, "A", IR65E_Zero);
		szLeft.Append('A');
	}

	if (bConstant)
	{
		szRight.Append(lliConstant);
	}
	else if (!Operand(sSecond, &szRight, true, IR65E_Zero))
	{
		LoadValue(sSecond, (StringCompare(szLeft.Text(), "B") == 0) ? "A" : "B", IR65E_Zero);
		szRight.Append((StringCompare(szLeft.Text(), "B") == 0) ? 'A' : 'B');
	}
	Line("cmp ")->Append(szLeft)->Append(", ")->Append(szRight)->AppendNewLine();
	szRight.Kill();
	szLeft.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// After cmp X, Y: z is X = Y, c clear is X < Y unsigned and n != v is X < Y
// signed.  > and <= swap the operands (or compare with the constant plus
// one).  There is no branch on n = v, so it branches round a bra.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::IntegerCompare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel)
{
	uint16			uiOpcode;
	bool			bSigned;
	bool			bSwap;
	bool			bLess;
	bool			bWide;
	int32			iSkip;
	SIRReference	sLeft;
	SIRReference	sRight;
	SIRConstant*	psConstant;
	int64			lliValue;
	int64			lliLimit;
	bool			bAdjusted;
	int				iBits;

	uiOpcode = psInstruction->uiOpcode;
	if (mbFlagsKnown)
	{
		//The block before compared the same operands.
		Branch(((uiOpcode == IRO_Eq) == bJumpIf) ? "bras z, " : "brac z, ", iLabel);
		return true;
	}
	bSigned = (uiOpcode == IRO_SLt) || (uiOpcode == IRO_SLe) || (uiOpcode == IRO_SGt) || (uiOpcode == IRO_SGe);
	bSwap = (uiOpcode == IRO_SGt) || (uiOpcode == IRO_SLe) || (uiOpcode == IRO_UGt) || (uiOpcode == IRO_ULe);
	bWide = IsWide(psInstruction->sType);
	sLeft = psInstruction->asArguments[0];
	sRight = psInstruction->asArguments[1];

	if (bWide || (bSigned && IsNarrow(psInstruction->sType)))
	{
		//Signed i8 and i16 are compared at their own width (the registers narrowed), which needs no extending.
		iBits = bWide ? 64 : psInstruction->sType.IntegerBits();
		if (iBits == 1)
		{
			LoadOperands(psInstruction, IR65E_Sign, !bSwap);
			iBits = 32;
		}
		else
		{
			LoadOperands(psInstruction, IR65E_Zero, !bSwap);
		}
		if (iBits != 32)
		{
			Line("wtth (A, B), ")->Append(iBits)->AppendNewLine();
		}
		if (mbRightImmediate)
		{
			Line("cmp A, ");
			AppendImmediate(mpszOut, sRight, bSigned ? IR65E_Sign : IR65E_Zero);
			mpszOut->AppendNewLine();
		}
		else
		{
			Line(bSwap ? "cmp B, A" : "cmp A, B")->AppendNewLine();
		}
		if (iBits != 32)
		{
			Line("wtth (A, B), 32")->AppendNewLine();
		}
	}
	else
	{
		bAdjusted = false;
		lliValue = 0;
		if (bSwap && sRight.IsConstant())
		{
			//x > c is x >= c + 1 and x <= c is x < c + 1, unless c is the largest value.
			psConstant = mpcFunction->GetConstant(sRight);
			if (bSigned)
			{
				lliValue = (int64)(int32)psConstant->ulliBits;
				lliLimit = 0x7FFFFFFF;
			}
			else
			{
				lliValue = (int64)IRTruncate(psConstant->sType, psConstant->ulliBits);
				lliLimit = (int64)IRTruncate(psConstant->sType, (uint64)-1);
			}
			if (lliValue < lliLimit)
			{
				lliValue = (int64)(int32)(lliValue + 1);
				bAdjusted = true;
				switch (uiOpcode)
				{
				case IRO_SGt:	uiOpcode = IRO_SGe;	break;
				case IRO_SLe:	uiOpcode = IRO_SLt;	break;
				case IRO_UGt:	uiOpcode = IRO_UGe;	break;
				default:		uiOpcode = IRO_ULt;	break;
				}
				bSwap = false;
			}
		}
		if (bSwap)
		{
			CompareOperands(sRight, sLeft, false, 0);
		}
		else
		{
			CompareOperands(sLeft, sRight, bAdjusted, lliValue);
		}
	}

	if ((uiOpcode == IRO_Eq) || (uiOpcode == IRO_Ne))
	{
		Branch(((uiOpcode == IRO_Eq) == bJumpIf) ? "bras z, " : "brac z, ", iLabel);
		return true;
	}

	//slt, ult and the swapped sgt, ugt are less; sge, uge and the swapped sle, ule are greater or equal.
	bLess = ((uiOpcode == IRO_SLt) || (uiOpcode == IRO_ULt) || (uiOpcode == IRO_SGt) || (uiOpcode == IRO_UGt)) == bJumpIf;
	if (!bSigned)
	{
		Branch(bLess ? "brac c, " : "bras c, ", iLabel);
	}
	else if (bLess)
	{
		Branch("braso (n, v), ", iLabel);
	}
	else
	{
		iSkip = NewLabel();
		Branch("braso (n, v), ", iSkip);
		Branch("bra ", iLabel);
		Label(iSkip);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The runtime's compare gives -1, 0 or 1, or 2 when either is a NaN (so
// every comparison but fne and funordered is false then).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::FloatCompare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel)
{
	const char*	szTrue;
	const char*	szFalse;

	Push(psInstruction->asArguments[1], psInstruction->sType);
	Push(psInstruction->asArguments[0], psInstruction->sType);
	CallRuntime(psInstruction->sType.Is(IRTK_F64) ? "SoftDoubleCompare" : "SoftFloatCompare", 2 * ValueBytes(psInstruction->sType));
	msAvailable = IRNone();

	szTrue = "bras z, ";
	szFalse = "brac z, ";
	switch (psInstruction->uiOpcode)
	{
	case IRO_FEq:
		Line("cmp A, 0")->AppendNewLine();
		break;
	case IRO_FNe:
		Line("cmp A, 0")->AppendNewLine();
		szTrue = "brac z, ";
		szFalse = "bras z, ";
		break;
	case IRO_FLt:
		Line("cmp A, -1")->AppendNewLine();
		break;
	case IRO_FGt:
		Line("cmp A, 1")->AppendNewLine();
		break;
	case IRO_FLe:
		//-1 or 0: A + 1 is below 2, unsigned.
		Line("add A, 1")->AppendNewLine();
		Line("cmp A, 2")->AppendNewLine();
		szTrue = "brac c, ";
		szFalse = "bras c, ";
		break;
	case IRO_FGe:
		//0 or 1.
		Line("cmp A, 2")->AppendNewLine();
		szTrue = "brac c, ";
		szFalse = "bras c, ";
		break;
	case IRO_FOrdered:
		Line("cmp A, 2")->AppendNewLine();
		szTrue = "brac z, ";
		szFalse = "bras z, ";
		break;
	default:
		Line("cmp A, 2")->AppendNewLine();
		break;
	}
	Branch(bJumpIf ? szTrue : szFalse, iLabel);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if psNext's lowering takes sValue (its first operand, and not its
// second) from A when the instruction before left it there, so a result
// it alone uses needn't be kept.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::TakesInA(SIRInstruction* psNext, SIRReference sValue)
{
	return psNext->asArguments[0].Equals(sValue) && !psNext->asArguments[1].Equals(sValue) && (IR65OpcodeFlags(psNext->uiOpcode) & IR65F_TAKES_A);
}


//////////////////////////////////////////////////////////////////////////
//
// The same for what the block's jump does first: a fused comparison, a
// brif's condition or a return's value.  Not when the jump copies phi
// values first (a jmp, or a brif's then edge after the test is fine).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::JumpTakesInA(CIRBlock* pcBlock, SIRInstruction* psFused, SIRReference sValue)
{
	if (psFused)
	{
		return TakesInA(psFused, sValue);
	}
	if ((pcBlock->msJump.uiKind == IRJ_Branch) || (pcBlock->msJump.uiKind == IRJ_Return))
	{
		return pcBlock->msJump.sArgument.Equals(sValue);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if sValue is a constant "op A, value" can take: an integer of 32
// bits or fewer.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Immediate(SIRReference sValue, SIRType sType)
{
	return sValue.IsConstant() && !IsWide(sType) && !sType.IsFloat();
}


//////////////////////////////////////////////////////////////////////////
//
// A two operand instruction's operands: the first in A, the second in B, or
// left as a constant when bImmediate and it is one (mbRightImmediate).  A
// second operand the instruction before left in A is copied to B first.
// For 64 bit values, signed narrow comparisons, mul and the divisions.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::LoadOperands(SIRInstruction* psInstruction, EIR65Extend eExtend, bool bImmediate)
{
	SIRReference	sLeft;
	SIRReference	sRight;
	SIRType			sType;

	sLeft = psInstruction->asArguments[0];
	sRight = psInstruction->asArguments[1];
	mbRightImmediate = false;
	if (sRight.IsNone())
	{
		LoadValue(sLeft, "A", eExtend);
		return;
	}
	sType = mpcFunction->TypeOf(sRight);
	if (bImmediate && Immediate(sRight, sType))
	{
		LoadValue(sLeft, "A", eExtend);
		mbRightImmediate = true;
		return;
	}
	if (IsInA(sRight) && !sLeft.Equals(sRight))
	{
		if (IsWide(sType))
		{
			Line("wtth (A, B), 64")->AppendNewLine();
			Line("cpy B, A")->AppendNewLine();
			Line("wtth (A, B), 32")->AppendNewLine();
		}
		else
		{
			Line("cpy B, A")->AppendNewLine();
		}
		msAvailable = IRNone();
		Extend("B", sType, eExtend);
		LoadValue(sLeft, "A", eExtend);
		return;
	}
	LoadValue(sLeft, "A", eExtend);
	LoadValue(sRight, "B", eExtend);
}


//////////////////////////////////////////////////////////////////////////
//
// True if psNext takes sValue as its second operand (and not its first)
// from A when the instruction before left it there.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::TakesInB(SIRInstruction* psNext, SIRReference sValue)
{
	return psNext->asArguments[1].Equals(sValue) && !psNext->asArguments[0].Equals(sValue) && (IR65OpcodeFlags(psNext->uiOpcode) & IR65F_TAKES_B);
}
