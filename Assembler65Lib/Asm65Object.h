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
#ifndef __ASM65_OBJECT_H__
#define __ASM65_OBJECT_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayUint8.h"
#include "BaseLib/MapStringTemplate.h"
#include "BaseLib/Chars.h"
#include "BaseLib/AbstractFile.h"
#include "BaseLib/ChunkFile.h"


//A symbol's section when it is not in one.
#define ASM65_SECTION_ABSOLUTE	-1
#define ASM65_SECTION_UNDEFINED	-2

//A relocation that targets a symbol instead of a section.
#define ASM65_TARGET_SYMBOL		-1

//The object file: a ChunkFile with Header, Sections, Symbols and Relocations chunks.
#define ASM65_OBJECT_MAGIC		"Asm65Object"
#define ASM65_OBJECT_VERSION	3


//The kind of a section comes from its name: "text", "text.anything", "rodata.x" ...
//The linker places the kinds in this order.
enum EAsm65SectionKind
{
	ASK65_Text,
	ASK65_ReadOnly,
	ASK65_Init,
	ASK65_Fini,
	ASK65_Data,
	ASK65_Bss,
	ASK65_NumKinds,
};


enum EAsm65Binding
{
	AB65_Local,
	AB65_Global,
	AB65_Weak,
};


//Bytes that move together.  A bss section has a size but no bytes.  A section in a
//COMDAT group (szGroup not empty) is kept from the first object that has the group
//and dropped from the others.  When the linker removes unused sections, a keep
//section is always used, and an associated section (iAssociated, in this object) is
//used when the section it is associated with is.  A resident section is never loaded
//lazily.
struct SAsm65Section
{
	CChars				szName;
	CChars				szGroup;
	EAsm65SectionKind	eKind;
	uint32				uiAlignment;
	uint32				uiSize;
	int32				iAssociated;
	bool				bKeep;
	bool				bResident;
	CArrayUint8			cBytes;
};


//A section offset, an absolute value or an undefined name (to be found by the linker).
struct SAsm65ObjectSymbol
{
	CChars			szName;
	int32			iSection;
	int64			iValue;
	EAsm65Binding	eBinding;
};


//A field the linker writes: the target (a section's address or a symbol's value) plus
//iAddend.  A relative field holds the result minus the field's own address.  The
//field is uiBytes little endian bytes and must hold the result, signed or unsigned.
struct SAsm65Relocation
{
	uint32	uiSection;
	uint32	uiOffset;
	int32	iTargetSection;
	uint32	uiTargetSymbol;
	int64	iAddend;
	uint8	uiBytes;
	bool	bSigned;
	bool	bRelative;
	uint32	uiLine;
	uint32	uiColumn;
};


typedef CArrayTemplate<SAsm65Section>		CArrayAsm65Sections;
typedef CArrayTemplate<SAsm65ObjectSymbol>	CArrayAsm65ObjectSymbols;
typedef CArrayTemplate<SAsm65Relocation>	CArrayAsm65Relocations;


//An assembled translation unit: sections, symbols and relocations.
class CAsm65Object
{
protected:
	CChars						mszName;
	CArrayAsm65Sections			mcSections;
	CArrayAsm65ObjectSymbols	mcSymbols;
	CMapStringTemplate<uint32>	mcSymbolIndices;
	CArrayAsm65Relocations		mcRelocations;

public:
	void				Init(const char* szName);
	void				Kill(void);

	char*				GetName(void);

	uint32				AddSection(const char* szName, const char* szGroup, EAsm65SectionKind eKind);
	size				NumSections(void);
	SAsm65Section*		GetSection(size uiIndex);
	int32				FindSection(const char* szName, const char* szGroup);

	uint32				PutSymbol(const char* szName);
	size				NumSymbols(void);
	SAsm65ObjectSymbol*	GetSymbol(size uiIndex);
	int32				FindSymbol(const char* szName);

	SAsm65Relocation*	AddRelocation(void);
	size				NumRelocations(void);
	SAsm65Relocation*	GetRelocation(size uiIndex);

	void				Dump(CChars* psz);

	bool				Write(CAbstractFile* pcFile);
	bool				Read(CAbstractFile* pcFile);

protected:
	bool				WriteChunks(CChunkFile* pcChunkFile);
	bool				ReadChunks(CChunkFile* pcChunkFile);
	bool				ReadHeader(CChunkFile* pcChunkFile);
	bool				ReadSections(CChunkFile* pcChunkFile);
	bool				ReadSymbols(CChunkFile* pcChunkFile);
	bool				ReadRelocations(CChunkFile* pcChunkFile);
};


bool		Asm65SectionKind(const char* szName, size uiLength, EAsm65SectionKind* peKind);
const char*	Asm65SectionKindName(EAsm65SectionKind eKind);
bool		Asm65Fits(int64 iValue, uint8 uiBytes, bool bSigned);
bool		Asm65WriteText(CFileWriter* pcWriter, const char* szText);
bool		Asm65ReadText(CFileReader* pcReader, CChars* psz);
bool		Asm65WriteMagic(CFileWriter* pcWriter, const char* szMagic, int32 iVersion);
bool		Asm65ReadMagic(CFileReader* pcReader, const char* szMagic, int32 iVersion);


#endif // __ASM65_OBJECT_H__
