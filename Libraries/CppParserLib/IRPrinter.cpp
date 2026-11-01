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
#include "IRPrinter.h"


#define IR_NOT_NUMBERED		0xFFFFFFFF


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::Init(CIRModule* pcModule, CChars* pszDest)
{
	mpcModule = pcModule;
	mpszDest = pszDest;
	mauiTemporaryNumbers.Init();
	mauiBlockNumbers.Init();
	mbSources = false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a line with the source of the instructions and jumps that follow
// is printed where it changes (IR_SOURCE_MARKER and the source token + 1,
// as the back ends write them for a listing).  The IR parser doesn't read
// them.
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::SetSources(bool bSources)
{
	mbSources = bSources;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::Kill(void)
{
	mauiBlockNumbers.Kill();
	mauiTemporaryNumbers.Kill();
	mpcModule = NULL;
	mpszDest = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The spaces, then the restrict symbols, then the globals, then the
// functions, with a blank line before each global and function that
// follows something.
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintModule(void)
{
	size	ui;
	size	uiStart;

	uiStart = mpszDest->Length();
	for (ui = 0; ui < mpcModule->NumSpaces(); ui++)
	{
		mpszDest->Append("space ");
		mpszDest->Append(mpcModule->GetSpaceName((uint8)ui));
		mpszDest->Append(' ');
		mpszDest->Append(mpcModule->GetPointerSize((uint8)ui));
		mpszDest->AppendNewLine();
	}
	for (ui = 0; ui < mpcModule->maszSymbols.NumElements(); ui++)
	{
		if (mpcModule->IsRestrict((uint32)ui))
		{
			mpszDest->Append("restrict @");
			mpszDest->Append(mpcModule->GetSymbolName((uint32)ui));
			mpszDest->AppendNewLine();
		}
	}
	for (ui = 0; ui < mpcModule->NumGlobals(); ui++)
	{
		if (mpszDest->Length() != uiStart)
		{
			mpszDest->AppendNewLine();
		}
		PrintGlobal(mpcModule->GetGlobal(ui));
	}
	for (ui = 0; ui < mpcModule->NumFunctions(); ui++)
	{
		if (mpszDest->Length() != uiStart)
		{
			mpszDest->AppendNewLine();
		}
		PrintFunction(mpcModule->GetFunction(ui));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintType(SIRType sType)
{
	mpszDest->Append(GetIRTypeName(sType));
	if (sType.IsPointer() && (sType.uiSpace != 0))
	{
		mpszDest->Append('.');
		mpszDest->Append(mpcModule->GetSpaceName(sType.uiSpace));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintAttributes(uint32 uiAttributes)
{
	size	ui;

	for (ui = 0; ui < IRA_NumWordAttributes; ui++)
	{
		if (uiAttributes & gasIRAttributeNames[ui].uiAttribute)
		{
			mpszDest->Append(", ");
			mpszDest->Append(gasIRAttributeNames[ui].szName);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// iDigits upper case hexadecimal digits.
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintHex(uint64 ulliValue, int iDigits)
{
	int		iDigit;

	for (iDigit = iDigits - 1; iDigit >= 0; iDigit--)
	{
		mpszDest->Append("0123456789ABCDEF"[(ulliValue >> (iDigit * 4)) & 0xF]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Integers in signed decimal (i1 as 0 or 1), floats as their bits in
// hexadecimal.
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintBits(SIRType sType, uint64 ulliBits)
{
	if (sType.IsFloat())
	{
		mpszDest->Append("0x");
		PrintHex(ulliBits, sType.Bits() / 4);
	}
	else if (sType.Is(IRTK_I1))
	{
		mpszDest->Append((int32)(ulliBits & 1));
	}
	else
	{
		mpszDest->Append(IRSignExtend(sType, ulliBits));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Gives each unnamed temporary of pcFunction its number (parameters, then
// phis and instructions in block order) and each unnamed block its number.
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::Number(CIRFunction* pcFunction)
{
	size			ui;
	size			uiInstruction;
	uint32			uiNext;
	uint32			uiBlock;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;

	mauiTemporaryNumbers.FakeSetUsedElements(0);
	mauiTemporaryNumbers.GrowToAtLeastNumElements(pcFunction->NumTemporaries(), true, 0xFF);
	mauiBlockNumbers.FakeSetUsedElements(0);
	mauiBlockNumbers.GrowToAtLeastNumElements(pcFunction->NumBlocks(), true, 0xFF);

	uiNext = 0;
	uiBlock = 0;
	for (ui = 0; ui < pcFunction->masParameters.NumElements(); ui++)
	{
		NumberTemporary(pcFunction, *pcFunction->masParameters.Get(ui), &uiNext);
	}
	for (ui = 0; ui < pcFunction->NumBlocks(); ui++)
	{
		pcBlock = pcFunction->GetBlock(ui);
		if (pcBlock->mszName.Empty())
		{
			mauiBlockNumbers.Set(ui, &uiBlock);
			uiBlock++;
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masPhis.NumElements(); uiInstruction++)
		{
			NumberTemporary(pcFunction, pcBlock->masPhis.Get(uiInstruction)->sResult, &uiNext);
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (psInstruction->sResult.IsTemporary())
			{
				NumberTemporary(pcFunction, psInstruction->sResult, &uiNext);
			}
		}
	}
	//Temporaries never defined (only in IR that doesn't verify) still print.
	for (ui = 0; ui < pcFunction->NumTemporaries(); ui++)
	{
		NumberTemporary(pcFunction, IRReference(IRRK_Temporary, (uint32)ui), &uiNext);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::NumberTemporary(CIRFunction* pcFunction, SIRReference sTemporary, uint32* puiNext)
{
	uint32*	puiNumber;

	puiNumber = mauiTemporaryNumbers.SafeGet(sTemporary.Index());
	if (puiNumber && (*puiNumber == IR_NOT_NUMBERED) && (pcFunction->GetTemporaryName(sTemporary) == NULL))
	{
		*puiNumber = *puiNext;
		(*puiNext)++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// %name, or %number for an unnamed temporary (after Number).
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintReference(CIRFunction* pcFunction, SIRReference sReference)
{
	SIRConstant*	psConstant;
	char*			szName;
	uint32*			puiNumber;

	switch (sReference.Kind())
	{
	case IRRK_Temporary:
		mpszDest->Append('%');
		szName = pcFunction->GetTemporaryName(sReference);
		puiNumber = mauiTemporaryNumbers.SafeGet(sReference.Index());
		if (szName)
		{
			mpszDest->Append(szName);
		}
		else if (puiNumber && (*puiNumber != IR_NOT_NUMBERED))
		{
			mpszDest->Append(*puiNumber);
		}
		else
		{
			mpszDest->Append('?');
		}
		break;
	case IRRK_Constant:
		psConstant = pcFunction->GetConstant(sReference);
		PrintBits(psConstant->sType, psConstant->ulliBits);
		break;
	case IRRK_Symbol:
		mpszDest->Append('@');
		mpszDest->Append(mpcModule->GetSymbolName(sReference.Index()));
		break;
	default:
		mpszDest->Append('?');
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintBlockName(CIRBlock* pcBlock)
{
	uint32*	puiNumber;

	if (pcBlock == NULL)
	{
		mpszDest->Append('?');
	}
	else if (!pcBlock->mszName.Empty())
	{
		mpszDest->Append(pcBlock->GetName());
	}
	else
	{
		puiNumber = mauiBlockNumbers.SafeGet(pcBlock->muiIndex);
		if (puiNumber && (*puiNumber != IR_NOT_NUMBERED))
		{
			mpszDest->Append(*puiNumber);
		}
		else
		{
			mpszDest->Append('?');
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A tab, then "%r = " if there is a result.
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintResult(CIRFunction* pcFunction, SIRReference sResult)
{
	mpszDest->Append('\t');
	if (!sResult.IsNone())
	{
		PrintReference(pcFunction, sResult);
		mpszDest->Append(" = ");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintTyped(CIRFunction* pcFunction, SIRType sType, SIRReference sReference)
{
	PrintType(sType);
	mpszDest->Append(' ');
	PrintReference(pcFunction, sReference);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintString(uint8* pucBytes, size uiLength)
{
	size	ui;
	uint8	uc;

	mpszDest->Append('"');
	for (ui = 0; ui < uiLength; ui++)
	{
		uc = pucBytes[ui];
		if ((uc >= ' ') && (uc <= '~') && (uc != '"') && (uc != '\\'))
		{
			mpszDest->Append((char)uc);
		}
		else
		{
			mpszDest->Append('\\');
			PrintHex(uc, 2);
		}
	}
	mpszDest->Append('"');
}


//////////////////////////////////////////////////////////////////////////
//
// global @name, align n{, attributes} { items }
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintGlobal(CIRGlobal* pcGlobal)
{
	size			ui;
	SIRDataItem*	psItem;

	mpszDest->Append("global @");
	mpszDest->Append(mpcModule->GetSymbolName(pcGlobal->muiSymbol));
	mpszDest->Append(", align ");
	mpszDest->Append(pcGlobal->muiAlign);
	PrintAttributes(pcGlobal->muiAttributes);
	mpszDest->Append(" {");
	for (ui = 0; ui < pcGlobal->masItems.NumElements(); ui++)
	{
		psItem = pcGlobal->masItems.Get(ui);
		mpszDest->Append(ui == 0 ? " " : ", ");
		switch (psItem->uiKind)
		{
		case IRDK_Value:
			PrintType(psItem->sType);
			mpszDest->Append(' ');
			PrintBits(psItem->sType, psItem->ulliValue);
			break;
		case IRDK_Symbol:
			PrintType(psItem->sType);
			mpszDest->Append(" @");
			mpszDest->Append(mpcModule->GetSymbolName((uint32)psItem->ulliValue));
			if (psItem->lliOffset > 0)
			{
				mpszDest->Append('+');
			}
			if (psItem->lliOffset != 0)
			{
				mpszDest->Append(psItem->lliOffset);
			}
			break;
		case IRDK_Bytes:
			mpszDest->Append("bytes ");
			PrintString(pcGlobal->macBytes.Get(psItem->uiStart), psItem->uiLength);
			break;
		case IRDK_Zero:
			mpszDest->Append("zero ");
			mpszDest->Append(psItem->ulliValue);
			break;
		}
	}
	mpszDest->Append(" }");
	mpszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// function T @name(T %p, ...){, attributes}
// {
// blocks
// }
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintFunction(CIRFunction* pcFunction)
{
	size			ui;
	SIRReference	sParameter;

	Number(pcFunction);
	mpszDest->Append("function ");
	PrintType(pcFunction->msReturnType);
	mpszDest->Append(" @");
	mpszDest->Append(mpcModule->GetSymbolName(pcFunction->muiSymbol));
	mpszDest->Append('(');
	for (ui = 0; ui < pcFunction->masParameters.NumElements(); ui++)
	{
		if (ui != 0)
		{
			mpszDest->Append(", ");
		}
		sParameter = *pcFunction->masParameters.Get(ui);
		PrintType(pcFunction->GetTemporaryType(sParameter));
		mpszDest->Append(pcFunction->IsRestrictParameter(ui) ? " restrict " : " ");
		PrintReference(pcFunction, sParameter);
	}
	if (pcFunction->muiAttributes & IRA_Variadic)
	{
		mpszDest->Append(ui == 0 ? "..." : ", ...");
	}
	mpszDest->Append(')');
	PrintAttributes(pcFunction->muiAttributes);
	mpszDest->AppendNewLine();
	mpszDest->Append('{');
	mpszDest->AppendNewLine();
	for (ui = 0; ui < pcFunction->NumBlocks(); ui++)
	{
		PrintBlock(pcFunction, pcFunction->GetBlock(ui));
	}
	mpszDest->Append('}');
	mpszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintBlock(CIRFunction* pcFunction, CIRBlock* pcBlock)
{
	size	ui;
	uint32	uiLast;

	PrintBlockName(pcBlock);
	mpszDest->Append(':');
	mpszDest->AppendNewLine();
	for (ui = 0; ui < pcBlock->masPhis.NumElements(); ui++)
	{
		PrintPhi(pcFunction, pcBlock->masPhis.Get(ui));
	}
	uiLast = 0;
	ui = 0;
	while (ui < pcBlock->masInstructions.NumElements())
	{
		PrintSource(pcBlock->masInstructions.Get(ui)->uiSource, &uiLast);
		ui = PrintInstruction(pcFunction, pcBlock, ui);
	}
	if (pcBlock->HasJump())
	{
		PrintSource(pcBlock->msJump.uiSource, &uiLast);
		PrintJump(pcFunction, &pcBlock->msJump);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The source line (SetSources) when uiSource isn't 0 or the last printed.
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintSource(uint32 uiSource, uint32* puiLast)
{
	if (!mbSources || (uiSource == 0) || (uiSource == *puiLast))
	{
		return;
	}
	*puiLast = uiSource;
	mpszDest->Append(IR_SOURCE_MARKER);
	mpszDest->Append((int64)uiSource);
	mpszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// %r = phi T [block, value], ...
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintPhi(CIRFunction* pcFunction, SIRPhi* psPhi)
{
	size			ui;
	SIRPhiArgument*	psArgument;

	PrintResult(pcFunction, psPhi->sResult);
	mpszDest->Append("phi ");
	PrintType(psPhi->sType);
	for (ui = 0; ui < psPhi->asArguments.NumElements(); ui++)
	{
		psArgument = psPhi->asArguments.Get(ui);
		mpszDest->Append(ui == 0 ? " [" : ", [");
		PrintBlockName(psArgument->pcBlock);
		mpszDest->Append(", ");
		PrintReference(pcFunction, psArgument->sValue);
		mpszDest->Append(']');
	}
	mpszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Prints the instruction at uiIndex and returns the index of the next.  Args
// straight before a call are printed in the call.
//
//////////////////////////////////////////////////////////////////////////
size CIRPrinter::PrintInstruction(CIRFunction* pcFunction, CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	SIROpcode*		psOpcode;
	size			uiCall;
	size			ui;

	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	if (psInstruction->uiOpcode == IRO_Arg)
	{
		uiCall = uiIndex;
		while ((uiCall < pcBlock->masInstructions.NumElements()) && (pcBlock->masInstructions.Get(uiCall)->uiOpcode == IRO_Arg))
		{
			uiCall++;
		}
		if ((uiCall < pcBlock->masInstructions.NumElements()) && (pcBlock->masInstructions.Get(uiCall)->uiOpcode == IRO_Call))
		{
			PrintCall(pcFunction, pcBlock, uiIndex, uiCall);
			return uiCall + 1;
		}
	}
	else if (psInstruction->uiOpcode == IRO_Call)
	{
		PrintCall(pcFunction, pcBlock, uiIndex, uiIndex);
		return uiIndex + 1;
	}

	PrintResult(pcFunction, psInstruction->sResult);
	psOpcode = mpcModule->GetOpcode(psInstruction->uiOpcode);
	mpszDest->Append(psOpcode ? psOpcode->szName : "?");
	for (ui = 0; ui < IRF_NumWordFlags; ui++)
	{
		if (psInstruction->uiFlags & gasIRFlagNames[ui].uiFlag)
		{
			mpszDest->Append(' ');
			mpszDest->Append(gasIRFlagNames[ui].szName);
		}
	}
	mpszDest->Append(' ');

	if (psOpcode && (psOpcode->uiShape == IRSH_Conversion))
	{
		PrintTyped(pcFunction, pcFunction->TypeOf(psInstruction->asArguments[0]), psInstruction->asArguments[0]);
		mpszDest->Append(" to ");
		PrintType(psInstruction->sType);
	}
	else
	{
		PrintTyped(pcFunction, psInstruction->sType, psInstruction->asArguments[0]);
		if (!psInstruction->asArguments[1].IsNone())
		{
			mpszDest->Append(", ");
			PrintReference(pcFunction, psInstruction->asArguments[1]);
		}
	}
	mpszDest->AppendNewLine();
	return uiIndex + 1;
}


//////////////////////////////////////////////////////////////////////////
//
// {%r = }call T callee(T a, ...)
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintCall(CIRFunction* pcFunction, CIRBlock* pcBlock, size uiFirstArg, size uiCall)
{
	SIRInstruction*	psCall;
	SIRInstruction*	psArg;
	size			ui;

	psCall = pcBlock->masInstructions.Get(uiCall);
	PrintResult(pcFunction, psCall->sResult);
	mpszDest->Append((psCall->uiFlags & IRF_ThisCall) ? "call thiscall " : "call ");
	PrintType(psCall->sType);
	mpszDest->Append(' ');
	PrintReference(pcFunction, psCall->asArguments[0]);
	mpszDest->Append('(');
	for (ui = uiFirstArg; ui < uiCall; ui++)
	{
		if ((psCall->uiFlags & IRF_Variadic) && (ui - uiFirstArg == psCall->uiNamed))
		{
			mpszDest->Append(ui == uiFirstArg ? "..., " : ", ..., ");
		}
		else if (ui != uiFirstArg)
		{
			mpszDest->Append(", ");
		}
		psArg = pcBlock->masInstructions.Get(ui);
		PrintTyped(pcFunction, psArg->sType, psArg->asArguments[0]);
	}
	if ((psCall->uiFlags & IRF_Variadic) && (uiCall - uiFirstArg <= psCall->uiNamed))
	{
		mpszDest->Append(uiCall == uiFirstArg ? "..." : ", ...");
	}
	mpszDest->Append(')');
	mpszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPrinter::PrintJump(CIRFunction* pcFunction, SIRJump* psJump)
{
	mpszDest->Append('\t');
	switch (psJump->uiKind)
	{
	case IRJ_Jump:
		mpszDest->Append("jmp ");
		PrintBlockName(psJump->pcThen);
		break;
	case IRJ_Branch:
		mpszDest->Append("brif ");
		PrintReference(pcFunction, psJump->sArgument);
		mpszDest->Append(", ");
		PrintBlockName(psJump->pcThen);
		mpszDest->Append(", ");
		PrintBlockName(psJump->pcElse);
		break;
	case IRJ_Return:
		mpszDest->Append("ret");
		if (!psJump->sType.IsNone())
		{
			mpszDest->Append(' ');
			PrintTyped(pcFunction, psJump->sType, psJump->sArgument);
		}
		break;
	case IRJ_Unreachable:
		mpszDest->Append("unreachable");
		break;
	}
	mpszDest->AppendNewLine();
}
