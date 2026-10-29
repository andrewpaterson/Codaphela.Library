/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/FileUtil.h"
#include "Asm65Assembler.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::Init(void)
{
	mcErrors.Init();
	mcWarnings.Init();
	mcTokeniser.Init(&mcErrors);
	mcSymbols.Init();
	mcOutput.Init();
	mcMinimumWords.Init();
	mcPendingLabels.Init();

	muiToken = 0;
	muiAddress = 0;
	muiStatementAddress = 0;
	muiPass = 0;
	mbFinal = false;
	mbChanged = false;
	mbStatementError = false;
	mbRelative = false;
	mbMemoryWidth = false;
	muiInstruction = 0;
	muiPasses = 0;

	mbObject = false;
	mpcObject = NULL;
	muiSection = 0;
	mcSectionAddresses.Init();
	mcUndefined.Init();
	mcAssociations.Init();
	miExpressionDepth = 0;
	muiStatementTerms = 0;
	muiStatementTermsUsed = 0;
	mszDirectory.Init();
	maszHexFiles.Init();
	mapcHexFiles.Init();
	mpsTermToken = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::Kill(void)
{
	size		i;
	CIntelHex*	pcHex;

	for (i = 0; i < mapcHexFiles.NumElements(); i++)
	{
		pcHex = mapcHexFiles.GetPtr(i);
		SafeKill(pcHex);
	}
	mapcHexFiles.Kill();
	maszHexFiles.Kill();
	mszDirectory.Kill();
	mcAssociations.Kill();
	mcUndefined.Kill();
	mcSectionAddresses.Kill();
	mcPendingLabels.Kill();
	mcMinimumWords.Kill();
	mcOutput.Kill();
	mcSymbols.Kill();
	mcTokeniser.Kill();
	mcWarnings.Kill();
	mcErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Passes run until no label moves and no instruction grows.  The final pass writes
// the output and reports errors.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Assemble(const char* szSource)
{
	if (!mcTokeniser.Tokenise(szSource))
	{
		return false;
	}

	mbFinal = false;
	for (muiPass = 1; muiPass <= ASM65_MAX_PASSES; muiPass++)
	{
		mbChanged = false;
		Pass();
		if (mbObject)
		{
			MarkExternals();
		}
		if (!mbChanged)
		{
			break;
		}
	}

	muiPass++;
	mbFinal = true;
	mbChanged = false;
	mcOutput.Clear();
	Pass();
	muiPasses = muiPass;

	if (mbChanged && (mcErrors.NumErrors() == 0))
	{
		mcErrors.Add(0, 0, "Labels did not settle");
	}
	return mcErrors.NumErrors() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Assembles into sections (.section) starting in "text".  Labels are section offsets
// and every use of one that the linker must place becomes a relocation.  Names that
// are used but never defined are externals.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::AssembleObject(const char* szSource, CAsm65Object* pcObject)
{
	bool	bResult;

	mbObject = true;
	mpcObject = pcObject;
	if (mpcObject->FindSection("text", "") != 0)
	{
		mpcObject->AddSection("text", "", ASK65_Text);
	}

	bResult = Assemble(szSource);
	if (bResult)
	{
		Associate();
		BuildObjectSymbols();
		bResult = mcErrors.NumErrors() == 0;
	}
	mpcObject = NULL;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::Pass(void)
{
	size			i;
	SAsm65Section*	psSection;

	muiToken = 0;
	muiAddress = 0;
	muiInstruction = 0;
	miLabelShift = 0;
	mcPendingLabels.Kill();
	mcPendingLabels.Init();
	mcUndefined.Kill();
	mcUndefined.Init();
	if (mbObject)
	{
		muiSection = 0;
		mcSectionAddresses.Kill();
		mcSectionAddresses.Init();
		for (i = 0; i < mpcObject->NumSections(); i++)
		{
			mcSectionAddresses.Add(0);
			if (mbFinal)
			{
				psSection = mpcObject->GetSection(i);
				psSection->cBytes.Kill();
				psSection->cBytes.Init();
			}
		}
	}

	SectionStarted();
	while (Peek()->eType != AT65_End)
	{
		Statement();
	}
	muiStatementAddress = muiAddress;
	BindLabels();

	if (mbObject)
	{
		mcSectionAddresses.SetValue(muiSection, muiAddress);
		if (mbFinal)
		{
			for (i = 0; i < mpcObject->NumSections(); i++)
			{
				mpcObject->GetSection(i)->uiSize = mcSectionAddresses.GetValue(i);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A name used in a pass but not defined by it becomes an external, which needs
// another pass: an external address is always 32 bits.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::MarkExternals(void)
{
	size			i;
	SAsm65Token*	psToken;
	SAsm65Symbol*	psSymbol;
	size			uiToken;

	for (i = 0; i < mcUndefined.NumElements(); i++)
	{
		uiToken = mcUndefined.GetValue(i);
		psToken = mcTokeniser.Get(uiToken);
		psSymbol = mcSymbols.Get(psToken->szText, psToken->uiLength);
		if ((psSymbol == NULL) || (!psSymbol->bResolved && (psSymbol->uiPass != muiPass)))
		{
			MakeExternal(mcSymbols.Put(psToken->szText, psToken->uiLength), uiToken);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The symbol is defined by another object; uiToken is where it was named.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::MakeExternal(SAsm65Symbol* psSymbol, size uiToken)
{
	psSymbol->iValue = 0;
	psSymbol->bResolved = true;
	psSymbol->bExternal = true;
	psSymbol->iSection = ASM65_SECTION_UNDEFINED;
	psSymbol->uiTermToken = uiToken;
	mbChanged = true;
}


//////////////////////////////////////////////////////////////////////////
//
// Every label and constant goes into the object (locals too, for the map) with
// externals that are declared or used by a relocation.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::BuildObjectSymbols(void)
{
	size				i;
	char*				szName;
	SAsm65Symbol*		psSymbol;
	SAsm65ObjectSymbol*	psObjectSymbol;
	EAsm65Binding		eBinding;

	for (i = 0; i < mcSymbols.NumSymbols(); i++)
	{
		szName = mcSymbols.GetName(i);
		psSymbol = mcSymbols.Get(szName, strlen(szName));
		eBinding = psSymbol->bWeak ? AB65_Weak : (psSymbol->bGlobal ? AB65_Global : AB65_Local);
		if (psSymbol->bExternal || !psSymbol->bResolved)
		{
			if ((eBinding != AB65_Local) || (mpcObject->FindSymbol(szName) >= 0))
			{
				psObjectSymbol = mpcObject->GetSymbol(mpcObject->PutSymbol(szName));
				psObjectSymbol->iSection = ASM65_SECTION_UNDEFINED;
				psObjectSymbol->iValue = 0;
				psObjectSymbol->eBinding = (eBinding == AB65_Weak) ? AB65_Weak : AB65_Global;
			}
		}
		else if (psSymbol->iSection == ASM65_SECTION_UNDEFINED)
		{
			if (eBinding != AB65_Local)
			{
				Report(&mcErrors, mcTokeniser.Get(psSymbol->uiTermToken), "A global symbol cannot be an external plus a value");
			}
		}
		else
		{
			psObjectSymbol = mpcObject->GetSymbol(mpcObject->PutSymbol(szName));
			psObjectSymbol->iSection = psSymbol->iSection;
			psObjectSymbol->iValue = psSymbol->iValue;
			psObjectSymbol->eBinding = eBinding;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// [label:]... [instruction | directive | name = expression] new line
//
// Labels are bound when the next statement starts (after an instruction aligns the
// address, a .org moves it or a .align pads it).
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::Statement(void)
{
	SAsm65Token*	psToken;
	bool			bResult;

	mbStatementError = false;
	mbRelative = false;
	mbMemoryWidth = false;
	muiStatementTerms = 0;
	muiStatementTermsUsed = 0;
	mpsTermToken = NULL;
	while ((Peek()->eType == AT65_Identifier) && Peek(1)->Is(AP65_Colon))
	{
		psToken = Next();
		Next();
		if (IsReservedName(psToken))
		{
			Error(psToken, "Reserved name cannot be a label");
		}
		else
		{
			mcPendingLabels.Add((int)(psToken - mcTokeniser.Get(0)));
		}
	}

	psToken = Peek();
	muiStatementAddress = muiAddress;
	bResult = true;
	if (psToken->IsEndOfStatement())
	{
	}
	else if (psToken->eType != AT65_Identifier)
	{
		bResult = Error(psToken, "Expected an instruction, directive or label");
	}
	else if (Peek(1)->Is(AP65_Equals))
	{
		BindLabels();
		Next();
		Next();
		bResult = Constant(psToken);
	}
	else if (psToken->szText[0] == '.')
	{
		Next();
		if (psToken->IsIdentifierIgnoreCase(".org"))
		{
			bResult = Origin();
			BindLabels();
		}
		else if (psToken->IsIdentifierIgnoreCase(".section"))
		{
			bResult = Section();
			BindLabels();
		}
		else if (psToken->IsIdentifierIgnoreCase(".align"))
		{
			bResult = Align();
			BindLabels();
		}
		else
		{
			BindLabels();
			if (psToken->IsIdentifierIgnoreCase(".byte"))
			{
				bResult = Data(1);
			}
			else if (psToken->IsIdentifierIgnoreCase(".word"))
			{
				bResult = Data(2);
			}
			else if (psToken->IsIdentifierIgnoreCase(".long"))
			{
				bResult = Data(4);
			}
			else if (psToken->IsIdentifierIgnoreCase(".quad"))
			{
				bResult = Data(8);
			}
			else if (psToken->IsIdentifierIgnoreCase(".space"))
			{
				bResult = Space();
			}
			else if (psToken->IsIdentifierIgnoreCase(".inchex"))
			{
				bResult = IncludeHex();
			}
			else if (psToken->IsIdentifierIgnoreCase(".global") || psToken->IsIdentifierIgnoreCase(".weak") || psToken->IsIdentifierIgnoreCase(".extern"))
			{
				bResult = SymbolBinding(psToken);
			}
			else if (!ProcessorDirective(psToken, &bResult))
			{
				bResult = Error(psToken, "Unknown directive");
			}
		}
	}
	else
	{
		AlignTo(InstructionAlignment());
		muiStatementAddress = muiAddress;
		BindLabels();
		Next();
		bResult = !InBss(psToken) && Instruction(psToken);
	}

	if (bResult)
	{
		bResult = ExpectEnd();
	}
	if (bResult && (muiStatementTerms != muiStatementTermsUsed))
	{
		Error(mpsTermToken, "A section address cannot be used here");
	}

	//Skip the rest of the line after an error.
	while (!Peek()->IsEndOfStatement())
	{
		Next();
	}
	if (Peek()->eType == AT65_NewLine)
	{
		Next();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::BindLabels(void)
{
	size			i;
	SAsm65Token*	psLabel;
	SAsm65Symbol*	psSymbol;
	int32			iSection;

	iSection = mbObject ? (int32)muiSection : ASM65_SECTION_ABSOLUTE;
	for (i = 0; i < mcPendingLabels.NumElements(); i++)
	{
		psLabel = mcTokeniser.Get(mcPendingLabels.GetValue(i));
		psSymbol = mcSymbols.Get(psLabel->szText, psLabel->uiLength);
		if (psSymbol && psSymbol->bLabel && psSymbol->bResolved && (psSymbol->uiPass + 1 == muiPass) && (psSymbol->iSection == iSection))
		{
			miLabelShift = (int64)muiAddress - psSymbol->iValue;
		}
		DefineSymbol(psLabel, muiAddress, true, iSection, mcPendingLabels.GetValue(i));
		mcSymbols.Get(psLabel->szText, psLabel->uiLength)->bLabel = true;
	}
	mcPendingLabels.Kill();
	mcPendingLabels.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// A symbol defined twice in one pass is an error.  A symbol whose value changed since
// the last pass needs another pass.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::DefineSymbol(SAsm65Token* psName, int64 iValue, bool bResolved, int32 iSection, size uiTermToken)
{
	SAsm65Symbol*	psSymbol;

	psSymbol = mcSymbols.Put(psName->szText, psName->uiLength);
	if (psSymbol->uiPass == muiPass)
	{
		Error(psName, "Symbol is already defined");
		return;
	}
	if (psSymbol->bExternal)
	{
		Error(psName, "Symbol is declared external");
		return;
	}
	if ((psSymbol->iValue != iValue) || (psSymbol->bResolved != bResolved) || (psSymbol->iSection != iSection))
	{
		mbChanged = true;
	}
	psSymbol->iValue = iValue;
	psSymbol->bResolved = bResolved;
	psSymbol->uiPass = muiPass;
	psSymbol->iSection = iSection;
	psSymbol->uiTermToken = uiTermToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Assembler::TokenIndex(SAsm65Token* psToken)
{
	return (size)(psToken - mcTokeniser.Get(0));
}


//////////////////////////////////////////////////////////////////////////
//
// name = expression
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Constant(SAsm65Token* psName)
{
	SAsm65Value	sValue;

	if (IsReservedName(psName))
	{
		return Error(psName, "Reserved name cannot be a constant");
	}
	if (!Expression(&sValue))
	{
		return false;
	}
	if (sValue.iTerms == 0)
	{
		DefineSymbol(psName, sValue.iValue, sValue.bResolved, ASM65_SECTION_ABSOLUTE, 0);
		return true;
	}
	if (sValue.iTerms != 1)
	{
		return Error(psName, "An address must be one label plus or minus a constant");
	}
	UseTerm();
	DefineSymbol(psName, sValue.iValue, sValue.bResolved, sValue.iSection, sValue.uiTermToken);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// .org address
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Origin(void)
{
	int64	iValue;

	if (mbObject)
	{
		return Error(Peek(), ".org is only allowed in absolute mode; use .section");
	}
	if (!ExpressionValue(&iValue))
	{
		return false;
	}
	muiAddress = (uint32)iValue;
	muiStatementAddress = muiAddress;
	miLabelShift = 0;
	SectionStarted();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// .align bytes (a power of two)
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Align(void)
{
	SAsm65Token*	psToken;
	int64			iValue;

	psToken = Peek();
	if (!ExpressionValue(&iValue))
	{
		return false;
	}
	if ((iValue <= 0) || (iValue > 0x10000) || ((iValue & (iValue - 1)) != 0))
	{
		return Error(psToken, "Alignment must be a power of two");
	}
	AlignTo((uint32)iValue);
	muiStatementAddress = muiAddress;
	miLabelShift = 0;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// .byte, .word, .long and .quad: values, little endian.  .byte also takes strings.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Data(uint8 uiBytes)
{
	SAsm65Token*	psToken;
	SAsm65Value		sValue;
	char*			szString;
	uint32			i;
	uint8			j;

	if (InBss(Peek()))
	{
		return false;
	}
	do
	{
		psToken = Peek();
		if (psToken->eType == AT65_String)
		{
			if (uiBytes != 1)
			{
				return Error(psToken, "Strings are only allowed in .byte");
			}
			Next();
			szString = mcTokeniser.GetString(psToken);
			for (i = 0; i < psToken->uiStringLength; i++)
			{
				WriteByte((uint8)szString[i]);
			}
		}
		else
		{
			if (!Expression(&sValue))
			{
				return false;
			}
			if (IsRelocatable(&sValue))
			{
				UseTerm();
				RecordRelocation(&sValue, muiAddress, uiBytes, false, false, 0, psToken);
				for (j = 0; j < uiBytes; j++)
				{
					WriteByte(0);
				}
				continue;
			}
			if (!Asm65Fits(sValue.iValue, uiBytes, false) && !Asm65Fits(sValue.iValue, uiBytes, true))
			{
				return Error(psToken, "Value does not fit");
			}
			for (j = 0; j < uiBytes; j++)
			{
				WriteByte((uint8)(sValue.iValue >> (8 * j)));
			}
		}
	}
	while (Accept(AP65_Comma));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Pads with zeros.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::AlignTo(uint32 uiAlignment)
{
	SAsm65Section*	psSection;

	if (mbObject)
	{
		psSection = mpcObject->GetSection(muiSection);
		if (psSection->uiAlignment < uiAlignment)
		{
			psSection->uiAlignment = uiAlignment;
		}
	}
	while ((muiAddress & (uiAlignment - 1)) != 0)
	{
		WriteByte(0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::WriteByte(uint8 uiByte)
{
	SAsm65Section*	psSection;

	if (mbFinal)
	{
		if (!mbObject)
		{
			mcOutput.Write(muiAddress, uiByte);
		}
		else
		{
			psSection = mpcObject->GetSection(muiSection);
			if (psSection->eKind != ASK65_Bss)
			{
				while (psSection->cBytes.NumElements() < muiAddress)
				{
					psSection->cBytes.Add(0);
				}
				psSection->cBytes.Add(uiByte);
			}
		}
	}
	muiAddress++;
}


//////////////////////////////////////////////////////////////////////////
//
// .space bytes (zeros)
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Space(void)
{
	SAsm65Token*	psToken;
	int64			iValue;
	int64			i;

	psToken = Peek();
	if (!ExpressionValue(&iValue))
	{
		return false;
	}
	if ((iValue < 0) || (iValue > 0x1000000))
	{
		return Error(psToken, "Space must be 0 to 16MB");
	}
	for (i = 0; i < iValue; i++)
	{
		WriteByte(0);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// .inchex "file" {, base}
// The bytes of an Intel HEX file from its address base (its lowest address
// when not given) to its highest, $FF where it has none (as an erased ROM).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::IncludeHex(void)
{
	SAsm65Token*	psName;
	SAsm65Token*	psBase;
	CIntelHex*		pcHex;
	int64			iBase;
	uint64			ulliLength;
	uint8*			puiBytes;
	uint32			i;

	if (InBss(Peek()))
	{
		return false;
	}
	psName = Peek();
	if (psName->eType != AT65_String)
	{
		return ErrorExpected(psName, "a file name");
	}
	Next();
	pcHex = GetHexFile(psName);
	if (!pcHex)
	{
		return false;
	}

	iBase = pcHex->GetLowAddress();
	if (Accept(AP65_Comma))
	{
		psBase = Peek();
		if (!ExpressionValue(&iBase))
		{
			return false;
		}
		if ((iBase < 0) || (iBase > 0xFFFFFFFFLL))
		{
			return Error(psBase, "The base must be a 32 bit address");
		}
		if (!pcHex->IsEmpty() && ((uint32)iBase > pcHex->GetLowAddress()))
		{
			return Error(psBase, "The file has bytes below the base");
		}
	}

	ulliLength = pcHex->IsEmpty() ? 0 : pcHex->GetEndAddress() - (uint64)iBase;
	if (ulliLength > 0x1000000)
	{
		return Error(psName, "Included bytes must be 0 to 16MB");
	}
	if (!mbFinal)
	{
		muiAddress += (uint32)ulliLength;
		return true;
	}
	puiBytes = (uint8*)malloc((size)ulliLength + 1);
	pcHex->Fill((uint32)iBase, (uint32)ulliLength, 0xFF, puiBytes);
	for (i = 0; i < (uint32)ulliLength; i++)
	{
		WriteByte(puiBytes[i]);
	}
	free(puiBytes);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The file named by the string psName (relative to SetDirectory's), read
// once for all the passes.
//
//////////////////////////////////////////////////////////////////////////
CIntelHex* CAsm65Assembler::GetHexFile(SAsm65Token* psName)
{
	CChars		szPath;
	CFileUtil	cFileUtil;
	CIntelHex*	pcHex;
	CChars		szMessage;
	size		i;

	szPath.Init();
	if (!mszDirectory.Empty() && !cFileUtil.IsAbsolutePath(mcTokeniser.GetString(psName)))
	{
		szPath.Append(mszDirectory);
		cFileUtil.AppendToPath(&szPath, mcTokeniser.GetString(psName));
	}
	else
	{
		szPath.Append(mcTokeniser.GetString(psName));
	}

	for (i = 0; i < maszHexFiles.NumElements(); i++)
	{
		if (StringCompare(maszHexFiles.GetText(i), szPath.Text()) == 0)
		{
			szPath.Kill();
			return mapcHexFiles.GetPtr(i);
		}
	}

	pcHex = NewMalloc<CIntelHex>();
	pcHex->Init();
	if (!pcHex->Read(szPath.Text()))
	{
		szMessage.Init(pcHex->GetError());
		Error(psName, szMessage.Text());
		szMessage.Kill();
		SafeKill(pcHex);
		szPath.Kill();
		return NULL;
	}
	maszHexFiles.Add(szPath.Text());
	mapcHexFiles.Add(pcHex);
	szPath.Kill();
	return pcHex;
}


//////////////////////////////////////////////////////////////////////////
//
// The directory .inchex file names are relative to.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::SetDirectory(const char* szDirectory)
{
	mszDirectory.Kill();
	mszDirectory.Init(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
// .section name {, option}...
// The name starts with text, rodata, init, fini, data or bss.  The options are
//   comdat {group}         a COMDAT section; its group is its name unless given
//   associated section     used when that section of this object is used
//   keep                   always used
//   resident               never loaded lazily
// Options given again when switching back to a section are added.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Section(void)
{
	SAsm65Token*		psName;
	SAsm65Token*		psGroup;
	SAsm65Token*		psAssociated;
	SAsm65Section*		psSection;
	EAsm65SectionKind	eKind;
	CChars				szName;
	CChars				szGroup;
	int32				iSection;
	bool				bKeep;
	bool				bResident;
	bool				bAt;
	uint32				uiAt;

	psName = Peek();
	if (!mbObject)
	{
		return Error(psName, ".section is only allowed when assembling an object");
	}
	if (psName->eType != AT65_Identifier)
	{
		return ErrorExpected(psName, "a section name");
	}
	if (!Asm65SectionKind(psName->szText, psName->uiLength, &eKind))
	{
		return Error(psName, "A section name starts with text, rodata, init, fini, data or bss");
	}
	Next();
	if (!SectionOptions(psName, &psGroup, &psAssociated, &bKeep, &bResident, &bAt, &uiAt))
	{
		return false;
	}

	szName.Init();
	szName.Append(psName->szText, psName->uiLength);
	szGroup.Init();
	if (psGroup)
	{
		szGroup.Append(psGroup->szText, psGroup->uiLength);
	}
	iSection = mpcObject->FindSection(szName.Text(), szGroup.Text());
	if (iSection < 0)
	{
		iSection = (int32)mpcObject->AddSection(szName.Text(), szGroup.Text(), eKind);
		mcSectionAddresses.Add(0);
	}
	szGroup.Kill();
	szName.Kill();

	psSection = mpcObject->GetSection(iSection);
	psSection->bKeep |= bKeep;
	psSection->bResident |= bResident;
	if (bAt)
	{
		if (psSection->bAt && (psSection->uiAt != uiAt))
		{
			return Error(psName, "The section is already at another address");
		}
		psSection->bAt = true;
		psSection->uiAt = uiAt;
	}
	if (psAssociated && mbFinal)
	{
		//Resolved after the final pass (Associate): the section may come later.
		mcAssociations.Add(iSection);
		mcAssociations.Add((int)TokenIndex(psAssociated));
	}

	SwitchSection((uint32)iSection);
	muiStatementAddress = muiAddress;
	SectionStarted();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::SectionOptions(SAsm65Token* psName, SAsm65Token** ppsGroup, SAsm65Token** ppsAssociated, bool* pbKeep, bool* pbResident, bool* pbAt, uint32* puiAt)
{
	SAsm65Token*	psOption;
	SAsm65Value		sValue;

	*ppsGroup = NULL;
	*ppsAssociated = NULL;
	*pbKeep = false;
	*pbResident = false;
	*pbAt = false;
	*puiAt = 0;
	while (Accept(AP65_Comma))
	{
		psOption = Peek();
		if (psOption->IsIdentifierIgnoreCase("comdat"))
		{
			Next();
			*ppsGroup = psName;
			if (Peek()->eType == AT65_Identifier)
			{
				*ppsGroup = Next();
			}
		}
		else if (psOption->IsIdentifierIgnoreCase("associated"))
		{
			Next();
			if (Peek()->eType != AT65_Identifier)
			{
				return ErrorExpected(Peek(), "a section name");
			}
			*ppsAssociated = Next();
		}
		else if (psOption->IsIdentifierIgnoreCase("keep"))
		{
			Next();
			*pbKeep = true;
		}
		else if (psOption->IsIdentifierIgnoreCase("resident"))
		{
			Next();
			*pbResident = true;
		}
		else if (psOption->IsIdentifierIgnoreCase("at"))
		{
			Next();
			if (!Expression(&sValue))
			{
				return false;
			}
			if ((sValue.iTerms != 0) || (sValue.bResolved && ((sValue.iValue < 0) || (sValue.iValue > 0xFFFFFFFFLL))))
			{
				return Error(psOption, "A section's address is a constant");
			}
			*pbAt = true;
			*puiAt = (uint32)sValue.iValue;
		}
		else
		{
			return ErrorExpected(psOption, "comdat, associated, keep, resident or at");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Points each associated section at the section it names (by name, in this object).
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::Associate(void)
{
	size			i;
	SAsm65Token*	psName;
	CChars			szName;
	int32			iSection;

	for (i = 0; i + 1 < mcAssociations.NumElements(); i += 2)
	{
		psName = mcTokeniser.Get(mcAssociations.GetValue(i + 1));
		szName.Init();
		szName.Append(psName->szText, psName->uiLength);
		iSection = mpcObject->FindSection(szName.Text(), NULL);
		szName.Kill();
		if (iSection < 0)
		{
			Report(&mcErrors, psName, "Unknown section");
		}
		else
		{
			mpcObject->GetSection(mcAssociations.GetValue(i))->iAssociated = iSection;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::SwitchSection(uint32 uiSection)
{
	mcSectionAddresses.SetValue(muiSection, muiAddress);
	muiSection = uiSection;
	muiAddress = mcSectionAddresses.GetValue(muiSection);
	miLabelShift = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A bss section only has labels, .space and .align.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::InBss(SAsm65Token* psToken)
{
	if (mbObject && (mpcObject->GetSection(muiSection)->eKind == ASK65_Bss))
	{
		Error(psToken, "A bss section only has labels, .space and .align");
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// .global name, ...    the name is visible to other objects (or is an external)
// .weak name, ...      as .global, but another object's global definition wins and
//                      an undefined weak name is 0
// .extern name, ...    the name is defined by another object
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::SymbolBinding(SAsm65Token* psDirective)
{
	SAsm65Token*	psName;
	SAsm65Symbol*	psSymbol;

	if (!mbObject)
	{
		return Error(psDirective, "Symbol bindings are only allowed when assembling an object");
	}
	do
	{
		psName = Peek();
		if ((psName->eType != AT65_Identifier) || IsReservedName(psName))
		{
			return ErrorExpected(psName, "a symbol name");
		}
		Next();

		psSymbol = mcSymbols.Put(psName->szText, psName->uiLength);
		if (psDirective->IsIdentifierIgnoreCase(".global"))
		{
			psSymbol->bGlobal = true;
		}
		else if (psDirective->IsIdentifierIgnoreCase(".weak"))
		{
			psSymbol->bWeak = true;
		}
		else if (!psSymbol->bExternal)
		{
			if (psSymbol->uiPass == muiPass)
			{
				return Error(psName, "Symbol is already defined");
			}
			MakeExternal(psSymbol, TokenIndex(psName));
		}
	}
	while (Accept(AP65_Comma));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A value the linker must finish: an address in a section or an external.  A value
// that is not resolved yet is sized like any forward reference (it grows next pass).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::IsRelocatable(SAsm65Value* psValue)
{
	return mbObject && psValue->bResolved && (psValue->iTerms != 0);
}


//////////////////////////////////////////////////////////////////////////
//
// A relative target in the current section is an offset the assembler knows.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::IsLocalTarget(SAsm65Value* psValue)
{
	return mbObject && psValue->bResolved && (psValue->iTerms == 1) && (psValue->iSection == (int32)muiSection);
}


//////////////////////////////////////////////////////////////////////////
//
// A relative target in another section or an external needs a relocation.  A target
// in this section becomes a plain offset (like the statement's address).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::TargetIsRelocatable(SAsm65Value* psTarget)
{
	if (IsLocalTarget(psTarget))
	{
		UseTerm();
		psTarget->iTerms = 0;
		psTarget->iSection = ASM65_SECTION_ABSOLUTE;
		return false;
	}
	return IsRelocatable(psTarget);
}


//////////////////////////////////////////////////////////////////////////
//
// Each section address in a statement must be used by a relocation or a constant.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::UseTerm(void)
{
	muiStatementTermsUsed++;
}


//////////////////////////////////////////////////////////////////////////
//
// A relocation for the field at uiOffset in the current section.  Externals, global
// and weak labels are relocated by name (so another object's definition or the kept
// copy of a COMDAT section is used), local labels by their section.  A relative field
// is measured from C, uiFieldToNext bytes after the field's start.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::RecordRelocation(SAsm65Value* psValue, uint32 uiOffset, uint8 uiBytes, bool bSigned, bool bRelative, uint32 uiFieldToNext, SAsm65Token* psToken)
{
	SAsm65Relocation*	psRelocation;
	SAsm65Token*		psTerm;
	SAsm65Symbol*		psSymbol;
	CChars				szName;

	if (!mbFinal)
	{
		return;
	}
	if (psValue->iTerms != 1)
	{
		Error(psToken, "An address must be one label plus or minus a constant");
		return;
	}

	psRelocation = mpcObject->AddRelocation();
	psRelocation->uiSection = muiSection;
	psRelocation->uiOffset = uiOffset;
	psRelocation->uiBytes = uiBytes;
	psRelocation->bSigned = bSigned;
	psRelocation->bRelative = bRelative;
	psRelocation->uiForm = psValue->uiForm;
	psRelocation->uiLine = psToken->uiLine;
	psRelocation->uiColumn = psToken->uiColumn;

	psTerm = mcTokeniser.Get(psValue->uiTermToken);
	psSymbol = mcSymbols.Get(psTerm->szText, psTerm->uiLength);
	if (psSymbol && (psSymbol->bExternal || psSymbol->bGlobal || psSymbol->bWeak))
	{
		szName.Init();
		szName.Append(psTerm->szText, psTerm->uiLength);
		psRelocation->iTargetSection = ASM65_TARGET_SYMBOL;
		psRelocation->uiTargetSymbol = mpcObject->PutSymbol(szName.Text());
		psRelocation->iAddend = psSymbol->bExternal ? psValue->iValue : psValue->iValue - psSymbol->iValue;
		szName.Kill();
	}
	else
	{
		psRelocation->iTargetSection = psValue->iSection;
		psRelocation->iAddend = psValue->iValue;
	}
	if (bRelative)
	{
		psRelocation->iAddend -= uiFieldToNext;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An instruction's length can only grow from pass to pass.
//
//////////////////////////////////////////////////////////////////////////
uint8 CAsm65Assembler::GetMinimumWords(void)
{
	if (muiInstruction < mcMinimumWords.NumElements())
	{
		return mcMinimumWords.GetValue(muiInstruction);
	}
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::Emit(SAsm65Instruction* psInstruction)
{
	uint8							i;
	SAsm65InstructionRelocation*	psRelocation;

	if (muiInstruction >= mcMinimumWords.NumElements())
	{
		mcMinimumWords.Add(psInstruction->uiWords);
		mbChanged = true;
	}
	else if (psInstruction->uiWords > mcMinimumWords.GetValue(muiInstruction))
	{
		mcMinimumWords.SetValue(muiInstruction, psInstruction->uiWords);
		mbChanged = true;
	}

	for (i = 0; i < psInstruction->uiRelocations; i++)
	{
		psRelocation = &psInstruction->asRelocations[i];
		RecordRelocation(&psRelocation->sValue, muiAddress + 2 * psRelocation->uiWord, psRelocation->uiBytes, psRelocation->bSigned, psRelocation->bRelative, 2 * (psInstruction->uiWords - psRelocation->uiWord), psRelocation->psToken);
	}

	for (i = 0; i < psInstruction->uiWords; i++)
	{
		WriteByte((uint8)psInstruction->auiWords[i]);
		WriteByte((uint8)(psInstruction->auiWords[i] >> 8));
	}
	muiInstruction++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65Token* CAsm65Assembler::Peek(size uiOffset)
{
	size	uiIndex;

	uiIndex = muiToken + uiOffset;
	if (uiIndex >= mcTokeniser.NumTokens())
	{
		uiIndex = mcTokeniser.NumTokens() - 1;
	}
	return mcTokeniser.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// Never moves past the end of a statement.
//
//////////////////////////////////////////////////////////////////////////
SAsm65Token* CAsm65Assembler::Next(void)
{
	SAsm65Token*	psToken;

	psToken = Peek();
	if (psToken->eType != AT65_End)
	{
		muiToken++;
	}
	return psToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Accept(EAsm65Punctuator ePunctuator)
{
	if (Peek()->Is(ePunctuator))
	{
		Next();
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Expect(EAsm65Punctuator ePunctuator, const char* szWhat)
{
	if (Accept(ePunctuator))
	{
		return true;
	}
	return ErrorExpected(Peek(), szWhat);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::ExpectEnd(void)
{
	if (Peek()->IsEndOfStatement())
	{
		return true;
	}
	return Error(Peek(), "Expected the end of the line");
}


//////////////////////////////////////////////////////////////////////////
//
// Only the final pass reports errors; earlier passes may not know every value yet.
// One error per statement.  Always returns false.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::Error(SAsm65Token* psToken, const char* szMessage)
{
	if (mbFinal && !mbStatementError)
	{
		Report(&mcErrors, psToken, szMessage);
	}
	mbStatementError = true;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// "Expected what".  Always returns false.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::ErrorExpected(SAsm65Token* psToken, const char* szWhat)
{
	CChars	szMessage;
	bool	bResult;

	szMessage.Init("Expected ");
	szMessage.Append(szWhat);
	bResult = Error(psToken, szMessage.Text());
	szMessage.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Warnings are reported by the final pass and do not stop assembly.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::Warning(SAsm65Token* psToken, const char* szMessage)
{
	if (mbFinal)
	{
		Report(&mcWarnings, psToken, szMessage);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::Report(CAsm65Errors* pcReport, SAsm65Token* psToken, const char* szMessage)
{
	CChars	szText;

	if (psToken->eType == AT65_NewLine)
	{
		pcReport->Add(psToken->uiLine, psToken->uiColumn, szMessage, "end of line");
	}
	else if (psToken->eType == AT65_End)
	{
		pcReport->Add(psToken->uiLine, psToken->uiColumn, szMessage, "end of file");
	}
	else
	{
		szText.Init();
		szText.Append(psToken->szText, psToken->uiLength);
		pcReport->Add(psToken->uiLine, psToken->uiColumn, szMessage, szText.Text());
		szText.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// 65C64 instructions are whole words.
//
//////////////////////////////////////////////////////////////////////////
uint32 CAsm65Assembler::InstructionAlignment(void)
{
	return 2;
}


//////////////////////////////////////////////////////////////////////////
//
// A directive only one processor's assembler has.  Returns false if it isn't one.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::ProcessorDirective(SAsm65Token* psDirective, bool* pbResult)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Called at the start of a pass, .org and .section.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Assembler::SectionStarted(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAsm65Output* CAsm65Assembler::GetOutput(void)
{
	return &mcOutput;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65Assembler::GetErrors(void)
{
	return mcErrors.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Assembler::NumErrors(void)
{
	return mcErrors.NumErrors();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65Assembler::GetWarnings(void)
{
	return mcWarnings.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Assembler::NumWarnings(void)
{
	return mcWarnings.NumErrors();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Assembler::GetSymbol(const char* szName, int64* piValue)
{
	SAsm65Symbol*	psSymbol;

	psSymbol = mcSymbols.Get(szName, strlen(szName));
	if ((psSymbol == NULL) || !psSymbol->bResolved)
	{
		return false;
	}
	*piValue = psSymbol->iValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CAsm65Assembler::NumPasses(void)
{
	return muiPasses;
}
