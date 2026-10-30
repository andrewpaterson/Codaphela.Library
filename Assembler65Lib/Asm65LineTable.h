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
#ifndef __ASM65_LINE_TABLE_H__
#define __ASM65_LINE_TABLE_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/MapStringTemplate.h"


//Where the code for a line of a source file starts.  A line of 0 is code for no line (past the end of a section that
//has lines).
struct SAsm65LineEntry
{
	uint32	uiAddress;
	uint32	uiFile;
	uint32	uiLine;
	uint32	uiOrder;		//When it was added (of lines at one address the last added is the one).
};


//A linked function's fingerprint (CAsm65Object's .fingerprint).
struct SAsm65LineFingerprint
{
	CChars	szLabel;
	uint32	uiHash;
	int64	iTime;
};


typedef CArrayTemplate<SAsm65LineEntry>			CArrayAsm65LineEntries;
typedef CArrayTemplate<SAsm65LineFingerprint>	CArrayAsm65LineFingerprints;


//The linker's line table: from an address in the program to the source line its code is for (.line), and the
//linked functions' fingerprints.  Lines are in address order once Sort is called.
class CAsm65LineTable
{
protected:
	CArrayChars					maszFiles;
	CMapStringTemplate<uint32>	mcFileIndices;
	CArrayAsm65LineEntries		masLines;
	CArrayAsm65LineFingerprints	masFingerprints;

public:
	void					Init(void);
	void					Kill(void);
	void					Clear(void);

	void					AddLine(uint32 uiAddress, const char* szFile, uint32 uiLine);
	void					AddFingerprint(const char* szLabel, uint32 uiHash, int64 iTime);
	void					Sort(void);

	size					NumLines(void);
	SAsm65LineEntry*		GetLine(size uiIndex);
	SAsm65LineEntry*		FindLine(uint32 uiAddress);
	size					NumFiles(void);
	char*					GetFile(size uiIndex);
	size					NumFingerprints(void);
	SAsm65LineFingerprint*	GetFingerprint(size uiIndex);
	SAsm65LineFingerprint*	FindFingerprint(const char* szLabel);
};


#endif // __ASM65_LINE_TABLE_H__

