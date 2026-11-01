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
#include "IRVerifier.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRVerifier::Init(CIRModule* pcModule, CCPPErrors* pcErrors)
{
	mpcModule = pcModule;
	mpcErrors = pcErrors;
	mpcFunction = NULL;
	mpcBlock = NULL;
	masDefinitions.Init();
	mcDominators.Init();
	mauiCounts.Init();
	mauiStamps.Init();
	muiStamp = 0;
	muiStartErrors = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRVerifier::Kill(void)
{
	mauiStamps.Kill();
	mauiCounts.Kill();
	mcDominators.Kill();
	masDefinitions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// True if every function is well formed.
//
//////////////////////////////////////////////////////////////////////////
bool CIRVerifier::Verify(void)
{
	size	ui;
	bool	bResult;

	bResult = true;
	for (ui = 0; ui < mpcModule->NumFunctions(); ui++)
	{
		bResult &= VerifyFunction(mpcModule->GetFunction(ui));
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRVerifier::VerifyFunction(CIRFunction* pcFunction)
{
	size	ui;
	size	uiInstruction;

	mpcFunction = pcFunction;
	mpcBlock = NULL;
	muiStartErrors = mpcErrors->NumErrors();

	CheckType(pcFunction->msReturnType, true);
	if (pcFunction->NumBlocks() == 0)
	{
		Error("Function has no blocks.");
		return false;
	}
	mauiStamps.FakeSetUsedElements(0);
	mauiStamps.GrowToAtLeastNumElements(pcFunction->NumBlocks(), true, 0);
	muiStamp = 0;
	if (!CheckBlocks() || !CheckDefinitions())
	{
		return false;
	}
	mcDominators.Compute(pcFunction);

	for (ui = 0; ui < pcFunction->NumBlocks(); ui++)
	{
		mpcBlock = pcFunction->GetBlock(ui);
		CheckPhis(mpcBlock);
		for (uiInstruction = 0; uiInstruction < mpcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			CheckInstruction(mpcBlock, uiInstruction);
		}
		CheckJump(mpcBlock);
	}
	mpcBlock = NULL;
	return mpcErrors->NumErrors() == muiStartErrors;
}


//////////////////////////////////////////////////////////////////////////
//
// "@function block: message [detail]".  An unnamed block is its index.
//
//////////////////////////////////////////////////////////////////////////
void CIRVerifier::Error(const char* szMessage, const char* szDetail)
{
	CChars	szError;

	szError.Init("@");
	szError.Append(mpcModule->GetSymbolName(mpcFunction->muiSymbol));
	if (mpcBlock)
	{
		szError.Append(' ');
		if (mpcBlock->mszName.Empty())
		{
			szError.Append(mpcBlock->muiIndex);
		}
		else
		{
			szError.Append(mpcBlock->GetName());
		}
	}
	szError.Append(": ");
	szError.Append(szMessage);
	mpcErrors->Add(NULL, -1, -1, -1, szError.Text(), szDetail);
	szError.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A temporary's name for an error's detail; NULL if it has none.
//
//////////////////////////////////////////////////////////////////////////
char* CIRVerifier::NameOf(SIRReference sReference)
{
	return sReference.IsTemporary() ? mpcFunction->GetTemporaryName(sReference) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRVerifier::CheckType(SIRType sType, bool bAllowVoid)
{
	if (sType.uiKind >= IRTK_NumKinds)
	{
		Error("Bad type.");
		return false;
	}
	if (sType.IsNone() && !bAllowVoid)
	{
		Error("Missing type.");
		return false;
	}
	if (sType.IsPointer() && (sType.uiSpace >= mpcModule->NumSpaces()))
	{
		Error("Unknown address space.");
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRVerifier::CheckReference(SIRReference sReference)
{
	switch (sReference.Kind())
	{
	case IRRK_Temporary:
		return sReference.Index() < mpcFunction->NumTemporaries();
	case IRRK_Constant:
		return sReference.Index() < mpcFunction->masConstants.NumElements();
	case IRRK_Symbol:
		return sReference.Index() < mpcModule->maszSymbols.NumElements();
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRVerifier::IsFunctionBlock(CIRBlock* pcBlock)
{
	return pcBlock && (pcBlock->muiIndex < mpcFunction->NumBlocks()) && (mpcFunction->GetBlock(pcBlock->muiIndex) == pcBlock);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRVerifier::JumpsTo(CIRBlock* pcFrom, CIRBlock* pcTo)
{
	return ((pcFrom->NumSuccessors() >= 1) && (pcFrom->msJump.pcThen == pcTo)) ||
		   ((pcFrom->NumSuccessors() == 2) && (pcFrom->msJump.pcElse == pcTo));
}


//////////////////////////////////////////////////////////////////////////
//
// A value no block's stamp has yet.
//
//////////////////////////////////////////////////////////////////////////
uint32 CIRVerifier::NewStamp(void)
{
	muiStamp++;
	return muiStamp;
}


//////////////////////////////////////////////////////////////////////////
//
// Each block ends with a jump to blocks of this function, the entry has no
// predecessors and each block's predecessors are exactly the blocks that
// jump to it.  The blocks are renumbered.
//
//////////////////////////////////////////////////////////////////////////
bool CIRVerifier::CheckBlocks(void)
{
	size		ui;
	size		uiSuccessor;
	size		uiPredecessor;
	CIRBlock*	pcSuccessor;
	CIRBlock*	pcPredecessor;
	uint32		uiStamp;
	bool		bResult;

	bResult = true;
	mpcFunction->Renumber();
	mauiCounts.FakeSetUsedElements(0);
	mauiCounts.GrowToAtLeastNumElements(mpcFunction->NumBlocks(), true, 0);
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		mpcBlock = mpcFunction->GetBlock(ui);
		if (!mpcBlock->HasJump())
		{
			Error("Block has no jump.");
			bResult = false;
			continue;
		}
		if ((mpcBlock->msJump.uiKind == IRJ_Branch) && (mpcBlock->msJump.pcThen == mpcBlock->msJump.pcElse))
		{
			Error("brif to the same block twice.");
			bResult = false;
			continue;
		}
		for (uiSuccessor = 0; uiSuccessor < mpcBlock->NumSuccessors(); uiSuccessor++)
		{
			pcSuccessor = mpcBlock->GetSuccessor(uiSuccessor);
			if (!IsFunctionBlock(pcSuccessor))
			{
				Error("Jump to a block not in the function.");
				bResult = false;
			}
			else
			{
				(*mauiCounts.Get(pcSuccessor->muiIndex))++;
			}
		}
	}
	if (!bResult)
	{
		mpcBlock = NULL;
		return false;
	}

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		mpcBlock = mpcFunction->GetBlock(ui);
		if ((ui == 0) && (mpcBlock->mapcPredecessors.NumElements() != 0))
		{
			Error("The entry block has predecessors.");
			bResult = false;
			continue;
		}
		uiStamp = NewStamp();
		for (uiPredecessor = 0; uiPredecessor < mpcBlock->mapcPredecessors.NumElements(); uiPredecessor++)
		{
			pcPredecessor = mpcBlock->mapcPredecessors.GetPtr(uiPredecessor);
			if (!IsFunctionBlock(pcPredecessor) || !JumpsTo(pcPredecessor, mpcBlock) || (*mauiStamps.Get(pcPredecessor->muiIndex) == uiStamp))
			{
				Error("Predecessor doesn't jump to the block once.");
				bResult = false;
				break;
			}
			mauiStamps.Set(pcPredecessor->muiIndex, &uiStamp);
		}
		if (bResult && (mpcBlock->mapcPredecessors.NumElements() != *mauiCounts.Get(ui)))
		{
			Error("A block that jumps to the block is missing from its predecessors.");
			bResult = false;
		}
	}
	mpcBlock = NULL;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Each temporary is defined once, with a type.
//
//////////////////////////////////////////////////////////////////////////
bool CIRVerifier::CheckDefinitions(void)
{
	size			ui;
	size			uiInstruction;
	SIRReference	sTemporary;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;

	masDefinitions.FakeSetUsedElements(0);
	masDefinitions.GrowToAtLeastNumElements(mpcFunction->NumTemporaries(), true, 0);

	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		Define(*mpcFunction->masParameters.Get(ui), NULL, 0);
	}
	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		mpcBlock = pcBlock;
		for (uiInstruction = 0; uiInstruction < pcBlock->masPhis.NumElements(); uiInstruction++)
		{
			Define(pcBlock->masPhis.Get(uiInstruction)->sResult, pcBlock, -1);
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (!psInstruction->sResult.IsNone())
			{
				Define(psInstruction->sResult, pcBlock, (int32)uiInstruction);
			}
		}
	}
	mpcBlock = NULL;

	for (ui = 0; ui < mpcFunction->NumTemporaries(); ui++)
	{
		sTemporary = IRReference(IRRK_Temporary, (uint32)ui);
		if (!CheckType(mpcFunction->GetTemporaryType(sTemporary), false))
		{
			Error("Temporary has no type.", NameOf(sTemporary));
		}
	}
	return mpcErrors->NumErrors() == muiStartErrors;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRVerifier::Define(SIRReference sResult, CIRBlock* pcBlock, int32 iPosition)
{
	SIRVerifierDefinition*	psDefinition;

	if (!sResult.IsTemporary() || !CheckReference(sResult))
	{
		Error("Result is not a temporary.");
		return;
	}
	psDefinition = masDefinitions.Get(sResult.Index());
	if (psDefinition->bDefined)
	{
		Error("Temporary defined twice.", NameOf(sResult));
		return;
	}
	psDefinition->bDefined = true;
	psDefinition->pcBlock = pcBlock;
	psDefinition->iPosition = iPosition;
}


//////////////////////////////////////////////////////////////////////////
//
// A temporary used at iPosition in pcBlock (an instruction's index, or the
// number of instructions for the jump and for a phi argument from pcBlock)
// must be defined there.
//
//////////////////////////////////////////////////////////////////////////
void CIRVerifier::CheckUse(SIRReference sOperand, CIRBlock* pcBlock, int32 iPosition)
{
	SIRVerifierDefinition*	psDefinition;

	if (!sOperand.IsTemporary())
	{
		return;
	}
	psDefinition = masDefinitions.Get(sOperand.Index());
	if (!psDefinition->bDefined)
	{
		Error("Temporary is never defined.", NameOf(sOperand));
		return;
	}
	if ((psDefinition->pcBlock == NULL) || !mcDominators.IsReachable(pcBlock))
	{
		return;
	}
	if (psDefinition->pcBlock == pcBlock)
	{
		if (psDefinition->iPosition >= iPosition)
		{
			Error("Temporary used before it is defined.", NameOf(sOperand));
		}
	}
	else if (!mcDominators.Dominates(psDefinition->pcBlock, pcBlock))
	{
		Error("Definition doesn't dominate the use.", NameOf(sOperand));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An operand of kind uiKind (sType is the instruction's), used at iPosition
// in pcBlock.
//
//////////////////////////////////////////////////////////////////////////
void CIRVerifier::CheckOperand(SIRReference sOperand, uint8 uiKind, SIRType sType, CIRBlock* pcBlock, int32 iPosition, const char* szWhat)
{
	SIRType	sOperandType;
	bool	bValid;
	CChars	szMessage;

	szMessage.Init(szWhat);
	if (!CheckReference(sOperand))
	{
		szMessage.Append(" is missing.");
		Error(szMessage.Text());
		szMessage.Kill();
		return;
	}
	CheckUse(sOperand, pcBlock, iPosition);

	sOperandType = mpcFunction->TypeOf(sOperand);
	switch (uiKind)
	{
	case IROK_Same:
		bValid = sOperandType.Equals(sType);
		break;
	case IROK_Pointer:
		bValid = sOperandType.IsPointer();
		break;
	case IROK_Offset:
		bValid = sOperandType.Equals(IRIntegerType(mpcModule->GetPointerSize(sType.uiSpace)));
		break;
	case IROK_Constant:
		bValid = sOperand.IsConstant() && sOperandType.Is(IRTK_I32);
		break;
	default:
		bValid = true;
		break;
	}
	if (!bValid)
	{
		szMessage.Append(" has the wrong type.");
		Error(szMessage.Text(), NameOf(sOperand));
	}
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRVerifier::CheckResult(SIRInstruction* psInstruction, SIROpcode* psOpcode)
{
	SIRType	sType;

	if (psInstruction->sResult.IsNone())
	{
		if ((psOpcode->uiResult == IRRS_Same) || (psOpcode->uiResult == IRRS_I1))
		{
			Error("Instruction has no result.", psOpcode->szName);
		}
		return;
	}
	if ((psOpcode->uiResult == IRRS_None) || ((psOpcode->uiResult == IRRS_Optional) && psInstruction->sType.IsNone()))
	{
		Error("Instruction can't have a result.", psOpcode->szName);
		return;
	}
	sType = (psOpcode->uiResult == IRRS_I1) ? IRType(IRTK_I1) : psInstruction->sType;
	if (!psInstruction->sResult.IsTemporary() || !CheckReference(psInstruction->sResult))
	{
		return;
	}
	if (!mpcFunction->GetTemporaryType(psInstruction->sResult).Equals(sType))
	{
		Error("Result has the wrong type.", NameOf(psInstruction->sResult));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// One argument for each predecessor, of the phi's type.
//
//////////////////////////////////////////////////////////////////////////
void CIRVerifier::CheckPhis(CIRBlock* pcBlock)
{
	size			ui;
	size			uiArgument;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;
	uint32			uiPredecessor;
	uint32			uiSeen;
	uint32*			puiStamp;

	if ((pcBlock == mpcFunction->GetEntry()) && (pcBlock->masPhis.NumElements() != 0))
	{
		Error("The entry block has phis.");
		return;
	}
	for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
	{
		psPhi = pcBlock->masPhis.Get(ui);
		if (!CheckType(psPhi->sType, false))
		{
			continue;
		}
		if (!mpcFunction->GetTemporaryType(psPhi->sResult).Equals(psPhi->sType))
		{
			Error("Result has the wrong type.", NameOf(psPhi->sResult));
		}
		if (psPhi->asArguments.NumElements() != pcBlock->mapcPredecessors.NumElements())
		{
			Error("Phi needs one argument for each predecessor.", NameOf(psPhi->sResult));
		}

		//Predecessors are stamped uiPredecessor, then uiSeen once an argument is from them.
		uiPredecessor = NewStamp();
		uiSeen = NewStamp();
		for (uiArgument = 0; uiArgument < pcBlock->mapcPredecessors.NumElements(); uiArgument++)
		{
			mauiStamps.Set(pcBlock->mapcPredecessors.GetPtr(uiArgument)->muiIndex, &uiPredecessor);
		}
		for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
		{
			psArgument = psPhi->asArguments.Get(uiArgument);
			puiStamp = IsFunctionBlock(psArgument->pcBlock) ? mauiStamps.Get(psArgument->pcBlock->muiIndex) : NULL;
			if ((puiStamp == NULL) || ((*puiStamp != uiPredecessor) && (*puiStamp != uiSeen)))
			{
				Error("Phi argument from a block that is not a predecessor.", NameOf(psPhi->sResult));
				continue;
			}
			if (*puiStamp == uiSeen)
			{
				Error("Phi has two arguments from one block.", NameOf(psPhi->sResult));
			}
			*puiStamp = uiSeen;
			CheckOperand(psArgument->sValue, IROK_Same, psPhi->sType, psArgument->pcBlock, (int32)psArgument->pcBlock->masInstructions.NumElements(), "Phi argument");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The instruction's type, operands, result and flags are what its opcode's
// table entry allows.
//
//////////////////////////////////////////////////////////////////////////
void CIRVerifier::CheckInstruction(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	SIROpcode*		psOpcode;
	SIRInstruction*	psNext;
	size			uiArguments;
	int				iOperand;

	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	psOpcode = mpcModule->GetOpcode(psInstruction->uiOpcode);
	if (psOpcode == NULL)
	{
		Error("Unknown opcode.");
		return;
	}
	if (!CheckType(psInstruction->sType, true))
	{
		return;
	}
	if ((psInstruction->sType.Mask() & psOpcode->uiTypes) == 0)
	{
		Error("Type not allowed for the opcode.", psOpcode->szName);
		return;
	}

	for (iOperand = 0; iOperand < psOpcode->NumOperands(); iOperand++)
	{
		CheckOperand(psInstruction->asArguments[iOperand], psOpcode->auiOperands[iOperand], psInstruction->sType, pcBlock, (int32)uiIndex, iOperand == 0 ? "Operand" : "Second operand");
	}
	if ((psOpcode->uiShape == IRSH_Conversion) && CheckReference(psInstruction->asArguments[0]))
	{
		CheckConversion(psInstruction, psOpcode);
	}
	CheckResult(psInstruction, psOpcode);
	if (psInstruction->uiFlags & ~psOpcode->uiFlags)
	{
		Error("Flag not allowed on the opcode.", psOpcode->szName);
	}

	if (psInstruction->uiOpcode == IRO_Arg)
	{
		psNext = pcBlock->masInstructions.SafeGet(uiIndex + 1);
		if ((psNext == NULL) || ((psNext->uiOpcode != IRO_Arg) && (psNext->uiOpcode != IRO_Call)))
		{
			Error("arg not followed by a call.");
		}
	}
	else if ((psInstruction->uiOpcode == IRO_Call) && (psInstruction->uiFlags & IRF_Variadic))
	{
		uiArguments = 0;
		while ((uiArguments < uiIndex) && (pcBlock->masInstructions.Get(uiIndex - uiArguments - 1)->uiOpcode == IRO_Arg))
		{
			uiArguments++;
		}
		if (psInstruction->uiNamed > uiArguments)
		{
			Error("More named arguments than arguments.");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The operand is one of the opcode's source types, and widens, narrows or
// changes space as the opcode does.
//
//////////////////////////////////////////////////////////////////////////
void CIRVerifier::CheckConversion(SIRInstruction* psInstruction, SIROpcode* psOpcode)
{
	SIRType	sFrom;
	SIRType	sTo;
	bool	bValid;

	sFrom = mpcFunction->TypeOf(psInstruction->asArguments[0]);
	sTo = psInstruction->sType;

	bValid = (sFrom.Mask() & psOpcode->uiSourceTypes) != 0;
	switch (psInstruction->uiOpcode)
	{
	case IRO_SExt:
	case IRO_ZExt:
		bValid &= sTo.IntegerBits() > sFrom.IntegerBits();
		break;
	case IRO_Trunc:
		bValid &= sTo.IntegerBits() < sFrom.IntegerBits();
		break;
	case IRO_FExt:
		bValid &= sFrom.Is(IRTK_F32) && sTo.Is(IRTK_F64);
		break;
	case IRO_FTrunc:
		bValid &= sFrom.Is(IRTK_F64) && sTo.Is(IRTK_F32);
		break;
	case IRO_SpaceCast:
		bValid &= sFrom.uiSpace != sTo.uiSpace;
		break;
	}
	if (!bValid)
	{
		Error("Conversion between these types is not allowed.", psOpcode->szName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A branch's condition is i1; a return's value is the function's type.
//
//////////////////////////////////////////////////////////////////////////
void CIRVerifier::CheckJump(CIRBlock* pcBlock)
{
	SIRJump*	psJump;
	int32		iEnd;

	psJump = &pcBlock->msJump;
	iEnd = (int32)pcBlock->masInstructions.NumElements();
	if (psJump->uiKind == IRJ_Branch)
	{
		CheckOperand(psJump->sArgument, IROK_Same, IRType(IRTK_I1), pcBlock, iEnd, "Condition");
	}
	else if (psJump->uiKind == IRJ_Return)
	{
		if (!psJump->sType.Equals(mpcFunction->msReturnType))
		{
			Error("ret's type is not the function's.");
		}
		else if (!psJump->sType.IsNone())
		{
			CheckOperand(psJump->sArgument, IROK_Same, psJump->sType, pcBlock, iEnd, "Return value");
		}
	}
}
