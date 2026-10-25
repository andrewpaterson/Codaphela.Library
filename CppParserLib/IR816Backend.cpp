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
#include <stdlib.h>
#include <string.h>
#include "IRParser.h"
#include "IR816Backend.h"


//The runtime routines, as IR for each width: $T is the type, $N its bits, $M its bits - 1 and $NAME the routine
//(R816_$NAME).  Multiply adds and shifts; divide is restoring division a bit at a time (a remainder that shifts
//past the top is bigger than any divisor); signed divide and remainder work on the magnitudes.
static const char* gszIR816Mul =
	"function $T @R816_$NAME($T %a, $T %b)\n{\n"
	"entry:\n\tjmp loop\n"
	"loop:\n\t%r = phi $T [entry, 0], [next, %r2]\n\t%x = phi $T [entry, %a], [next, %x2]\n\t%y = phi $T [entry, %b], [next, %y2]\n"
	"\t%z = eq $T %y, 0\n\tbrif %z, done, body\n"
	"body:\n\t%bit = and $T %y, 1\n\t%odd = ne $T %bit, 0\n\tbrif %odd, add, keep\n"
	"add:\n\t%ra = add $T %r, %x\n\tjmp next\n"
	"keep:\n\tjmp next\n"
	"next:\n\t%r2 = phi $T [add, %ra], [keep, %r]\n\t%x2 = shl $T %x, 1\n\t%y2 = lshr $T %y, 1\n\tjmp loop\n"
	"done:\n\tret $T %r\n}\n\n";

static const char* gszIR816Divide =
	"function $T @R816_$NAME($T %n, $T %d)\n{\n"
	"entry:\n\tjmp loop\n"
	"loop:\n\t%i = phi i16 [entry, $N], [next, %i2]\n\t%q = phi $T [entry, 0], [next, %q2]\n\t%r = phi $T [entry, 0], [next, %r2]\n\t%m = phi $T [entry, %n], [next, %m1]\n"
	"\t%done = eq i16 %i, 0\n\tbrif %done, exit, body\n"
	"body:\n\t%top = lshr $T %m, $M\n\t%rs = shl $T %r, 1\n\t%rb = or $T %rs, %top\n\t%m1 = shl $T %m, 1\n\t%qs = shl $T %q, 1\n"
	"\t%rtop = lshr $T %r, $M\n\t%over = ne $T %rtop, 0\n\tbrif %over, sub, test\n"
	"test:\n\t%ge = uge $T %rb, %d\n\tbrif %ge, sub, keep\n"
	"sub:\n\t%rd = sub $T %rb, %d\n\t%qo = or $T %qs, 1\n\tjmp next\n"
	"keep:\n\tjmp next\n"
	"next:\n\t%r2 = phi $T [sub, %rd], [keep, %rb]\n\t%q2 = phi $T [sub, %qo], [keep, %qs]\n\t%i2 = sub i16 %i, 1\n\tjmp loop\n"
	"exit:\n\tret $T $RESULT\n}\n\n";

