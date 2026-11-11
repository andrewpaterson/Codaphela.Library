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
#include "IRBackend.h"


//////////////////////////////////////////////////////////////////////////
//
// piLabels is the counter labels are numbered from (the generator's).
// Function sections have no options until a back end sets them.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackend::Init(CIRModule* pcModule, int* piLabels, CMallocator* pcMallocator)
{
	mpcModule = pcModule;
	mpiLabels = piLabels;
	mpcFunction = NULL;
	mpszOut = NULL;
	muiSource = 0;
	mbSourceMarkers = false;
	mcSourceMarkers.Init();
	mpcErrors = NULL;
	mbResult = true;
	maiBlockLabels.Init();
	maszRuntime.Init(pcMallocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBackend::Kill(void)
{
	maszRuntime.Kill();
	maiBlockLabels.Kill();
	mcSourceMarkers.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The runtime routines the functions written called (UseRuntime), for the
// generator to link or write.
//
//////////////////////////////////////////////////////////////////////////
CArrayChars* CIRBackend::GetRuntime(void)
{
	return &maszRuntime;
}


//////////////////////////////////////////////////////////////////////////
//
// Notes that the code calls the runtime routine szName (once each), for
// GetRuntime.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackend::UseRuntime(const char* szName)
{
	maszRuntime.AddIfUnique((char*)szName);
}


//////////////////////////////////////////////////////////////////////////
//
// The comparison a block's brif branches on when it is the block's last
// instruction (a back end branches on it without making the i1 when the
// brif is its only use), else NULL.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIRBackend::BranchComparison(CIRBlock* pcBlock)
{
	SIRInstruction*		psLast;
	const SIROpcode*	psOpcode;

	if ((pcBlock->msJump.uiKind != IRJ_Branch) || (pcBlock->masInstructions.NumElements() == 0))
	{
		return NULL;
	}
	psLast = pcBlock->masInstructions.Tail();
	psOpcode = GetIROpcode(psLast->uiOpcode);
	if (psOpcode && (psOpcode->uiResult == IRRS_I1) && psLast->sResult.IsTemporary() && psLast->sResult.Equals(pcBlock->msJump.sArgument))
	{
		return psLast;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The order the phis' copies on a jump (pasCopies) are written in, as one
// parallel copy (pasSteps): a copy is written once no copy still to be
// written reads its destination (CopyOverlaps); when only cycles are left,
// the value in the first copy's destination is saved and the copies that
// read it read the saved value.  The copies are used up.
//
//////////////////////////////////////////////////////////////////////////
bool CIRBackend::ParallelCopy(CArrayIRCopies* pasCopies, CArrayIRCopySteps* pasSteps)
{
	size			ui;
	size			uiOther;
	SIRCopy*		psCopy;
	SIRCopy*		psOther;
	SIRCopyStep*	psStep;
	SIRReference	sCycle;
	bool			bBlocked;
	bool			bProgress;
	bool			bLeft;

	pasSteps->FakeSetUsedElements(0);
	for (;;)
	{
		bProgress = false;
		bLeft = false;
		for (ui = 0; ui < pasCopies->NumElements(); ui++)
		{
			psCopy = pasCopies->Get(ui);
			if (psCopy->sSource.IsNone())
			{
				continue;
			}
			bLeft = true;

			//A copy to where its value is already changes nothing, and goes in its turn.
			bBlocked = false;
			for (uiOther = 0; !CopySamePlace(psCopy->sSource, psCopy->sDestination) && (uiOther < pasCopies->NumElements()); uiOther++)
			{
				psOther = pasCopies->Get(uiOther);
				if ((uiOther != ui) && !psOther->bSaved && !psOther->sSource.IsNone() && CopyOverlaps(psOther->sSource, psCopy->sDestination))
				{
					bBlocked = true;
					break;
				}
			}
			if (!bBlocked)
			{
				psStep = pasSteps->Add();
				psStep->bSave = false;
				psStep->sDestination = psCopy->sDestination;
				psStep->sSource = psCopy->sSource;
				psStep->sType = psCopy->sType;
				psStep->bSaved = psCopy->bSaved;
				psCopy->sSource = IRNone();
				bProgress = true;
			}
		}
		if (!bLeft)
		{
			return true;
		}
		if (!bProgress)
		{
			//Only cycles: the value in the first copy's destination is saved.
			psCopy = NULL;
			for (ui = 0; ui < pasCopies->NumElements(); ui++)
			{
				psCopy = pasCopies->Get(ui);
				if (!psCopy->sSource.IsNone())
				{
					break;
				}
			}
			if ((psCopy == NULL) || psCopy->sSource.IsNone())
			{
				return Error("Phi copies have no cycle to break.");
			}
			sCycle = IRNone();
			for (uiOther = 0; uiOther < pasCopies->NumElements(); uiOther++)
			{
				psOther = pasCopies->Get(uiOther);
				if (!psOther->sSource.IsNone() && CopyOverlaps(psOther->sSource, psCopy->sDestination))
				{
					if (sCycle.IsNone())
					{
						sCycle = psOther->sSource;
						psStep = pasSteps->Add();
						psStep->bSave = true;
						psStep->sDestination = IRNone();
						psStep->sSource = sCycle;
						psStep->sType = psOther->sType;
						psStep->bSaved = false;
					}
					if (CopySamePlace(psOther->sSource, sCycle))
					{
						psOther->bSaved = true;
					}
					else
					{
						return Error("Phi copies overlap.");
					}
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every function (each a text section of its own) and global (a data
// section) of the module, for the target's assembler (AssembleObject).
//
//////////////////////////////////////////////////////////////////////////
bool CIRBackend::GenerateModule(CChars* pszDest, CCPPErrors* pcErrors)
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
void CIRBackend::Binding(const char* szName, uint32 uiAttributes, CChars* pszDest)
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
void CIRBackend::GenerateGlobal(CIRGlobal* pcGlobal, CChars* pszDest)
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
// An error in the function being written ("@name: message"); the
// function's result is false.  Returns false.
//
//////////////////////////////////////////////////////////////////////////
bool CIRBackend::Error(const char* szMessage, const char* szDetail)
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
// The marker line (IR_SOURCE_MARKER) before the code of an instruction or
// a jump with uiSource (0 for none) when it isn't the last one's.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackend::Source(uint32 uiSource)
{
	if (!mbSourceMarkers || (uiSource == 0) || (uiSource == muiSource))
	{
		return;
	}
	muiSource = uiSource;
	mpszOut->Append(IR_SOURCE_MARKER);
	mpszOut->Append((int64)muiSource);
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether Source writes markers (for a listing): they are taken out while
// the text passes run, so the code is the same as without them.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackend::SetSourceMarkers(bool bSourceMarkers)
{
	mbSourceMarkers = bSourceMarkers;
}


//////////////////////////////////////////////////////////////////////////
//
// The next label number from the shared counter.
//
//////////////////////////////////////////////////////////////////////////
int32 CIRBackend::NewLabel(void)
{
	(*mpiLabels)++;
	return *mpiLabels;
}


//////////////////////////////////////////////////////////////////////////
//
// A label line, "L" and its number.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackend::Label(int32 iLabel)
{
	mpszOut->Append('L');
	mpszOut->Append(iLabel);
	mpszOut->Append(':');
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// The soft float runtime's routine (Libc65's softfloat.cpp) for a float
// operation, conversion or compare, without the label's "_": SoftFloat or
// SoftDouble (the float's type), then Add, Subtract, Multiply, Divide,
// Compare, ToDouble or ToFloat, or From or To an Int, Unsigned, LongLong or
// UnsignedLongLong (an integer narrower than 32 bits is an Int).  sFrom is
// the operand's type.  Both back ends call these.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackend::AppendSoftFloatName(CChars* psz, SIRInstruction* psInstruction, SIRType sFrom)
{
	SIRType	sTo;
	SIRType	sInteger;
	bool	bSigned;

	sTo = psInstruction->sType;
	switch (psInstruction->uiOpcode)
	{
	case IRO_FAdd:	psz->Append(sTo.Is(IRTK_F64) ? "SoftDoubleAdd" : "SoftFloatAdd");				return;
	case IRO_FSub:	psz->Append(sTo.Is(IRTK_F64) ? "SoftDoubleSubtract" : "SoftFloatSubtract");	return;
	case IRO_FMul:	psz->Append(sTo.Is(IRTK_F64) ? "SoftDoubleMultiply" : "SoftFloatMultiply");	return;
	case IRO_FDiv:	psz->Append(sTo.Is(IRTK_F64) ? "SoftDoubleDivide" : "SoftFloatDivide");		return;
	case IRO_FExt:
	case IRO_FTrunc:
		psz->Append(sTo.Is(IRTK_F64) ? "SoftFloatToDouble" : "SoftDoubleToFloat");
		return;
	case IRO_SIToF:
	case IRO_UIToF:
		bSigned = psInstruction->uiOpcode == IRO_SIToF;
		sInteger = sFrom;
		psz->Append(sTo.Is(IRTK_F64) ? "SoftDoubleFrom" : "SoftFloatFrom");
		break;
	case IRO_FToSI:
	case IRO_FToUI:
		bSigned = psInstruction->uiOpcode == IRO_FToSI;
		sInteger = sTo;
		psz->Append(sFrom.Is(IRTK_F64) ? "SoftDoubleTo" : "SoftFloatTo");
		break;
	default:
		//A compare: of the operands' type.
		psz->Append(sFrom.Is(IRTK_F64) ? "SoftDoubleCompare" : "SoftFloatCompare");
		return;
	}
	if (sInteger.Is(IRTK_I64))
	{
		psz->Append(bSigned ? "LongLong" : "UnsignedLongLong");
	}
	else
	{
		psz->Append((bSigned || (sInteger.IntegerBits() < 32)) ? "Int" : "Unsigned");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The soft float compare's results (-1, 0 or 1, or 2 when either is a NaN)
// that make the comparison uiOpcode true, in aiValues (one, or two in a
// row), or false when *pbNot (fne and fordered).  Returns how many.
//
//////////////////////////////////////////////////////////////////////////
int CIRBackend::FloatCompareValues(uint16 uiOpcode, int* aiValues, bool* pbNot)
{
	*pbNot = (uiOpcode == IRO_FNe) || (uiOpcode == IRO_FOrdered);
	switch (uiOpcode)
	{
	case IRO_FEq:
	case IRO_FNe:		aiValues[0] = 0;							return 1;
	case IRO_FLt:		aiValues[0] = -1;							return 1;
	case IRO_FGt:		aiValues[0] = 1;							return 1;
	case IRO_FLe:		aiValues[0] = -1;	aiValues[1] = 0;		return 2;
	case IRO_FGe:		aiValues[0] = 0;	aiValues[1] = 1;		return 2;
	default:			aiValues[0] = 2;							return 1;	//fordered and funordered: a NaN.
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether two comparisons have the same two operands, in either order (eq
// and ne give the same either way round).
//
//////////////////////////////////////////////////////////////////////////
bool CIRBackend::SameOperands(SIRInstruction* psFirst, SIRInstruction* psSecond)
{
	return (psFirst->asArguments[0].Equals(psSecond->asArguments[0]) && psFirst->asArguments[1].Equals(psSecond->asArguments[1])) ||
		   (psFirst->asArguments[0].Equals(psSecond->asArguments[1]) && psFirst->asArguments[1].Equals(psSecond->asArguments[0]));
}


//////////////////////////////////////////////////////////////////////////
//
// The signed digits of ulliMultiplier (iBits wide): its non-adjacent form,
// digits of +1 and -1 never two next to each other, lowest first, their
// places in aiPlaces and signs in aiSigns.  Digits past the top bit are 0.
// Returns how many, or -1 for more than IR_MAX_MULTIPLY_DIGITS.
//
//////////////////////////////////////////////////////////////////////////
int CIRBackend::SignedDigits(uint64 ulliMultiplier, int iBits, int* aiPlaces, int* aiSigns)
{
	int		iDigits;
	int		iPlace;

	iDigits = 0;
	for (iPlace = 0; (ulliMultiplier != 0) && (iPlace < iBits); iPlace++)
	{
		if (ulliMultiplier & 1)
		{
			if (iDigits == IR_MAX_MULTIPLY_DIGITS)
			{
				return -1;
			}
			aiPlaces[iDigits] = iPlace;
			aiSigns[iDigits] = ((ulliMultiplier & 3) == 3) ? -1 : 1;
			ulliMultiplier = (aiSigns[iDigits] < 0) ? ulliMultiplier + 1 : ulliMultiplier - 1;
			iDigits++;
		}
		ulliMultiplier >>= 1;
	}
	return iDigits;
}


//////////////////////////////////////////////////////////////////////////
//
// A mul by a constant (either side) is written as shifts and adds when the
// target says they cost less (ShiftAddCost against MultiplyCost): x times
// the constant's signed digits (SignedDigits, top first) Horner's way:
// acc = x (or -x), then for each digit acc = (acc << the gap) + x (or - x),
// and the last acc << the lowest digit's place.  Multiplies by powers of 2,
// 0 and 1 were already made shifts by the optimiser (FoldConstants).
//
//////////////////////////////////////////////////////////////////////////
void CIRBackend::ExpandMultiplies(void)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRReference	sValue;
	SIRReference	sAcc;
	SIRType			sType;
	uint64			ulliConstant;
	int				aiPlaces[IR_MAX_MULTIPLY_DIGITS];
	int				aiSigns[IR_MAX_MULTIPLY_DIGITS];
	int				iDigits;
	int				i;
	size			ui;
	size			uiIndex;
	bool			bDone;

	for (ui = 0; ui < mpcFunction->NumBlocks(); ui++)
	{
		pcBlock = mpcFunction->GetBlock(ui);
		for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiIndex);
			sType = psInstruction->sType;
			if ((psInstruction->uiOpcode != IRO_Mul) || !psInstruction->sResult.IsTemporary() || !sType.IsInteger() || sType.Is(IRTK_I1) ||
				(psInstruction->asArguments[0].IsConstant() == psInstruction->asArguments[1].IsConstant()))
			{
				continue;
			}
			i = psInstruction->asArguments[0].IsConstant() ? 0 : 1;
			ulliConstant = IRTruncate(sType, mpcFunction->GetConstant(psInstruction->asArguments[i])->ulliBits);
			sValue = psInstruction->asArguments[1 - i];

			iDigits = SignedDigits(ulliConstant, sType.IntegerBits(), aiPlaces, aiSigns);
			if ((iDigits < 2) || (ShiftAddCost(sType, aiPlaces, aiSigns, iDigits) >= MultiplyCost(sType, ulliConstant)))
			{
				continue;
			}

			//The new instructions go before the mul, which becomes the last of them (keeping its result).
			sAcc = sValue;
			if (aiSigns[iDigits - 1] < 0)
			{
				sAcc = InsertBefore(pcBlock, &uiIndex, IRO_Neg, sType, sAcc, IRNone());
			}
			bDone = false;
			for (i = iDigits - 1; i > 0; i--)
			{
				sAcc = InsertBefore(pcBlock, &uiIndex, IRO_Shl, sType, sAcc, mpcFunction->AddInteger(sType, aiPlaces[i] - aiPlaces[i - 1]));
				if ((i == 1) && (aiPlaces[0] == 0))
				{
					//The last digit is at bit 0: its add is the mul's place.
					psInstruction = pcBlock->masInstructions.Get(uiIndex);
					psInstruction->uiOpcode = (aiSigns[0] > 0) ? IRO_Add : IRO_Sub;
					psInstruction->asArguments[0] = sAcc;
					psInstruction->asArguments[1] = sValue;
					bDone = true;
					break;
				}
				sAcc = InsertBefore(pcBlock, &uiIndex, (aiSigns[i - 1] > 0) ? IRO_Add : IRO_Sub, sType, sAcc, sValue);
			}
			if (!bDone)
			{
				//acc << the lowest digit's place is the mul's place.
				psInstruction = pcBlock->masInstructions.Get(uiIndex);
				psInstruction->uiOpcode = IRO_Shl;
				psInstruction->asArguments[0] = sAcc;
				psInstruction->asArguments[1] = mpcFunction->AddInteger(sType, aiPlaces[0]);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A new instruction before pcBlock's instruction *puiIndex, which moves up
// one (*puiIndex follows it).  Returns its result.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRBackend::InsertBefore(CIRBlock* pcBlock, size* puiIndex, uint16 uiOpcode, SIRType sType, SIRReference sFirst, SIRReference sSecond)
{
	SIRInstruction	sMoved;
	SIRReference	sResult;
	size			uiLast;

	sResult = mpcFunction->Add(pcBlock, uiOpcode, sType, sFirst, sSecond);
	uiLast = pcBlock->masInstructions.NumElements() - 1;
	sMoved = *pcBlock->masInstructions.Get(uiLast);
	pcBlock->masInstructions.RemoveAt(uiLast, true);
	pcBlock->masInstructions.InsertAt(&sMoved, *puiIndex);
	(*puiIndex)++;
	return sResult;
}

