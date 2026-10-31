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
#include "Asm65Object.h"


static const char* gaszSectionKinds[ASK65_NumKinds] = { "text", "rodata", "init", "fini", "data", "bss" };
static const char* gaszBindings[3] = { "local", "global", "weak" };


//////////////////////////////////////////////////////////////////////////
//
// The kind is the name up to its first '.' after the start.
//
//////////////////////////////////////////////////////////////////////////
bool Asm65SectionKind(const char* szName, size uiLength, EAsm65SectionKind* peKind)
{
	size	uiKindLength;
	int		i;

	for (uiKindLength = 0; uiKindLength < uiLength; uiKindLength++)
	{
		if (szName[uiKindLength] == '.')
		{
			break;
		}
	}
	for (i = 0; i < ASK65_NumKinds; i++)
	{
		if ((strlen(gaszSectionKinds[i]) == uiKindLength) && (memcmp(gaszSectionKinds[i], szName, uiKindLength) == 0))
		{
			*peKind = (EAsm65SectionKind)i;
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* Asm65SectionKindName(EAsm65SectionKind eKind)
{
	return gaszSectionKinds[eKind];
}


//////////////////////////////////////////////////////////////////////////
//
// A field of uiBytes holds the value signed or unsigned (the assembler's widths and
// the linker's relocated fields).
//
//////////////////////////////////////////////////////////////////////////
bool Asm65Fits(int64 iValue, uint8 uiBytes, bool bSigned)
{
	int64	iLimit;

	if (uiBytes >= 8)
	{
		return true;
	}
	iLimit = (int64)1 << (8 * uiBytes);
	if (bSigned)
	{
		return (iValue >= -(iLimit / 2)) && (iValue < iLimit / 2);
	}
	return (iValue >= 0) && (iValue < iLimit);
}


//////////////////////////////////////////////////////////////////////////
//
// The value a relocation's form writes into its field at iFieldAddress (after a relative
// field has been made relative).  False if an in bank value is in another bank.
//
//////////////////////////////////////////////////////////////////////////
bool Asm65ApplyForm(uint8 uiForm, int64 iFieldAddress, int64* piValue)
{
	switch (uiForm)
	{
	case ASM65RF_Bank:
		*piValue = *piValue >> 16;
		return true;
	case ASM65RF_Low16:
		*piValue = *piValue & 0xFFFF;
		return true;
	case ASM65RF_InBank:
		if ((*piValue >> 16) != (iFieldAddress >> 16))
		{
			return false;
		}
		*piValue = *piValue & 0xFFFF;
		return true;
	default:
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Object::Init(const char* szName)
{
	mszName.Init(szName);
	mcSections.Init();
	mcSymbols.Init();
	mcSymbolIndices.Init(true, true);
	mcRelocations.Init();
	maszSourceFiles.Init();
	mcSourceLines.Init();
	mcFingerprints.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Object::Kill(void)
{
	size			i;
	SAsm65Section*	psSection;

	for (i = 0; i < mcFingerprints.NumElements(); i++)
	{
		mcFingerprints.Get(i)->szLabel.Kill();
	}
	mcFingerprints.Kill();
	mcSourceLines.Kill();
	maszSourceFiles.Kill();
	mcRelocations.Kill();
	mcSymbolIndices.Kill();
	for (i = 0; i < mcSymbols.NumElements(); i++)
	{
		mcSymbols.Get(i)->szName.Kill();
	}
	mcSymbols.Kill();
	for (i = 0; i < mcSections.NumElements(); i++)
	{
		psSection = mcSections.Get(i);
		psSection->cBytes.Kill();
		psSection->szGroup.Kill();
		psSection->szName.Kill();
	}
	mcSections.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65Object::GetName(void)
{
	return mszName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CAsm65Object::AddSection(const char* szName, const char* szGroup, EAsm65SectionKind eKind)
{
	SAsm65Section*	psSection;

	psSection = mcSections.Add();
	psSection->szName.Init(szName);
	psSection->szGroup.Init(szGroup);
	psSection->eKind = eKind;
	psSection->uiAlignment = 1;
	psSection->uiSize = 0;
	psSection->iAssociated = -1;
	psSection->bKeep = false;
	psSection->bResident = false;
	psSection->bAt = false;
	psSection->uiAt = 0;
	psSection->cBytes.Init();
	return (uint32)(mcSections.NumElements() - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Object::NumSections(void)
{
	return mcSections.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65Section* CAsm65Object::GetSection(size uiIndex)
{
	return mcSections.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The first section with the name in the COMDAT group (NULL for any group).
//
//////////////////////////////////////////////////////////////////////////
int32 CAsm65Object::FindSection(const char* szName, const char* szGroup)
{
	size			i;
	SAsm65Section*	psSection;

	for (i = 0; i < mcSections.NumElements(); i++)
	{
		psSection = mcSections.Get(i);
		if (psSection->szName.Equals(szName) && ((szGroup == NULL) || psSection->szGroup.Equals(szGroup)))
		{
			return (int32)i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Finds the symbol or adds it as undefined and local.
//
//////////////////////////////////////////////////////////////////////////
uint32 CAsm65Object::PutSymbol(const char* szName)
{
	uint32*				puiIndex;
	SAsm65ObjectSymbol*	psSymbol;
	uint32				uiIndex;

	puiIndex = mcSymbolIndices.Get(szName);
	if (puiIndex)
	{
		return *puiIndex;
	}

	uiIndex = (uint32)mcSymbols.NumElements();
	psSymbol = mcSymbols.Add();
	psSymbol->szName.Init(szName);
	psSymbol->iSection = ASM65_SECTION_UNDEFINED;
	psSymbol->iValue = 0;
	psSymbol->eBinding = AB65_Local;
	mcSymbolIndices.Put(szName, &uiIndex);
	return uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Object::NumSymbols(void)
{
	return mcSymbols.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65ObjectSymbol* CAsm65Object::GetSymbol(size uiIndex)
{
	return mcSymbols.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CAsm65Object::FindSymbol(const char* szName)
{
	uint32*	puiIndex;

	puiIndex = mcSymbolIndices.Get(szName);
	if (puiIndex)
	{
		return (int32)*puiIndex;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65Relocation* CAsm65Object::AddRelocation(void)
{
	SAsm65Relocation*	psRelocation;

	psRelocation = mcRelocations.Add();
	memset(psRelocation, 0, sizeof(SAsm65Relocation));
	return psRelocation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Object::NumRelocations(void)
{
	return mcRelocations.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65Relocation* CAsm65Object::GetRelocation(size uiIndex)
{
	return mcRelocations.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The code at uiOffset in the section is for line uiLine (1 based) of
// szFile.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Object::AddSourceLine(uint32 uiSection, uint32 uiOffset, const char* szFile, uint32 uiLine)
{
	SAsm65SourceLine*	psSourceLine;
	size				uiFile;

	for (uiFile = maszSourceFiles.NumElements(); uiFile > 0; uiFile--)
	{
		if (maszSourceFiles.Get(uiFile - 1)->Equals(szFile))
		{
			break;
		}
	}
	if (uiFile == 0)
	{
		maszSourceFiles.Add(szFile);
		uiFile = maszSourceFiles.NumElements();
	}
	psSourceLine = mcSourceLines.Add();
	psSourceLine->uiSection = uiSection;
	psSourceLine->uiOffset = uiOffset;
	psSourceLine->uiFile = (uint32)(uiFile - 1);
	psSourceLine->uiLine = uiLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Object::NumSourceLines(void)
{
	return mcSourceLines.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65SourceLine* CAsm65Object::GetSourceLine(size uiIndex)
{
	return mcSourceLines.SafeGet(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65Object::GetSourceFile(size uiIndex)
{
	return (uiIndex < maszSourceFiles.NumElements()) ? maszSourceFiles.Get(uiIndex)->Text() : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Object::AddFingerprint(const char* szLabel, uint32 uiHash, int64 iTime)
{
	SAsm65Fingerprint*	psFingerprint;

	psFingerprint = mcFingerprints.Add();
	psFingerprint->szLabel.Init(szLabel);
	psFingerprint->uiHash = uiHash;
	psFingerprint->iTime = iTime;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Object::NumFingerprints(void)
{
	return mcFingerprints.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65Fingerprint* CAsm65Object::GetFingerprint(size uiIndex)
{
	return mcFingerprints.SafeGet(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// A readable listing of the sections, symbols and relocations (for tests).
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Object::Dump(CChars* psz)
{
	size				i;
	size				j;
	SAsm65Section*		psSection;
	SAsm65ObjectSymbol*	psSymbol;
	SAsm65Relocation*	psRelocation;

	for (i = 0; i < mcSections.NumElements(); i++)
	{
		psSection = mcSections.Get(i);
		psz->Append("section ");
		psz->Append((uint32)i);
		psz->Append(" ");
		psz->Append(psSection->szName.Text());
		if (!psSection->szGroup.Empty())
		{
			psz->Append(" comdat ");
			psz->Append(psSection->szGroup.Text());
		}
		if (psSection->iAssociated >= 0)
		{
			psz->Append(" associated ");
			psz->Append(psSection->iAssociated);
		}
		if (psSection->bKeep)
		{
			psz->Append(" keep");
		}
		if (psSection->bResident)
		{
			psz->Append(" resident");
		}
		if (psSection->bAt)
		{
			psz->Append(" at ");
			psz->Append(psSection->uiAt);
		}
		psz->Append(" align ");
		psz->Append(psSection->uiAlignment);
		psz->Append(" size ");
		psz->Append(psSection->uiSize);
		for (j = 0; j < psSection->cBytes.NumElements(); j++)
		{
			psz->Append((j % 16 == 0) ? "\n  0x" : " 0x");
			psz->AppendHexHiLo(psSection->cBytes.Get(j), 1);
		}
		psz->AppendNewLine();
	}

	for (i = 0; i < mcSymbols.NumElements(); i++)
	{
		psSymbol = mcSymbols.Get(i);
		psz->Append("symbol ");
		psz->Append((uint32)i);
		psz->Append(" ");
		psz->Append(psSymbol->szName.Text());
		psz->Append(" ");
		psz->Append(gaszBindings[psSymbol->eBinding]);
		if (psSymbol->iSection == ASM65_SECTION_UNDEFINED)
		{
			psz->Append(" undefined");
		}
		else
		{
			if (psSymbol->iSection == ASM65_SECTION_ABSOLUTE)
			{
				psz->Append(" absolute ");
			}
			else
			{
				psz->Append(" section ");
				psz->Append(psSymbol->iSection);
				psz->Append(" + ");
			}
			psz->Append(psSymbol->iValue);
		}
		psz->AppendNewLine();
	}

	for (i = 0; i < mcRelocations.NumElements(); i++)
	{
		psRelocation = mcRelocations.Get(i);
		psz->Append("relocation section ");
		psz->Append(psRelocation->uiSection);
		psz->Append(" + ");
		psz->Append(psRelocation->uiOffset);
		psz->Append(psRelocation->bRelative ? " relative " : " absolute ");
		psz->Append(psRelocation->bSigned ? "signed " : "");
		psz->Append((uint32)psRelocation->uiBytes);
		psz->Append(" bytes = ");
		if (psRelocation->iTargetSection == ASM65_TARGET_SYMBOL)
		{
			psz->Append(mcSymbols.Get(psRelocation->uiTargetSymbol)->szName.Text());
		}
		else
		{
			psz->Append("section ");
			psz->Append(psRelocation->iTargetSection);
		}
		if (psRelocation->iAddend >= 0)
		{
			psz->Append(" + ");
			psz->Append(psRelocation->iAddend);
		}
		else
		{
			psz->Append(" - ");
			psz->Append(-psRelocation->iAddend);
		}
		if (psRelocation->uiForm == ASM65RF_Bank)
		{
			psz->Append(" >> 16");
		}
		else if (psRelocation->uiForm == ASM65RF_Low16)
		{
			psz->Append(" & 0xFFFF");
		}
		else if (psRelocation->uiForm == ASM65RF_InBank)
		{
			psz->Append(" in bank");
		}
		psz->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A file's magic string and version (objects and libraries).
//
//////////////////////////////////////////////////////////////////////////
bool Asm65WriteMagic(CFileWriter* pcWriter, const char* szMagic, int32 iVersion)
{
	return Asm65WriteText(pcWriter, szMagic) && pcWriter->WriteInt32(iVersion);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool Asm65ReadMagic(CFileReader* pcReader, const char* szMagic, int32 iVersion)
{
	CChars	szRead;
	int32	iRead;
	bool	bResult;

	szRead.Init();
	bResult = Asm65ReadText(pcReader, &szRead) && szRead.Equals(szMagic);
	szRead.Kill();
	return bResult && pcReader->ReadInt32(&iRead) && (iRead == iVersion);
}


//////////////////////////////////////////////////////////////////////////
//
// Writes a string the way CChars::WriteString does (for Asm65ReadText).
//
//////////////////////////////////////////////////////////////////////////
bool Asm65WriteText(CFileWriter* pcWriter, const char* szText)
{
	CChars	szWrite;
	bool	bResult;

	szWrite.Init(szText);
	bResult = szWrite.WriteString(pcWriter);
	szWrite.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads a string written by CChars::WriteString into an initialised CChars.
//
//////////////////////////////////////////////////////////////////////////
bool Asm65ReadText(CFileReader* pcReader, CChars* psz)
{
	CChars	szRead;

	if (!szRead.ReadString(pcReader))
	{
		return false;
	}
	psz->Kill();
	psz->Init(szRead.Text());
	szRead.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the object as a ChunkFile.  pcFile is freed by the chunk file when it is
// a must free file (DiskFile, MemoryFile).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Object::Write(CAbstractFile* pcFile)
{
	CChunkFile	cChunkFile;
	bool		bResult;

	cChunkFile.Init(pcFile);
	bResult = cChunkFile.WriteOpen();
	if (bResult)
	{
		bResult = WriteChunks(&cChunkFile);
		bResult &= cChunkFile.WriteClose();
	}
	cChunkFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Object::WriteChunks(CChunkFile* pcChunkFile)
{
	size				i;
	SAsm65Section*		psSection;
	SAsm65ObjectSymbol*	psSymbol;
	SAsm65Relocation*	psRelocation;
	SAsm65SourceLine*	psSourceLine;
	SAsm65Fingerprint*	psFingerprint;
	bool				bResult;

	bResult = pcChunkFile->WriteChunkBegin();
	bResult &= Asm65WriteMagic(pcChunkFile, ASM65_OBJECT_MAGIC, ASM65_OBJECT_VERSION);
	bResult &= mszName.WriteString(pcChunkFile);
	bResult &= pcChunkFile->WriteChunkEnd("Header");

	bResult &= pcChunkFile->WriteChunkBegin();
	bResult &= pcChunkFile->WriteInt32((uint32)mcSections.NumElements());
	for (i = 0; i < mcSections.NumElements(); i++)
	{
		psSection = mcSections.Get(i);
		bResult &= psSection->szName.WriteString(pcChunkFile);
		bResult &= psSection->szGroup.WriteString(pcChunkFile);
		bResult &= pcChunkFile->WriteInt32((int32)psSection->eKind);
		bResult &= pcChunkFile->WriteInt32(psSection->uiAlignment);
		bResult &= pcChunkFile->WriteInt32(psSection->uiSize);
		bResult &= pcChunkFile->WriteInt32(psSection->iAssociated);
		bResult &= pcChunkFile->WriteBool(psSection->bKeep);
		bResult &= pcChunkFile->WriteBool(psSection->bResident);
		bResult &= pcChunkFile->WriteBool(psSection->bAt);
		bResult &= pcChunkFile->WriteInt32(psSection->uiAt);
		bResult &= pcChunkFile->WriteInt32((uint32)psSection->cBytes.NumElements());
		if (psSection->cBytes.NumElements() != 0)
		{
			bResult &= pcChunkFile->WriteData(psSection->cBytes.GetData(), psSection->cBytes.NumElements());
		}
	}
	bResult &= pcChunkFile->WriteChunkEnd("Sections");

	bResult &= pcChunkFile->WriteChunkBegin();
	bResult &= pcChunkFile->WriteInt32((uint32)mcSymbols.NumElements());
	for (i = 0; i < mcSymbols.NumElements(); i++)
	{
		psSymbol = mcSymbols.Get(i);
		bResult &= psSymbol->szName.WriteString(pcChunkFile);
		bResult &= pcChunkFile->WriteInt32(psSymbol->iSection);
		bResult &= pcChunkFile->WriteInt64(psSymbol->iValue);
		bResult &= pcChunkFile->WriteInt32((int32)psSymbol->eBinding);
	}
	bResult &= pcChunkFile->WriteChunkEnd("Symbols");

	bResult &= pcChunkFile->WriteChunkBegin();
	bResult &= pcChunkFile->WriteInt32((uint32)mcRelocations.NumElements());
	for (i = 0; i < mcRelocations.NumElements(); i++)
	{
		psRelocation = mcRelocations.Get(i);
		bResult &= pcChunkFile->WriteInt32(psRelocation->uiSection);
		bResult &= pcChunkFile->WriteInt32(psRelocation->uiOffset);
		bResult &= pcChunkFile->WriteInt32(psRelocation->iTargetSection);
		bResult &= pcChunkFile->WriteInt32(psRelocation->uiTargetSymbol);
		bResult &= pcChunkFile->WriteInt64(psRelocation->iAddend);
		bResult &= pcChunkFile->WriteInt8(psRelocation->uiBytes);
		bResult &= pcChunkFile->WriteBool(psRelocation->bSigned);
		bResult &= pcChunkFile->WriteBool(psRelocation->bRelative);
		bResult &= pcChunkFile->WriteInt8(psRelocation->uiForm);
		bResult &= pcChunkFile->WriteInt32(psRelocation->uiLine);
		bResult &= pcChunkFile->WriteInt32(psRelocation->uiColumn);
	}
	bResult &= pcChunkFile->WriteChunkEnd("Relocations");

	bResult &= pcChunkFile->WriteChunkBegin();
	bResult &= pcChunkFile->WriteInt32((uint32)maszSourceFiles.NumElements());
	for (i = 0; i < maszSourceFiles.NumElements(); i++)
	{
		bResult &= maszSourceFiles.Get(i)->WriteString(pcChunkFile);
	}
	bResult &= pcChunkFile->WriteInt32((uint32)mcSourceLines.NumElements());
	for (i = 0; i < mcSourceLines.NumElements(); i++)
	{
		psSourceLine = mcSourceLines.Get(i);
		bResult &= pcChunkFile->WriteInt32(psSourceLine->uiSection);
		bResult &= pcChunkFile->WriteInt32(psSourceLine->uiOffset);
		bResult &= pcChunkFile->WriteInt32(psSourceLine->uiFile);
		bResult &= pcChunkFile->WriteInt32(psSourceLine->uiLine);
	}
	bResult &= pcChunkFile->WriteInt32((uint32)mcFingerprints.NumElements());
	for (i = 0; i < mcFingerprints.NumElements(); i++)
	{
		psFingerprint = mcFingerprints.Get(i);
		bResult &= psFingerprint->szLabel.WriteString(pcChunkFile);
		bResult &= pcChunkFile->WriteInt32(psFingerprint->uiHash);
		bResult &= pcChunkFile->WriteInt64(psFingerprint->iTime);
	}
	bResult &= pcChunkFile->WriteChunkEnd("Sources");
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads an object written by Write into this (empty, initialised) object.  Returns
// false for a file that is not a valid object; the object must still be killed.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Object::Read(CAbstractFile* pcFile)
{
	CChunkFile	cChunkFile;
	bool		bResult;

	//ReadClose frees what a failed ReadOpen allocated too.
	cChunkFile.Init(pcFile);
	bResult = cChunkFile.ReadOpen();
	bResult = bResult && ReadChunks(&cChunkFile);
	bResult &= cChunkFile.ReadClose();
	cChunkFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Object::ReadChunks(CChunkFile* pcChunkFile)
{
	if ((mcSections.NumElements() != 0) || (mcSymbols.NumElements() != 0) || (mcRelocations.NumElements() != 0))
	{
		return false;
	}
	return ReadHeader(pcChunkFile) && ReadSections(pcChunkFile) && ReadSymbols(pcChunkFile) && ReadRelocations(pcChunkFile) && ReadSources(pcChunkFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Object::ReadHeader(CChunkFile* pcChunkFile)
{
	return pcChunkFile->ReadChunkBegin("Header") &&
		Asm65ReadMagic(pcChunkFile, ASM65_OBJECT_MAGIC, ASM65_OBJECT_VERSION) &&
		Asm65ReadText(pcChunkFile, &mszName) &&
		pcChunkFile->ReadChunkEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Object::ReadSections(CChunkFile* pcChunkFile)
{
	uint32			uiCount;
	uint32			i;
	uint32			uiBytes;
	int32			iKind;
	SAsm65Section*	psSection;

	if (!pcChunkFile->ReadChunkBegin("Sections") || !pcChunkFile->ReadInt32(&uiCount))
	{
		return false;
	}
	for (i = 0; i < uiCount; i++)
	{
		psSection = mcSections.Get(AddSection("", "", ASK65_Text));
		if (!Asm65ReadText(pcChunkFile, &psSection->szName) || !Asm65ReadText(pcChunkFile, &psSection->szGroup) ||
			!pcChunkFile->ReadInt32(&iKind) || !pcChunkFile->ReadInt32(&psSection->uiAlignment) ||
			!pcChunkFile->ReadInt32(&psSection->uiSize) || !pcChunkFile->ReadInt32(&psSection->iAssociated) ||
			!pcChunkFile->ReadBool(&psSection->bKeep) || !pcChunkFile->ReadBool(&psSection->bResident) ||
			!pcChunkFile->ReadBool(&psSection->bAt) || !pcChunkFile->ReadInt32(&psSection->uiAt) || !pcChunkFile->ReadInt32(&uiBytes))
		{
			return false;
		}
		if ((iKind < 0) || (iKind >= ASK65_NumKinds) || (uiBytes > psSection->uiSize) ||
			(psSection->uiAlignment == 0) || ((psSection->uiAlignment & (psSection->uiAlignment - 1)) != 0))
		{
			return false;
		}
		if ((psSection->iAssociated < -1) || (psSection->iAssociated >= (int32)uiCount))
		{
			return false;
		}
		psSection->eKind = (EAsm65SectionKind)iKind;
		if (uiBytes != 0)
		{
			psSection->cBytes.Resize(uiBytes);
			if (!pcChunkFile->ReadData(psSection->cBytes.GetData(), uiBytes))
			{
				return false;
			}
		}
	}
	return pcChunkFile->ReadChunkEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Object::ReadSymbols(CChunkFile* pcChunkFile)
{
	uint32				uiCount;
	uint32				i;
	int32				iBinding;
	CChars				szName;
	SAsm65ObjectSymbol*	psSymbol;
	bool				bResult;

	if (!pcChunkFile->ReadChunkBegin("Symbols") || !pcChunkFile->ReadInt32(&uiCount))
	{
		return false;
	}
	for (i = 0; i < uiCount; i++)
	{
		szName.Init();
		bResult = Asm65ReadText(pcChunkFile, &szName) && (FindSymbol(szName.Text()) < 0);
		if (!bResult)
		{
			szName.Kill();
			return false;
		}
		psSymbol = mcSymbols.Get(PutSymbol(szName.Text()));
		szName.Kill();
		if (!pcChunkFile->ReadInt32(&psSymbol->iSection) || !pcChunkFile->ReadInt64(&psSymbol->iValue) || !pcChunkFile->ReadInt32(&iBinding))
		{
			return false;
		}
		if ((iBinding < AB65_Local) || (iBinding > AB65_Weak) || (psSymbol->iSection < ASM65_SECTION_UNDEFINED) || (psSymbol->iSection >= (int32)mcSections.NumElements()))
		{
			return false;
		}
		psSymbol->eBinding = (EAsm65Binding)iBinding;
	}
	return pcChunkFile->ReadChunkEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Object::ReadRelocations(CChunkFile* pcChunkFile)
{
	uint32				uiCount;
	uint32				i;
	SAsm65Relocation*	psRelocation;

	if (!pcChunkFile->ReadChunkBegin("Relocations") || !pcChunkFile->ReadInt32(&uiCount))
	{
		return false;
	}
	for (i = 0; i < uiCount; i++)
	{
		psRelocation = AddRelocation();
		if (!pcChunkFile->ReadInt32(&psRelocation->uiSection) || !pcChunkFile->ReadInt32(&psRelocation->uiOffset) ||
			!pcChunkFile->ReadInt32(&psRelocation->iTargetSection) || !pcChunkFile->ReadInt32(&psRelocation->uiTargetSymbol) ||
			!pcChunkFile->ReadInt64(&psRelocation->iAddend) || !pcChunkFile->ReadInt8(&psRelocation->uiBytes) ||
			!pcChunkFile->ReadBool(&psRelocation->bSigned) || !pcChunkFile->ReadBool(&psRelocation->bRelative) ||
			!pcChunkFile->ReadInt8(&psRelocation->uiForm) || !pcChunkFile->ReadInt32(&psRelocation->uiLine) || !pcChunkFile->ReadInt32(&psRelocation->uiColumn))
		{
			return false;
		}
		if ((psRelocation->uiSection >= mcSections.NumElements()) || (psRelocation->uiBytes == 0) || (psRelocation->uiBytes > 8) || (psRelocation->uiForm >= ASM65RF_NumForms) ||
			((uint64)psRelocation->uiOffset + psRelocation->uiBytes > mcSections.Get(psRelocation->uiSection)->cBytes.NumElements()))
		{
			return false;
		}
		if (psRelocation->iTargetSection == ASM65_TARGET_SYMBOL)
		{
			if (psRelocation->uiTargetSymbol >= mcSymbols.NumElements())
			{
				return false;
			}
		}
		else if ((psRelocation->iTargetSection < 0) || (psRelocation->iTargetSection >= (int32)mcSections.NumElements()))
		{
			return false;
		}
	}
	return pcChunkFile->ReadChunkEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Object::ReadSources(CChunkFile* pcChunkFile)
{
	uint32				uiCount;
	uint32				i;
	SAsm65SourceLine*	psSourceLine;
	SAsm65Fingerprint*	psFingerprint;
	CChars				szFile;

	if (!pcChunkFile->ReadChunkBegin("Sources") || !pcChunkFile->ReadInt32(&uiCount))
	{
		return false;
	}
	for (i = 0; i < uiCount; i++)
	{
		szFile.Init();
		if (!Asm65ReadText(pcChunkFile, &szFile))
		{
			szFile.Kill();
			return false;
		}
		maszSourceFiles.Add(szFile.Text());
		szFile.Kill();
	}
	if (!pcChunkFile->ReadInt32(&uiCount))
	{
		return false;
	}
	for (i = 0; i < uiCount; i++)
	{
		psSourceLine = mcSourceLines.Add();
		if (!pcChunkFile->ReadInt32(&psSourceLine->uiSection) || !pcChunkFile->ReadInt32(&psSourceLine->uiOffset) ||
			!pcChunkFile->ReadInt32(&psSourceLine->uiFile) || !pcChunkFile->ReadInt32(&psSourceLine->uiLine))
		{
			return false;
		}
		if ((psSourceLine->uiSection >= mcSections.NumElements()) || (psSourceLine->uiFile >= maszSourceFiles.NumElements()))
		{
			return false;
		}
	}
	if (!pcChunkFile->ReadInt32(&uiCount))
	{
		return false;
	}
	for (i = 0; i < uiCount; i++)
	{
		psFingerprint = mcFingerprints.Add();
		psFingerprint->szLabel.Init();
		if (!Asm65ReadText(pcChunkFile, &psFingerprint->szLabel) || !pcChunkFile->ReadInt32(&psFingerprint->uiHash) || !pcChunkFile->ReadInt64(&psFingerprint->iTime))
		{
			return false;
		}
	}
	return pcChunkFile->ReadChunkEnd();
}