static const char* gszIR816Signed =
	"function $T @R816_$NAME($T %a, $T %b)\n{\n"
	"entry:\n\t%an = slt $T %a, 0\n\tbrif %an, negA, posA\n"
	"negA:\n\t%na = sub $T 0, %a\n\tjmp gotA\n"
	"posA:\n\tjmp gotA\n"
	"gotA:\n\t%ua = phi $T [negA, %na], [posA, %a]\n\t%bn = slt $T %b, 0\n\tbrif %bn, negB, posB\n"
	"negB:\n\t%nb = sub $T 0, %b\n\tjmp gotB\n"
	"posB:\n\tjmp gotB\n"
	"gotB:\n\t%ub = phi $T [negB, %nb], [posB, %b]\n\t%u = call $T @R816_U$UNAME($T %ua, $T %ub)\n"
	"\t%s = $SIGN\n\tbrif %s, neg, pos\n"
	"neg:\n\t%nu = sub $T 0, %u\n\tret $T %nu\n"
	"pos:\n\tret $T %u\n}\n\n";


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Init(CIRModule* pcModule, int* piLabels)
{
	mpcModule = pcModule;
	mpiLabels = piLabels;
	maszRuntime.Init();
	mpcFunction = NULL;
	mpszOut = NULL;
	mpcErrors = NULL;
	mbResult = true;
	maiOffsets.Init();
	maiIncoming.Init();
	maiSlots.Init();
	maiBlockLabels.Init();
	maiParameters.Init();
	maiPendingArguments.Init();
	miFrameBytes = 0;
	miHiddenResult = -1;
	mszSectionOptions = "";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Kill(void)
{
	maiPendingArguments.Kill();
	maiParameters.Kill();
	maiBlockLabels.Kill();
	maiSlots.Kill();
	maiIncoming.Kill();
	maiOffsets.Kill();
	maszRuntime.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each function and global a section of its own.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::GenerateModule(CChars* pszDest, CCPPErrors* pcErrors)
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
		pszDest->Append(mszSectionOptions);
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
// The runtime routines GenerateModule's functions called.
//
//////////////////////////////////////////////////////////////////////////
CArrayChars* CIR816Backend::GetRuntime(void)
{
	return &maszRuntime;
}


//////////////////////////////////////////////////////////////////////////
//
// .global or .weak, or nothing for the unit's own label.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Binding(const char* szName, uint32 uiAttributes, CChars* pszDest)
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
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::GenerateGlobal(CIRGlobal* pcGlobal, CChars* pszDest)
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
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::Error(const char* szMessage, const char* szDetail)
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
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Line(const char* szText)
{
	mpszOut->Append("\t\t");
	mpszOut->Append(szText);
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// szText followed by a number.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Line(const char* szText, int64 lliValue)
{
	mpszOut->Append("\t\t");
	mpszOut->Append(szText);
	mpszOut->Append(lliValue);
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CIR816Backend::NewLabel(void)
{
	(*mpiLabels)++;
	return *mpiLabels;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Label(int32 iLabel)
{
	mpszOut->Append('L');
	mpszOut->Append(iLabel);
	mpszOut->Append(':');
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::UseRuntime(const char* szName)
{
	size	i;

	for (i = 0; i < maszRuntime.NumElements(); i++)
	{
		if (strcmp(maszRuntime.GetText(i), szName) == 0)
		{
			return;
		}
	}
	maszRuntime.Add(szName);
}


//////////////////////////////////////////////////////////////////////////
//
// A value's place in the frame: a word for i1 to i16 (zero extended), two for 32 bits, four for 64.
//
//////////////////////////////////////////////////////////////////////////
int CIR816Backend::ValueBytes(SIRType sType)
{
	switch (sType.uiKind)
	{
	case IRTK_I1:
	case IRTK_I8:
	case IRTK_I16:
		return 2;
	case IRTK_I32:
	case IRTK_F32:
		return 4;
	case IRTK_I64:
	case IRTK_F64:
		return 8;
	case IRTK_Pointer:
		if (mpcModule->GetPointerSize(sType.uiSpace) != 4)
		{
			Error("Only 4 byte pointers are done yet");
		}
		return 4;
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIR816Backend::Words(SIRType sType)
{
	return ValueBytes(sType) / 2;
}


//////////////////////////////////////////////////////////////////////////
//
// A 64 bit result is written through a hidden pointer.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::IsHiddenResult(SIRType sType)
{
	return ValueBytes(sType) == 8;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CIR816Backend::Allocate(int32* piNext, int32 iBytes)
{
	int32	iOffset;

	iOffset = *piNext;
	*piNext += (iBytes + 1) & ~1;
	return iOffset;
}


//////////////////////////////////////////////////////////////////////////
//
// Every temporary, phi's way in and slot gets its own place after the scratch bytes.  The arguments are above the
// frame, the saved D and the return address: the hidden result pointer first, then the parameters in order.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Layout(void)
{
	size				ui;
	size				uiBlock;
	size				uiInstruction;
	int32				iNext;
	int32				iNone;
	int32				iArgument;
	CIRBlock*			pcBlock;
	SIRPhi*				psPhi;
	SIRInstruction*		psInstruction;
	SIRReference		sParameter;
	int32				iBytes;

	iNone = -1;
	maiOffsets.FakeSetUsedElements(0);
	maiIncoming.FakeSetUsedElements(0);
	maiSlots.FakeSetUsedElements(0);
	maiParameters.FakeSetUsedElements(0);
	for (ui = 0; ui < mpcFunction->NumTemporaries(); ui++)
	{
		maiOffsets.Add(&iNone);
		maiIncoming.Add(&iNone);
		maiSlots.Add(&iNone);
	}

	iNext = IR816_FIRST_TEMPORARY;
	for (ui = 0; ui < mpcFunction->NumTemporaries(); ui++)
	{
		iBytes = ValueBytes(mpcFunction->GetTemporaryType(IRReference(IRRK_Temporary, (uint32)ui)));
		if (iBytes != 0)
		{
			*maiOffsets.Get(ui) = Allocate(&iNext, iBytes);
		}
	}
	for (uiBlock = 0; uiBlock < mpcFunction->NumBlocks(); uiBlock++)
	{
		pcBlock = mpcFunction->GetBlock(uiBlock);
		for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
		{
			psPhi = pcBlock->masPhis.Get(ui);
			*maiIncoming.Get(psPhi->sResult.Index()) = Allocate(&iNext, ValueBytes(psPhi->sType));
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (psInstruction->uiOpcode == IRO_Slot)
			{
				*maiSlots.Get(psInstruction->sResult.Index()) = Allocate(&iNext, (int32)mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits);
			}
		}
	}
	miFrameBytes = iNext - 1;

	iArgument = miFrameBytes + 1 + IR816_LINK_BYTES;
	miHiddenResult = -1;
	if (IsHiddenResult(mpcFunction->msReturnType))
	{
		miHiddenResult = iArgument;
		iArgument += 4;
	}
	for (ui = 0; ui < mpcFunction->masParameters.NumElements(); ui++)
	{
		sParameter = *mpcFunction->masParameters.Get(ui);
		maiParameters.Add(&iArgument);
		*maiOffsets.Get(sParameter.Index()) = iArgument;
		iArgument += ValueBytes(mpcFunction->GetTemporaryType(sParameter));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CIR816Backend::OffsetOf(SIRReference sTemporary)
{
	return *maiOffsets.Get(sTemporary.Index());
}


//////////////////////////////////////////////////////////////////////////
//
// szMnemonic on the frame's word at iOffset: [D + offset], or through X past the direct page's 256 bytes.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Frame(const char* szMnemonic, int32 iOffset)
{
	mpszOut->Append("\t\t");
	if (iOffset <= 255)
	{
		mpszOut->Append(szMnemonic);
		mpszOut->Append(" [D + ");
		mpszOut->Append(iOffset);
		mpszOut->Append("]");
	}
	else
	{
		mpszOut->Append("ldx ");
		mpszOut->Append(iOffset);
		mpszOut->AppendNewLine();
		mpszOut->Append("\t\t");
		mpszOut->Append(szMnemonic);
		mpszOut->Append(" [D + X]");
	}
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// szMnemonic with word iWord of a temporary, constant or symbol.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Operand(const char* szMnemonic, SIRReference sValue, int iWord)
{
	uint16	uiWord;
	char*	szName;

	if (sValue.IsTemporary())
	{
		if (iWord < Words(mpcFunction->GetTemporaryType(sValue)))
		{
			Frame(szMnemonic, OffsetOf(sValue) + 2 * iWord);
			return;
		}
		uiWord = 0;
	}
	else if (sValue.IsConstant())
	{
		uiWord = (iWord < 4) ? (uint16)(mpcFunction->GetConstant(sValue)->ulliBits >> (16 * iWord)) : 0;
	}
	else if (sValue.IsSymbol())
	{
		szName = mpcModule->GetSymbolName(sValue.Index());
		mpszOut->Append("\t\t");
		mpszOut->Append(szMnemonic);
		if (iWord == 0)
		{
			mpszOut->Append(" ");
			mpszOut->Append(szName);
			mpszOut->Append(" & $FFFF");
		}
		else if (iWord == 1)
		{
			mpszOut->Append(" ");
			mpszOut->Append(szName);
			mpszOut->Append(" >> 16");
		}
		else
		{
			mpszOut->Append(" 0");
		}
		mpszOut->AppendNewLine();
		return;
	}
	else
	{
		uiWord = 0;
	}
	mpszOut->Append("\t\t");
	mpszOut->Append(szMnemonic);
	mpszOut->Append(" ");
	mpszOut->Append((int32)uiWord);
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Store(SIRReference sResult, int iWord)
{
	Frame("sta", OffsetOf(sResult) + 2 * iWord);
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps an i8 or i1 in A zero extended.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Mask(SIRType sType)
{
	if (sType.Is(IRTK_I8))
	{
		Line("and $FF");
	}
	else if (sType.Is(IRTK_I1))
	{
		Line("and 1");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::CopyWords(int32 iDestination, SIRReference sValue, int iWords)
{
	int		i;

	for (i = 0; i < iWords; i++)
	{
		Operand("lda", sValue, i);
		Frame("sta", iDestination + 2 * i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::ZeroWords(int32 iDestination, int iFirst, int iWords)
{
	int		i;

	for (i = 0; i < iWords; i++)
	{
		Frame("stz", iDestination + 2 * (iFirst + i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Fills iWords words from iFirst with the sign of the word before them.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FillSign(int32 iDestination, int iFirst, int iWords)
{
	int		i;

	if (iWords <= 0)
	{
		return;
	}
	Frame("lda", iDestination + 2 * (iFirst - 1));
	Line("asl A");
	Line("lda 0");
	Line("adc $FFFF");
	Line("eor $FFFF");
	for (i = 0; i < iWords; i++)
	{
		Frame("sta", iDestination + 2 * (iFirst + i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes a value's words, the high word first so it is little endian on the stack.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PushValue(SIRReference sValue, SIRType sType)
{
	int		i;

	for (i = Words(sType) - 1; i >= 0; i--)
	{
		if (sValue.IsTemporary())
		{
			Operand("lda", sValue, i);
			Line("pha");
		}
		else
		{
			Operand("pea", sValue, i);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the 24 bit address (in bank 0) of the frame's byte at iOffset.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PushFrameAddress(int32 iOffset)
{
	Line("pea 0");
	Line("tdc");
	Line("clc");
	Line("adc ", iOffset);
	Line("pha");
}


//////////////////////////////////////////////////////////////////////////
//
// Where a load or store's pointer is: a symbol (long addresses) or a place in the direct page ([d],y needs it within
// the first 256 bytes, else it is copied to the scratch pointer).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PointerPlace(SIRReference sPointer, int32* piPlace, char** pszSymbol)
{
	*pszSymbol = NULL;
	*piPlace = IR816_SCRATCH_POINTER;
	if (sPointer.IsSymbol())
	{
		*pszSymbol = mpcModule->GetSymbolName(sPointer.Index());
		return;
	}
	if (sPointer.IsTemporary() && (OffsetOf(sPointer) <= 253))
	{
		*piPlace = OffsetOf(sPointer);
		return;
	}
	CopyWords(IR816_SCRATCH_POINTER, sPointer, 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::GenerateFunction(CIRFunction* pcFunction, const char* szLabel, CChars* pszDest, CCPPErrors* pcErrors)
{
	size		ui;
	size		uiInstruction;
	CIRBlock*	pcBlock;
	CIRBlock*	pcNext;
	int32		iLabel;

	mpcFunction = pcFunction;
	mpszOut = pszDest;
	mpcErrors = pcErrors;
	mbResult = true;

	pcFunction->Renumber();
	Layout();
	maiBlockLabels.FakeSetUsedElements(0);
	for (ui = 0; ui < pcFunction->NumBlocks(); ui++)
	{
		iLabel = NewLabel();
		maiBlockLabels.Add(&iLabel);
	}

	Prologue(szLabel);
	for (ui = 0; ui < pcFunction->NumBlocks(); ui++)
	{
		pcBlock = pcFunction->GetBlock(ui);
		pcNext = (ui + 1 < pcFunction->NumBlocks()) ? pcFunction->GetBlock(ui + 1) : NULL;
		Label(*maiBlockLabels.Get(ui));
		PhiCopies(pcBlock);
		maiPendingArguments.FakeSetUsedElements(0);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			Instruction(pcBlock, uiInstruction);
		}
		Jump(pcBlock, pcNext);
	}
	pszDest->AppendNewLine();
	return mbResult;
}


//////////////////////////////////////////////////////////////////////////
//
// D is pointed at the frame: the frame is D + 1 to D + frame size.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Prologue(const char* szLabel)
{
	mpszOut->Append(szLabel);
	mpszOut->Append(':');
	mpszOut->AppendNewLine();
	Line(".width (A, X), 16");
	Line("phd");
	Line("tsc");
	Line("sec");
	Line("sbc ", miFrameBytes);
	Line("tcs");
	Line("tcd");
}


//////////////////////////////////////////////////////////////////////////
//
// The result is in A (and X); Y keeps A while S is moved back.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Epilogue(void)
{
	Line("tay");
	Line("tdc");
	Line("clc");
	Line("adc ", miFrameBytes);
	Line("tcs");
	Line("tya");
	Line("pld");
	Line("rtl");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Return(CIRBlock* pcBlock)
{
	SIRJump*	psJump;
	int			i;
	int			iWords;

	psJump = &pcBlock->msJump;
	if (!psJump->sType.IsNone())
	{
		iWords = Words(psJump->sType);
		if (miHiddenResult >= 0)
		{
			Frame("lda", miHiddenResult);
			Frame("sta", IR816_SCRATCH_POINTER);
			Frame("lda", miHiddenResult + 2);
			Frame("sta", IR816_SCRATCH_POINTER + 2);
			for (i = 0; i < iWords; i++)
			{
				Line("ldy ", 2 * i);
				Operand("lda", psJump->sArgument, i);
				Line("sta [[D + 1]:24 + Y]");
			}
		}
		else
		{
			CopyWords(IR816_SCRATCH_RESULT, psJump->sArgument, iWords);
			Frame("lda", IR816_SCRATCH_RESULT);
			if (iWords > 1)
			{
				Frame("ldx", IR816_SCRATCH_RESULT + 2);
			}
		}
	}
	Epilogue();
}


//////////////////////////////////////////////////////////////////////////
//
// A jump into a block with phis puts each phi's value in its way in; the block copies them to the phis as it starts,
// so phis that take each other's values (a swap) are all read before any is written.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::EdgeCopies(CIRBlock* pcFrom, CIRBlock* pcTo)
{
	size			ui;
	SIRPhi*			psPhi;
	SIRPhiArgument*	psArgument;

	for (ui = 0; ui < pcTo->masPhis.NumElements(); ui++)
	{
		psPhi = pcTo->masPhis.Get(ui);
		psArgument = pcTo->FindPhiArgument(psPhi, pcFrom);
		if (psArgument)
		{
			CopyWords(*maiIncoming.Get(psPhi->sResult.Index()), psArgument->sValue, Words(psPhi->sType));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PhiCopies(CIRBlock* pcBlock)
{
	size		ui;
	SIRPhi*		psPhi;
	int			i;
	int32		iIncoming;

	for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
	{
		psPhi = pcBlock->masPhis.Get(ui);
		iIncoming = *maiIncoming.Get(psPhi->sResult.Index());
		for (i = 0; i < Words(psPhi->sType); i++)
		{
			Frame("lda", iIncoming + 2 * i);
			Store(psPhi->sResult, i);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::JumpTo(CIRBlock* pcTo, CIRBlock* pcNext)
{
	CChars	sz;

	if (pcTo == pcNext)
	{
		return;
	}
	sz.Init("brl L");
	sz.Append(*maiBlockLabels.Get(pcTo->muiIndex));
	Line(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A brif skips a brl to its else edge when the condition isn't 0, so neither edge is limited to a short branch.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Jump(CIRBlock* pcBlock, CIRBlock* pcNext)
{
	SIRJump*	psJump;
	int32		iElse;
	CChars		sz;

	psJump = &pcBlock->msJump;
	switch (psJump->uiKind)
	{
	case IRJ_Jump:
		EdgeCopies(pcBlock, psJump->pcThen);
		JumpTo(psJump->pcThen, pcNext);
		break;
	case IRJ_Branch:
		iElse = NewLabel();
		Operand("lda", psJump->sArgument, 0);
		Line("bne C + 3");
		sz.Init("brl L");
		sz.Append(iElse);
		Line(sz.Text());
		sz.Kill();
		EdgeCopies(pcBlock, psJump->pcThen);
		JumpTo(psJump->pcThen, NULL);
		Label(iElse);
		EdgeCopies(pcBlock, psJump->pcElse);
		JumpTo(psJump->pcElse, pcNext);
		break;
	case IRJ_Return:
		Return(pcBlock);
		break;
	default:
		Line("stp");
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::Instruction(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	int				i;

	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	switch (psInstruction->uiOpcode)
	{
	case IRO_Add:
	case IRO_PtrAdd:
		Arithmetic(psInstruction, "clc", "adc");
		return true;
	case IRO_Sub:
		Arithmetic(psInstruction, "sec", "sbc");
		return true;
	case IRO_And:
		Logic(psInstruction, "and", 0);
		return true;
	case IRO_Or:
		Logic(psInstruction, "ora", 0);
		return true;
	case IRO_Xor:
		Logic(psInstruction, "eor", 0);
		return true;
	case IRO_Not:
		Logic(psInstruction, "eor", 0xFFFFFFFFFFFFFFFFULL);
		return true;
	case IRO_Neg:
		Line("sec");
		for (i = 0; i < Words(psInstruction->sType); i++)
		{
			Line("lda 0");
			Operand("sbc", psInstruction->asArguments[0], i);
			if (Words(psInstruction->sType) == 1)
			{
				Mask(psInstruction->sType);
			}
			Store(psInstruction->sResult, i);
		}
		return true;
	case IRO_Copy:
		CopyWords(OffsetOf(psInstruction->sResult), psInstruction->asArguments[0], Words(psInstruction->sType));
		return true;
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
		Shift(psInstruction);
		return true;
	case IRO_Mul:
	case IRO_UDiv:
	case IRO_SDiv:
	case IRO_URem:
	case IRO_SRem:
		return RuntimeCall(psInstruction);
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
		Compare(psInstruction);
		return true;
	case IRO_SExt:
	case IRO_ZExt:
	case IRO_Trunc:
	case IRO_PtrToI:
	case IRO_IToPtr:
	case IRO_SpaceCast:
		Extend(psInstruction);
		return true;
	case IRO_Slot:
		Address(psInstruction, *maiSlots.Get(psInstruction->sResult.Index()));
		return true;
	case IRO_ArgAddress:
		Address(psInstruction, *maiParameters.Get((size)mpcFunction->GetConstant(psInstruction->asArguments[0])->ulliBits));
		return true;
	case IRO_Load:
		Load(psInstruction);
		return true;
	case IRO_Store:
		StoreMemory(psInstruction);
		return true;
	case IRO_Arg:
		maiPendingArguments.Add(&uiIndex);
		return true;
	case IRO_Call:
		Call(pcBlock, psInstruction);
		return true;
	case IRO_FAdd:
	case IRO_FSub:
	case IRO_FMul:
	case IRO_FDiv:
		FloatArithmetic(psInstruction);
		return true;
	case IRO_FNeg:
		CopyWords(OffsetOf(psInstruction->sResult), psInstruction->asArguments[0], Words(psInstruction->sType));
		Frame("lda", OffsetOf(psInstruction->sResult) + 2 * (Words(psInstruction->sType) - 1));
		Line("eor $8000");
		Frame("sta", OffsetOf(psInstruction->sResult) + 2 * (Words(psInstruction->sType) - 1));
		return true;
	case IRO_FEq:
	case IRO_FNe:
	case IRO_FLt:
	case IRO_FLe:
	case IRO_FGt:
	case IRO_FGe:
	case IRO_FOrdered:
	case IRO_FUnordered:
		FloatCompare(psInstruction);
		return true;
	case IRO_SIToF:
	case IRO_UIToF:
	case IRO_FToSI:
	case IRO_FToUI:
	case IRO_FExt:
	case IRO_FTrunc:
		FloatConversion(psInstruction);
		return true;
	default:
		return Error("The W65C816 back end doesn't do this instruction yet", GetIROpcode(psInstruction->uiOpcode)->szName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The soft float runtime (Libc65's softfloat.cpp, compiled for the W65C816): _SoftFloat<operation> for f32 and
// _SoftDouble<operation> for f64.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FloatArithmetic(SIRInstruction* psInstruction)
{
	CChars	szName;

	szName.Init(psInstruction->sType.Is(IRTK_F64) ? "_SoftDouble" : "_SoftFloat");
	switch (psInstruction->uiOpcode)
	{
	case IRO_FAdd:	szName.Append("Add");		break;
	case IRO_FSub:	szName.Append("Subtract");	break;
	case IRO_FMul:	szName.Append("Multiply");	break;
	default:		szName.Append("Divide");	break;
	}
	PushValue(psInstruction->asArguments[1], psInstruction->sType);
	PushValue(psInstruction->asArguments[0], psInstruction->sType);
	CallFunction(szName.Text(), 2 * ValueBytes(psInstruction->sType), psInstruction);
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the hidden result pointer when the result is 64 bits, calls szName and removes the iBytes of arguments
// already pushed (and the pointer), leaving the result in its place.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::CallFunction(const char* szName, int32 iBytes, SIRInstruction* psInstruction)
{
	CChars	sz;

	UseRuntime(szName);
	if (!psInstruction->sResult.IsNone() && IsHiddenResult(psInstruction->sType))
	{
		PushFrameAddress(OffsetOf(psInstruction->sResult));
		iBytes += 4;
	}
	sz.Init("jsl ");
	sz.Append(szName);
	Line(sz.Text());
	sz.Kill();
	CallerCleanUp(iBytes, psInstruction);
}


//////////////////////////////////////////////////////////////////////////
//
// _SoftFloatCompare gives -1, 0 or 1, or 2 when either is a NaN, in A: each comparison is true for some of those.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FloatCompare(SIRInstruction* psInstruction)
{
	int					aiValues[2];
	int					iValues;
	bool				bNot;
	int32				iTrue;
	int32				iFalse;
	int32				iEnd;
	int					i;
	CChars				sz;
	SIRType				sType;

	sType = psInstruction->sType;
	PushValue(psInstruction->asArguments[1], sType);
	PushValue(psInstruction->asArguments[0], sType);
	UseRuntime(sType.Is(IRTK_F64) ? "_SoftDoubleCompare" : "_SoftFloatCompare");
	Line(sType.Is(IRTK_F64) ? "jsl _SoftDoubleCompare" : "jsl _SoftFloatCompare");
	Frame("sta", IR816_SCRATCH_RESULT);
	Line("tsc");
	Line("clc");
	Line("adc ", 2 * ValueBytes(sType));
	Line("tcs");

	//The compare's results that make it true (or false, for fne and fordered).
	bNot = false;
	iValues = 1;
	switch (psInstruction->uiOpcode)
	{
	case IRO_FEq:		aiValues[0] = 0;	break;
	case IRO_FNe:		aiValues[0] = 0;	bNot = true;	break;
	case IRO_FLt:		aiValues[0] = -1;	break;
	case IRO_FGt:		aiValues[0] = 1;	break;
	case IRO_FLe:		aiValues[0] = -1;	aiValues[1] = 0;	iValues = 2;	break;
	case IRO_FGe:		aiValues[0] = 0;	aiValues[1] = 1;	iValues = 2;	break;
	case IRO_FOrdered:	aiValues[0] = 2;	bNot = true;	break;
	default:			aiValues[0] = 2;	break;
	}

	iTrue = NewLabel();
	iFalse = NewLabel();
	iEnd = NewLabel();
	sz.Init();
	Frame("lda", IR816_SCRATCH_RESULT);
	for (i = 0; i < iValues; i++)
	{
		Line("cmp ", aiValues[i] & 0xFFFF);
		sz.Clear();
		sz.Append("beq L");
		sz.Append(bNot ? iFalse : iTrue);
		Line(sz.Text());
	}
	sz.Clear();
	sz.Append("bra L");
	sz.Append(bNot ? iTrue : iFalse);
	Line(sz.Text());
	Label(iTrue);
	Line("lda 1");
	sz.Clear();
	sz.Append("bra L");
	sz.Append(iEnd);
	Line(sz.Text());
	Label(iFalse);
	Line("lda 0");
	Label(iEnd);
	Store(psInstruction->sResult, 0);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Conversions with floats: an integer narrower than 32 bits converts as an int32_t (sign or zero extended), a 64
// bit one as a long long.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::FloatConversion(SIRInstruction* psInstruction)
{
	SIRType			sFrom;
	SIRType			sTo;
	SIRReference	sValue;
	bool			bSigned;
	CChars			szName;
	int32			iBytes;

	sValue = psInstruction->asArguments[0];
	sFrom = mpcFunction->TypeOf(sValue);
	sTo = psInstruction->sType;
	szName.Init();
	switch (psInstruction->uiOpcode)
	{
	case IRO_FExt:
	case IRO_FTrunc:
		szName.Append(sTo.Is(IRTK_F64) ? "_SoftFloatToDouble" : "_SoftDoubleToFloat");
		PushValue(sValue, sFrom);
		iBytes = ValueBytes(sFrom);
		break;
	case IRO_SIToF:
	case IRO_UIToF:
		bSigned = psInstruction->uiOpcode == IRO_SIToF;
		szName.Append(sTo.Is(IRTK_F64) ? "_SoftDoubleFrom" : "_SoftFloatFrom");
		if (sFrom.Is(IRTK_I64))
		{
			szName.Append(bSigned ? "LongLong" : "UnsignedLongLong");
			PushValue(sValue, sFrom);
			iBytes = 8;
		}
		else
		{
			szName.Append((bSigned || (sFrom.IntegerBits() < 32)) ? "Int" : "Unsigned");
			PushInt32(sValue, sFrom, bSigned);
			iBytes = 4;
		}
		break;
	default:
		bSigned = psInstruction->uiOpcode == IRO_FToSI;
		szName.Append(sFrom.Is(IRTK_F64) ? "_SoftDoubleTo" : "_SoftFloatTo");
		if (sTo.Is(IRTK_I64))
		{
			szName.Append(bSigned ? "LongLong" : "UnsignedLongLong");
		}
		else
		{
			szName.Append((bSigned || (sTo.IntegerBits() < 32)) ? "Int" : "Unsigned");
		}
		PushValue(sValue, sFrom);
		iBytes = ValueBytes(sFrom);
		break;
	}
	CallFunction(szName.Text(), iBytes, psInstruction);
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes an integer of up to 32 bits as 32 bits, sign or zero extended.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::PushInt32(SIRReference sValue, SIRType sType, bool bSigned)
{
	if (Words(sType) == 2)
	{
		PushValue(sValue, sType);
		return;
	}
	Operand("lda", sValue, 0);
	if (bSigned && sType.Is(IRTK_I8))
	{
		Line("eor $80");
		Line("sec");
		Line("sbc $80");
	}
	Frame("sta", IR816_SCRATCH_VALUE);
	if (bSigned && !sType.Is(IRTK_I1))
	{
		Line("asl A");
		Line("lda 0");
		Line("adc $FFFF");
		Line("eor $FFFF");
	}
	else
	{
		Line("lda 0");
	}
	Line("pha");
	Frame("lda", IR816_SCRATCH_VALUE);
	Line("pha");
}


//////////////////////////////////////////////////////////////////////////
//
// add, sub and ptradd: a word at a time with the carry between them.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Arithmetic(SIRInstruction* psInstruction, const char* szCarry, const char* szMnemonic)
{
	int		i;
	int		iWords;

	iWords = Words(psInstruction->sType);
	Line(szCarry);
	for (i = 0; i < iWords; i++)
	{
		Operand("lda", psInstruction->asArguments[0], i);
		Operand(szMnemonic, psInstruction->asArguments[1], i);
		if (iWords == 1)
		{
			Mask(psInstruction->sType);
		}
		Store(psInstruction->sResult, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// and, or, xor, or not (eor with ulliNot's words) when ulliNot isn't 0.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Logic(SIRInstruction* psInstruction, const char* szMnemonic, uint64 ulliNot)
{
	int		i;

	for (i = 0; i < Words(psInstruction->sType); i++)
	{
		Operand("lda", psInstruction->asArguments[0], i);
		if (ulliNot)
		{
			Line("eor $FFFF");
			Mask(psInstruction->sType);
		}
		else
		{
			Operand(szMnemonic, psInstruction->asArguments[1], i);
		}
		Store(psInstruction->sResult, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The value is copied to the result and shifted there a bit at a time; the count is in the scratch word.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Shift(SIRInstruction* psInstruction)
{
	int32			iResult;
	int				iWords;
	int				i;
	int32			iLoop;
	int32			iEnd;
	SIRReference	sAmount;
	SIRType			sType;
	CChars			sz;
	int64			lliAmount;

	sType = psInstruction->sType;
	iResult = OffsetOf(psInstruction->sResult);
	iWords = Words(sType);
	sAmount = psInstruction->asArguments[1];
	CopyWords(iResult, psInstruction->asArguments[0], iWords);
	if (sAmount.IsConstant())
	{
		lliAmount = (int64)(mpcFunction->GetConstant(sAmount)->ulliBits & 63);
		if (lliAmount == 0)
		{
			return;
		}
	}

	if ((psInstruction->uiOpcode == IRO_AShr) && (sType.Is(IRTK_I8) || sType.Is(IRTK_I1)))
	{
		Frame("lda", iResult);
		Line(sType.Is(IRTK_I8) ? "eor $80" : "eor 1");
		Line("sec");
		Line(sType.Is(IRTK_I8) ? "sbc $80" : "sbc 1");
		Frame("sta", iResult);
	}

	iLoop = NewLabel();
	iEnd = NewLabel();
	Operand("lda", sAmount, 0);
	Line("and 63");
	Frame("sta", IR816_SCRATCH_COUNT);
	sz.Init("beq L");
	sz.Append(iEnd);
	Line(sz.Text());
	sz.Kill();

	Label(iLoop);
	if (psInstruction->uiOpcode == IRO_Shl)
	{
		Frame("asl", iResult);
		for (i = 1; i < iWords; i++)
		{
			Frame("rol", iResult + 2 * i);
		}
	}
	else
	{
		if (psInstruction->uiOpcode == IRO_LShr)
		{
			Frame("lsr", iResult + 2 * (iWords - 1));
		}
		else
		{
			Frame("lda", iResult + 2 * (iWords - 1));
			Line("cmp $8000");
			Line("ror A");
			Frame("sta", iResult + 2 * (iWords - 1));
		}
		for (i = iWords - 2; i >= 0; i--)
		{
			Frame("ror", iResult + 2 * i);
		}
	}
	Frame("dec", IR816_SCRATCH_COUNT);
	sz.Init("bne L");
	sz.Append(iLoop);
	Line(sz.Text());
	sz.Kill();
	Label(iEnd);

	if (sType.Is(IRTK_I8) || sType.Is(IRTK_I1))
	{
		Frame("lda", iResult);
		Mask(sType);
		Frame("sta", iResult);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// mul, udiv, sdiv, urem and srem call R816_<operation><bits>, an i8 as 16 bits (sign extended for sdiv and srem).
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::RuntimeCall(SIRInstruction* psInstruction)
{
	CChars		szName;
	SIRType		sType;
	int			iBits;
	int32		iBytes;
	bool		bSigned;
	int			i;

	sType = psInstruction->sType;
	if (sType.IsFloat() || sType.Is(IRTK_I1))
	{
		return Error("Multiply and divide are for i8 to i64");
	}
	iBits = sType.Is(IRTK_I8) ? 16 : sType.Bits();
	bSigned = (psInstruction->uiOpcode == IRO_SDiv) || (psInstruction->uiOpcode == IRO_SRem);

	szName.Init("R816_");
	switch (psInstruction->uiOpcode)
	{
	case IRO_Mul:	szName.Append("Mul");	break;
	case IRO_UDiv:	szName.Append("UDiv");	break;
	case IRO_SDiv:	szName.Append("SDiv");	break;
	case IRO_URem:	szName.Append("URem");	break;
	default:		szName.Append("SRem");	break;
	}
	szName.Append(iBits);
	UseRuntime(szName.Text());

	iBytes = 2 * ValueBytes(sType);
	for (i = 1; i >= 0; i--)
	{
		if (bSigned && sType.Is(IRTK_I8))
		{
			Operand("lda", psInstruction->asArguments[i], 0);
			Line("eor $80");
			Line("sec");
			Line("sbc $80");
			Line("pha");
		}
		else
		{
			PushValue(psInstruction->asArguments[i], sType);
		}
	}
	if (IsHiddenResult(sType))
	{
		PushFrameAddress(OffsetOf(psInstruction->sResult));
		iBytes += 4;
	}
	szName.Insert(0, "jsl ");
	Line(szName.Text());
	szName.Kill();
	CallerCleanUp(iBytes, psInstruction);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// After a call: the result is kept in the scratch bytes while the arguments are removed, then copied to its place.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::CallerCleanUp(int32 iBytes, SIRInstruction* psInstruction)
{
	bool	bResult;
	int		iWords;
	int		i;

	bResult = !psInstruction->sResult.IsNone() && !IsHiddenResult(psInstruction->sType);
	if (bResult)
	{
		Frame("sta", IR816_SCRATCH_RESULT);
		Frame("stx", IR816_SCRATCH_RESULT + 2);
	}
	if (iBytes != 0)
	{
		Line("tsc");
		Line("clc");
		Line("adc ", iBytes);
		Line("tcs");
	}
	if (bResult)
	{
		iWords = Words(psInstruction->sType);
		for (i = 0; i < iWords; i++)
		{
			Frame("lda", IR816_SCRATCH_RESULT + 2 * i);
			if (iWords == 1)
			{
				Mask(psInstruction->sType);
			}
			Store(psInstruction->sResult, i);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Leaves C and Z as an unsigned compare of the left with the right leaves them, high words first.  A signed compare
// flips the sign bits so it can compare unsigned.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::CompareFlags(SIRReference sLeft, SIRReference sRight, SIRType sType, bool bSigned)
{
	int		iWords;
	int		i;
	int32	iDone;
	const char*	szFlip;
	CChars	sz;

	iWords = Words(sType);
	iDone = NewLabel();
	szFlip = sType.Is(IRTK_I1) ? "eor 1" : (sType.Is(IRTK_I8) ? "eor $80" : "eor $8000");
	for (i = iWords - 1; i >= 0; i--)
	{
		if (bSigned && (i == iWords - 1))
		{
			Operand("lda", sRight, i);
			Line(szFlip);
			Frame("sta", IR816_SCRATCH_VALUE);
			Operand("lda", sLeft, i);
			Line(szFlip);
			Frame("cmp", IR816_SCRATCH_VALUE);
		}
		else
		{
			Operand("lda", sLeft, i);
			Operand("cmp", sRight, i);
		}
		if (i > 0)
		{
			sz.Init("bne L");
			sz.Append(iDone);
			Line(sz.Text());
			sz.Kill();
		}
	}
	Label(iDone);
}


//////////////////////////////////////////////////////////////////////////
//
// The comparison's i1 (0 or 1).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Compare(SIRInstruction* psInstruction)
{
	int32	iTrue;
	int32	iFalse;
	int32	iEnd;
	uint16	uiOpcode;
	CChars	sz;

	uiOpcode = psInstruction->uiOpcode;
	CompareFlags(psInstruction->asArguments[0], psInstruction->asArguments[1], psInstruction->sType, IsIRSignedComparison(uiOpcode));
	iTrue = NewLabel();
	iFalse = NewLabel();
	iEnd = NewLabel();
	sz.Init();
	switch (uiOpcode)
	{
	case IRO_Eq:
		sz.Append("bne L");
		break;
	case IRO_Ne:
		sz.Append("beq L");
		break;
	case IRO_SLt:
	case IRO_ULt:
		sz.Append("bcs L");
		break;
	case IRO_SGe:
	case IRO_UGe:
		sz.Append("bcc L");
		break;
	case IRO_SGt:
	case IRO_UGt:
		sz.Append("bcc L");
		sz.Append(iFalse);
		Line(sz.Text());
		sz.Clear();
		sz.Append("beq L");
		break;
	default:
		//le: true when C is clear or Z set.
		sz.Append("bcc L");
		sz.Append(iTrue);
		Line(sz.Text());
		sz.Clear();
		sz.Append("bne L");
		break;
	}
	sz.Append(iFalse);
	Line(sz.Text());
	Label(iTrue);
	Line("lda 1");
	sz.Clear();
	sz.Append("bra L");
	sz.Append(iEnd);
	Line(sz.Text());
	Label(iFalse);
	Line("lda 0");
	Label(iEnd);
	Store(psInstruction->sResult, 0);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// zext, sext, trunc and the pointer conversions.  Narrow values are already zero extended in their word.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Extend(SIRInstruction* psInstruction)
{
	SIRType			sSource;
	SIRType			sType;
	SIRReference	sValue;
	int32			iResult;
	int				iSourceWords;
	int				iWords;
	int				iCopied;

	sValue = psInstruction->asArguments[0];
	sSource = mpcFunction->TypeOf(sValue);
	sType = psInstruction->sType;
	iResult = OffsetOf(psInstruction->sResult);
	iSourceWords = Words(sSource);
	iWords = Words(sType);
	iCopied = (iSourceWords < iWords) ? iSourceWords : iWords;

	if (psInstruction->uiOpcode == IRO_SExt)
	{
		if (sSource.Is(IRTK_I1))
		{
			Line("lda 0");
			Line("sec");
			Operand("sbc", sValue, 0);
			Mask(sType);
			Frame("sta", iResult);
			iCopied = 1;
		}
		else if (sSource.Is(IRTK_I8))
		{
			Operand("lda", sValue, 0);
			Line("eor $80");
			Line("sec");
			Line("sbc $80");
			Frame("sta", iResult);
			iCopied = 1;
		}
		else
		{
			CopyWords(iResult, sValue, iCopied);
		}
		FillSign(iResult, iCopied, iWords - iCopied);
		return;
	}

	CopyWords(iResult, sValue, iCopied);
	ZeroWords(iResult, iCopied, iWords - iCopied);
	if ((psInstruction->uiOpcode == IRO_Trunc) && (sType.Is(IRTK_I8) || sType.Is(IRTK_I1)))
	{
		Frame("lda", iResult);
		Mask(sType);
		Frame("sta", iResult);
	}
	else if (psInstruction->uiOpcode == IRO_IToPtr)
	{
		//A pointer's top byte is always 0.
		Frame("lda", iResult + 2);
		Line("and $FF");
		Frame("sta", iResult + 2);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A slot's or argument's address: D + offset, in bank 0.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Address(SIRInstruction* psInstruction, int32 iOffset)
{
	Line("tdc");
	Line("clc");
	Line("adc ", iOffset);
	Store(psInstruction->sResult, 0);
	Frame("stz", OffsetOf(psInstruction->sResult) + 2);
}


//////////////////////////////////////////////////////////////////////////
//
// A word at a time through the pointer ([[D + p]:24 + Y]) or a symbol's long address; an i8 with m 8 bits.
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Load(SIRInstruction* psInstruction)
{
	int32	iPlace;
	char*	szSymbol;
	int		i;
	CChars	sz;

	PointerPlace(psInstruction->asArguments[0], &iPlace, &szSymbol);
	sz.Init();
	if (psInstruction->sType.Is(IRTK_I8))
	{
		Line("sep m");
	}
	for (i = 0; i < Words(psInstruction->sType); i++)
	{
		sz.Clear();
		if (szSymbol)
		{
			sz.Append("lda [");
			sz.Append(szSymbol);
			sz.Append(" + ");
			sz.Append(2 * i);
			sz.Append("]:24");
		}
		else
		{
			Line("ldy ", 2 * i);
			sz.Append("lda [[D + ");
			sz.Append(iPlace);
			sz.Append("]:24 + Y]");
		}
		Line(sz.Text());
		if (psInstruction->sType.Is(IRTK_I8))
		{
			Line("rep m");
			Line("and $FF");
		}
		Store(psInstruction->sResult, i);
	}
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::StoreMemory(SIRInstruction* psInstruction)
{
	int32	iPlace;
	char*	szSymbol;
	int		i;
	CChars	sz;

	PointerPlace(psInstruction->asArguments[1], &iPlace, &szSymbol);
	sz.Init();
	if (psInstruction->sType.Is(IRTK_I8))
	{
		Line("sep m");
	}
	for (i = 0; i < Words(psInstruction->sType); i++)
	{
		sz.Clear();
		if (szSymbol)
		{
			sz.Append("sta [");
			sz.Append(szSymbol);
			sz.Append(" + ");
			sz.Append(2 * i);
			sz.Append("]:24");
		}
		else
		{
			Line("ldy ", 2 * i);
			sz.Append("sta [[D + ");
			sz.Append(iPlace);
			sz.Append("]:24 + Y]");
		}
		Operand("lda", psInstruction->asArguments[0], i);
		Line(sz.Text());
	}
	if (psInstruction->sType.Is(IRTK_I8))
	{
		Line("rep m");
	}
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the arguments last first (and the hidden result pointer), calls and removes them.  A call through a pointer
// pushes the return address and the target - 1, and rtl's to it (rtl adds 1 to the low 16 bits only).
//
//////////////////////////////////////////////////////////////////////////
void CIR816Backend::Call(CIRBlock* pcBlock, SIRInstruction* psInstruction)
{
	int32			iBytes;
	size			ui;
	SIRInstruction*	psArgument;
	SIRReference	sCallee;
	int32			iReturn;
	int32			iCallee;
	CChars			sz;

	iBytes = 0;
	for (ui = maiPendingArguments.NumElements(); ui > 0; ui--)
	{
		psArgument = pcBlock->masInstructions.Get(*maiPendingArguments.Get(ui - 1));
		PushValue(psArgument->asArguments[0], psArgument->sType);
		iBytes += ValueBytes(psArgument->sType);
	}
	maiPendingArguments.FakeSetUsedElements(0);
	if (!psInstruction->sResult.IsNone() && IsHiddenResult(psInstruction->sType))
	{
		PushFrameAddress(OffsetOf(psInstruction->sResult));
		iBytes += 4;
	}

	sz.Init();
	sCallee = psInstruction->asArguments[0];
	if (sCallee.IsSymbol())
	{
		sz.Append("jsl ");
		sz.Append(mpcModule->GetSymbolName(sCallee.Index()));
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
	CallerCleanUp(iBytes, psInstruction);
}


//////////////////////////////////////////////////////////////////////////
//
// Generates the runtime routines named in paszRuntime (and those they call) from their IR.
//
//////////////////////////////////////////////////////////////////////////
bool CIR816Backend::GenerateRuntime(CArrayChars* paszRuntime, int* piLabels, CChars* pszDest, CCPPErrors* pcErrors)
{
	CArrayChars		aszNames;
	CChars			szIR;
	CChars			szText;
	CChars			szType;
	CChars			szBits;
	CChars			szTop;
	CIRModule		cModule;
	CIRParser		cParser;
	CIR816Backend	cBackend;
	size			i;
	size			j;
	char*			szName;
	int				iBits;
	bool			bResult;
	bool			bFound;

	//Only the R816_ routines are made here (the soft float functions are Libc65's).
	aszNames.Init();
	for (i = 0; i < paszRuntime->NumElements(); i++)
	{
		if (strncmp(paszRuntime->GetText(i), "R816_", 5) == 0)
		{
			aszNames.Add(paszRuntime->GetText(i));
		}
	}
	for (i = 0; i < aszNames.NumElements(); i++)
	{
		//Signed divide and remainder call the unsigned ones.
		szName = aszNames.GetText(i);
		if ((strncmp(szName, "R816_SDiv", 9) == 0) || (strncmp(szName, "R816_SRem", 9) == 0))
		{
			szText.Init("R816_U");
			szText.Append(&szName[6]);
			bFound = false;
			for (j = 0; j < aszNames.NumElements(); j++)
			{
				bFound |= (strcmp(aszNames.GetText(j), szText.Text()) == 0);
			}
			if (!bFound)
			{
				aszNames.Add(szText.Text());
			}
			szText.Kill();
		}
	}

	szIR.Init();
	for (i = 0; i < aszNames.NumElements(); i++)
	{
		szName = aszNames.GetText(i);
		iBits = atoi(&szName[strlen(szName) - 2]);
		szType.Init("i");
		szType.Append(iBits);
		szBits.Init();
		szBits.Append(iBits);
		szTop.Init();
		szTop.Append(iBits - 1);
		if (strncmp(szName, "R816_Mul", 8) == 0)
		{
			szText.Init(gszIR816Mul);
		}
		else if (szName[5] == 'U')
		{
			szText.Init(gszIR816Divide);
			szText.Replace("$RESULT", (szName[6] == 'D') ? "%q" : "%r");
		}
		else
		{
			szText.Init(gszIR816Signed);
			szText.Replace("$UNAME", &szName[6]);
			szText.Replace("$SIGN", (szName[6] == 'D') ? "ne i1 %an, %bn" : "copy i1 %an");
		}
		szText.Replace("$NAME", &szName[5]);
		szText.Replace("$T", szType.Text());
		szText.Replace("$N", szBits.Text());
		szText.Replace("$M", szTop.Text());
		szIR.Append(szText);
		szText.Kill();
		szTop.Kill();
		szBits.Kill();
		szType.Kill();
	}
	aszNames.Kill();

	cModule.Init();
	cModule.AddSpace("default", 4);
	cParser.Init(&cModule, pcErrors, "runtime");
	bResult = cParser.Parse(szIR.Text());
	cParser.Kill();
	szIR.Kill();
	if (bResult)
	{
		//Each unit that uses a routine writes it, so it is COMDAT.
		cBackend.Init(&cModule, piLabels);
		cBackend.mszSectionOptions = ", comdat";
		bResult = cBackend.GenerateModule(pszDest, pcErrors);
		cBackend.Kill();
	}
	cModule.Kill();
	return bResult;
}

