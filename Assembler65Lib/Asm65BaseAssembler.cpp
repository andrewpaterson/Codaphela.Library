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
#include "Asm65BaseAssembler.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65BaseAssembler::Init(void)
{
	mcErrors.Init();
	mcWarnings.Init();
	mcTokeniser.Init(&mcErrors);
	mcSymbols.Init();
	mcOutput.Init();
	mcMinimumLengths.Init();
	mcPendingLabels.Init();
	mpcSource = NULL;
	muiLine = 0;

	muiToken = 0;
	muiAddress = 0;
	muiStatementAddress = 0;
	muiPass = 0;
	mbFinal = false;
	mbChanged = false;
	mbStatementError = false;
	mbRelative = false;
	muiInstruction = 0;
	muiPasses = 0;

	mbObject = false;
	mpcObject = NULL;
	muiSection = 0;
	mcSectionAddresses.Init();
	mcUndefined.Init();
	mcUndefinedNames.Init();
	mcAssociatedSections.Init();
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
void CAsm65BaseAssembler::Kill(void)
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
	mcAssociatedSections.Kill();
	mcUndefinedNames.Kill();
	mcUndefined.Kill();
	mcSectionAddresses.Kill();
	mcPendingLabels.Kill();
	mcMinimumLengths.Kill();
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
bool CAsm65BaseAssembler::Assemble(const char* szSource)
{
	bool	bResult;

	mcTextSource.Init(szSource);
	bResult = Assemble(&mcTextSource);
	mcTextSource.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The source is read a line at a time, every pass.  A line the tokeniser can't
// read stops the assembly before the first pass.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::Assemble(CAsm65Source* pcSource)
{
	bool	bResult;

	mpcSource = pcSource;
	bResult = CheckTokens();
	if (!bResult)
	{
		mpcSource = NULL;
		return false;
	}
	mcTokeniser.SetErrors(NULL);
	bResult = Passes();
	mcTokeniser.SetErrors(&mcErrors);
	mpcSource = NULL;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::Passes(void)
{
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
bool CAsm65BaseAssembler::AssembleObject(const char* szSource, CAsm65Object* pcObject)
{
	bool	bResult;

	mcTextSource.Init(szSource);
	bResult = AssembleObject(&mcTextSource, pcObject);
	mcTextSource.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::AssembleObject(CAsm65Source* pcSource, CAsm65Object* pcObject)
{
	bool	bResult;

	mbObject = true;
	mpcObject = pcObject;
	if (mpcObject->FindSection("text", "") != 0)
	{
		mpcObject->AddSection("text", "", ASK65_Text);
	}

	bResult = Assemble(pcSource);
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
// Tokenises every line once, reporting the tokeniser's errors (the passes don't
// report them again).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::CheckTokens(void)
{
	const char*	szLine;
	uint32		uiLine;
	bool		bResult;
	bool		bLine;

	bResult = true;
	mpcSource->Rewind();
	for (uiLine = 1; (szLine = mpcSource->NextLine()) != NULL; uiLine++)
	{
		mcTokeniser.TokeniseLine(szLine, uiLine, &bLine);
		bResult &= bLine;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the next line's tokens.  False after the last line.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::NextLine(void)
{
	const char*	szLine;
	bool		bResult;

	szLine = mpcSource->NextLine();
	if (szLine == NULL)
	{
		return false;
	}
	muiLine++;
	mcTokeniser.TokeniseLine(szLine, muiLine, &bResult);
	muiToken = 0;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65BaseAssembler::Pass(void)
{
	size			i;
	SAsm65Section*	psSection;

	muiToken = 0;
	muiAddress = 0;
	muiInstruction = 0;
	miLabelShift = 0;
	mcPendingLabels.Clear();
	mcUndefined.Clear();
	mcUndefinedNames.Kill();
	mcUndefinedNames.Init();
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
	mpcSource->Rewind();
	muiLine = 0;
	while (NextLine() && (Peek()->eType != AT65_End))
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
void CAsm65BaseAssembler::MarkExternals(void)
{
	lsize			i;
	SAsm65Token*	psToken;
	SAsm65Symbol*	psSymbol;

	for (i = 0; i < mcUndefined.NumTokens(); i++)
	{
		psToken = mcUndefined.Get(i);
		psSymbol = mcSymbols.Get(psToken->szText, psToken->uiLength);
		if ((psSymbol == NULL) || (!psSymbol->bResolved && (psSymbol->uiPass != muiPass)))
		{
			MakeExternal(mcSymbols.Put(psToken->szText, psToken->uiLength), psToken);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The symbol is defined by another object; psNamed is where it was named.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65BaseAssembler::MakeExternal(SAsm65Symbol* psSymbol, SAsm65Token* psNamed)
{
	psSymbol->iValue = 0;
	psSymbol->bResolved = true;
	psSymbol->bExternal = true;
	psSymbol->iSection = ASM65_SECTION_UNDEFINED;
	psSymbol->uiTerm = psSymbol->uiIndex;
	psSymbol->uiNamedLine = psNamed->uiLine;
	psSymbol->uiNamedColumn = psNamed->uiColumn;
	mbChanged = true;
}


//////////////////////////////////////////////////////////////////////////
//
// A name used before it is defined (or never defined: an external).  Only its
// first use in a pass matters (MarkExternals), so later ones aren't kept.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65BaseAssembler::AddUndefined(SAsm65Token* psToken)
{
	CChars	szName;

	szName.Init();
	szName.Append(psToken->szText, psToken->uiLength);
	if (mcUndefinedNames.Get(szName.Text(), -1) == -1)
	{
		mcUndefinedNames.Put(szName.Text(), 0);
		mcUndefined.Add(psToken);
	}
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Every label and constant goes into the object (locals too, for the map) with
// externals that are declared or used by a relocation.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65BaseAssembler::BuildObjectSymbols(void)
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
				ReportTerm(psSymbol->uiTerm, "A global symbol cannot be an external plus a value");
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
void CAsm65BaseAssembler::Statement(void)
{
	SAsm65Token*	psToken;
	bool			bResult;

	mbStatementError = false;
	mbRelative = false;
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
			mcPendingLabels.Add(psToken);
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
			else if (psToken->IsIdentifierIgnoreCase(".line"))
			{
				bResult = SourceLine(psToken);
			}
			else if (psToken->IsIdentifierIgnoreCase(".fingerprint"))
			{
				bResult = Fingerprint(psToken);
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
void CAsm65BaseAssembler::BindLabels(void)
{
	lsize			i;
	SAsm65Token*	psLabel;
	SAsm65Symbol*	psSymbol;
	int32			iSection;

	iSection = mbObject ? (int32)muiSection : ASM65_SECTION_ABSOLUTE;
	for (i = 0; i < mcPendingLabels.NumTokens(); i++)
	{
		psLabel = mcPendingLabels.Get(i);
		psSymbol = mcSymbols.Get(psLabel->szText, psLabel->uiLength);
		if (psSymbol && psSymbol->bLabel && psSymbol->bResolved && (psSymbol->uiPass + 1 == muiPass) && (psSymbol->iSection == iSection))
		{
			miLabelShift = (int64)muiAddress - psSymbol->iValue;
		}
		DefineSymbol(psLabel, muiAddress, true, iSection, ASM65_TERM_SELF);
		mcSymbols.Get(psLabel->szText, psLabel->uiLength)->bLabel = true;
	}
	mcPendingLabels.Clear();
}


//////////////////////////////////////////////////////////////////////////
//
// A symbol defined twice in one pass is an error.  A symbol whose value changed since
// the last pass needs another pass.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65BaseAssembler::DefineSymbol(SAsm65Token* psName, int64 iValue, bool bResolved, int32 iSection, size uiTerm)
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
	psSymbol->uiTerm = (uiTerm == ASM65_TERM_SELF) ? psSymbol->uiIndex : uiTerm;
}


//////////////////////////////////////////////////////////////////////////
//
// name = expression
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::Constant(SAsm65Token* psName)
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
	DefineSymbol(psName, sValue.iValue, sValue.bResolved, sValue.iSection, sValue.uiTerm);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// .org address
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::Origin(void)
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
bool CAsm65BaseAssembler::Align(void)
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
bool CAsm65BaseAssembler::Data(uint8 uiBytes)
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
void CAsm65BaseAssembler::AlignTo(uint32 uiAlignment)
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
void CAsm65BaseAssembler::WriteByte(uint8 uiByte)
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
bool CAsm65BaseAssembler::Space(void)
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
// .inchex "file" {, start {, length}}
// The bytes of an Intel HEX file from address start (its lowest address when
// not given) to its highest, or length bytes from start (a slice): $FF where
// the file has none (as an erased ROM).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::IncludeHex(void)
{
	SAsm65Token*	psName;
	SAsm65Token*	psToken;
	CIntelHex*		pcHex;
	int64			iStart;
	int64			iLength;
	uint64			ulliEnd;
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

	iStart = pcHex->GetLowAddress();
	ulliEnd = pcHex->GetEndAddress();
	iLength = (ulliEnd > (uint64)iStart) ? (int64)(ulliEnd - (uint64)iStart) : 0;
	if (Accept(AP65_Comma))
	{
		psToken = Peek();
		if (!ExpressionValue(&iStart))
		{
			return false;
		}
		if ((iStart < 0) || (iStart > 0xFFFFFFFFLL))
		{
			return Error(psToken, "The start must be a 32 bit address");
		}
		iLength = (ulliEnd > (uint64)iStart) ? (int64)(ulliEnd - (uint64)iStart) : 0;
		if (Accept(AP65_Comma))
		{
			psToken = Peek();
			if (!ExpressionValue(&iLength))
			{
				return false;
			}
		}
	}
	if ((iLength < 0) || (iLength > 0x1000000))
	{
		return Error(psName, "Included bytes must be 0 to 16MB");
	}
	if (!mbFinal)
	{
		muiAddress += (uint32)iLength;
		return true;
	}
	puiBytes = (uint8*)malloc((size)iLength + 1);
	pcHex->Fill((uint32)iStart, (uint32)iLength, 0xFF, puiBytes);
	for (i = 0; i < (uint32)iLength; i++)
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
CIntelHex* CAsm65BaseAssembler::GetHexFile(SAsm65Token* psName)
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
void CAsm65BaseAssembler::SetDirectory(const char* szDirectory)
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
bool CAsm65BaseAssembler::Section(void)
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
		mcAssociatedSections.Add(iSection);
		mcAssociations.Add(psAssociated);
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
bool CAsm65BaseAssembler::SectionOptions(SAsm65Token* psName, SAsm65Token** ppsGroup, SAsm65Token** ppsAssociated, bool* pbKeep, bool* pbResident, bool* pbAt, uint32* puiAt)
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
void CAsm65BaseAssembler::Associate(void)
{
	size			i;
	SAsm65Token*	psName;
	CChars			szName;
	int32			iSection;

	for (i = 0; i < mcAssociatedSections.NumElements(); i++)
	{
		psName = mcAssociations.Get(i);
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
			mpcObject->GetSection(mcAssociatedSections.GetValue(i))->iAssociated = iSection;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65BaseAssembler::SwitchSection(uint32 uiSection)
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
bool CAsm65BaseAssembler::InBss(SAsm65Token* psToken)
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
bool CAsm65BaseAssembler::SymbolBinding(SAsm65Token* psDirective)
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
			MakeExternal(psSymbol, psName);
		}
	}
	while (Accept(AP65_Comma));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// .line "file", line
// The code from here is for the line (1 based) of the source file, for the
// linker's line table (CAsm65Output) and a profile of the program by line.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::SourceLine(SAsm65Token* psDirective)
{
	SAsm65Token*	psFile;
	SAsm65Token*	psLine;
	int64			iLine;

	if (!mbObject)
	{
		return Error(psDirective, ".line is only allowed when assembling an object");
	}
	psFile = Peek();
	if (psFile->eType != AT65_String)
	{
		return ErrorExpected(psFile, "a file name");
	}
	Next();
	if (!Accept(AP65_Comma))
	{
		return ErrorExpected(Peek(), "','");
	}
	psLine = Peek();
	if (!ExpressionValue(&iLine))
	{
		return false;
	}
	if ((iLine < 1) || (iLine > 0x7FFFFFFF))
	{
		return Error(psLine, "A line number must be 1 or more");
	}
	if (mbFinal)
	{
		mpcObject->AddSourceLine(muiSection, muiAddress, mcTokeniser.GetString(psFile), (uint32)iLine);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// .fingerprint "label", hash, time
// The function at the label was compiled from code with the hash (32 bits)
// at the time (seconds since 1970): a profile of it is for that code.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::Fingerprint(SAsm65Token* psDirective)
{
	SAsm65Token*	psLabel;
	SAsm65Token*	psHash;
	int64			iHash;
	int64			iTime;

	if (!mbObject)
	{
		return Error(psDirective, ".fingerprint is only allowed when assembling an object");
	}
	psLabel = Peek();
	if (psLabel->eType != AT65_String)
	{
		return ErrorExpected(psLabel, "a label");
	}
	Next();
	if (!Accept(AP65_Comma))
	{
		return ErrorExpected(Peek(), "','");
	}
	psHash = Peek();
	if (!ExpressionValue(&iHash))
	{
		return false;
	}
	if ((iHash < 0) || (iHash > 0xFFFFFFFFll))
	{
		return Error(psHash, "A fingerprint's hash is 32 bits");
	}
	if (!Accept(AP65_Comma))
	{
		return ErrorExpected(Peek(), "','");
	}
	if (!ExpressionValue(&iTime))
	{
		return false;
	}
	if (mbFinal)
	{
		mpcObject->AddFingerprint(mcTokeniser.GetString(psLabel), (uint32)iHash, iTime);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A value the linker must finish: an address in a section or an external.  A value
// that is not resolved yet is sized like any forward reference (it grows next pass).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::IsRelocatable(SAsm65Value* psValue)
{
	return mbObject && psValue->bResolved && (psValue->iTerms != 0);
}


//////////////////////////////////////////////////////////////////////////
//
// A relative target in the current section is an offset the assembler knows.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::IsLocalTarget(SAsm65Value* psValue)
{
	return mbObject && psValue->bResolved && (psValue->iTerms == 1) && (psValue->iSection == (int32)muiSection);
}


//////////////////////////////////////////////////////////////////////////
//
// A relative target in another section or an external needs a relocation.  A target
// in this section becomes a plain offset (like the statement's address).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::TargetIsRelocatable(SAsm65Value* psTarget)
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
void CAsm65BaseAssembler::UseTerm(void)
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
void CAsm65BaseAssembler::RecordRelocation(SAsm65Value* psValue, uint32 uiOffset, uint8 uiBytes, bool bSigned, bool bRelative, uint32 uiFieldToNext, SAsm65Token* psToken)
{
	SAsm65Relocation*	psRelocation;
	char*				szTerm;
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

	szTerm = mcSymbols.GetName(psValue->uiTerm);
	psSymbol = mcSymbols.Get(szTerm, strlen(szTerm));
	if (psSymbol && (psSymbol->bExternal || psSymbol->bGlobal || psSymbol->bWeak))
	{
		szName.Init();
		szName.Append(szTerm);
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
uint8 CAsm65BaseAssembler::GetMinimumLength(void)
{
	if (muiInstruction < mcMinimumLengths.NumElements())
	{
		return mcMinimumLengths.GetValue(muiInstruction);
	}
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Records the length of the instruction just assembled (in words on the 65C64, bytes
// on the W65C816S) and moves on to the next.  A longer length changes the pass.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65BaseAssembler::RecordLength(uint8 uiLength)
{
	if (muiInstruction >= mcMinimumLengths.NumElements())
	{
		mcMinimumLengths.Add(uiLength);
		mbChanged = true;
	}
	else if (uiLength > mcMinimumLengths.GetValue(muiInstruction))
	{
		mcMinimumLengths.SetValue(muiInstruction, uiLength);
		mbChanged = true;
	}
	muiInstruction++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65Token* CAsm65BaseAssembler::Peek(size uiOffset)
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
SAsm65Token* CAsm65BaseAssembler::Next(void)
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
bool CAsm65BaseAssembler::Accept(EAsm65Punctuator ePunctuator)
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
bool CAsm65BaseAssembler::Expect(EAsm65Punctuator ePunctuator, const char* szWhat)
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
bool CAsm65BaseAssembler::ExpectEnd(void)
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
bool CAsm65BaseAssembler::Error(SAsm65Token* psToken, const char* szMessage)
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
// Reports at where the term (a symbol's index) was named.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65BaseAssembler::ReportTerm(size uiTerm, const char* szMessage)
{
	SAsm65Token		sToken;
	SAsm65Symbol*	psTerm;
	char*			szName;

	szName = mcSymbols.GetName(uiTerm);
	psTerm = mcSymbols.Get(szName, strlen(szName));
	memset(&sToken, 0, sizeof(sToken));
	sToken.eType = AT65_Identifier;
	sToken.uiLine = psTerm->uiNamedLine;
	sToken.uiColumn = psTerm->uiNamedColumn;
	sToken.szText = szName;
	sToken.uiLength = (uint32)strlen(szName);
	Report(&mcErrors, &sToken, szMessage);
}


//////////////////////////////////////////////////////////////////////////
//
// "Expected what".  Always returns false.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::ErrorExpected(SAsm65Token* psToken, const char* szWhat)
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
void CAsm65BaseAssembler::Warning(SAsm65Token* psToken, const char* szMessage)
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
void CAsm65BaseAssembler::Report(CAsm65Errors* pcReport, SAsm65Token* psToken, const char* szMessage)
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
// A directive only one processor's assembler has.  Returns false if it isn't one.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::ProcessorDirective(SAsm65Token* psDirective, bool* pbResult)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Called at the start of a pass, .org and .section.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65BaseAssembler::SectionStarted(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAsm65Output* CAsm65BaseAssembler::GetOutput(void)
{
	return &mcOutput;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65BaseAssembler::GetErrors(void)
{
	return mcErrors.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65BaseAssembler::NumErrors(void)
{
	return mcErrors.NumErrors();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65BaseAssembler::GetWarnings(void)
{
	return mcWarnings.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65BaseAssembler::NumWarnings(void)
{
	return mcWarnings.NumErrors();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::GetSymbol(const char* szName, int64* piValue)
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
uint32 CAsm65BaseAssembler::NumPasses(void)
{
	return muiPasses;
}
