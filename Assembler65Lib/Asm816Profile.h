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
#ifndef __ASM816_PROFILE_H__
#define __ASM816_PROFILE_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/MapStringTemplate.h"
#include "MicroprocessorLib/Fast65C816.h"
#include "Asm65Memory.h"
#include "Asm65LineTable.h"


//What a profiled run measured of a function (by its label): the instructions with a direct page operand run on its
//frame (its own and the runtime routines' it called, on its D), how often it was called, and whether it was ever
//called while it was already running (recursion, directly or through others).  With a line table (CAsm65LineTable):
//the cycles its own code ran for and the fingerprint of the code that was run (its hash and compile time, 0 for
//none).
struct SAsm816FunctionProfile
{
	uint64	ulliDirectPage;
	uint64	ulliCalls;
	bool	bRecursive;
	uint64	ulliCycles;
	uint32	uiHash;
	int64	iTime;
};


//What a profiled run measured of a source line's code in a function (inlined code is the function's it is in): how
//often its code was entered at its start (of its pieces, the most), the instructions and cycles run in it, and the
//cycles of code with no line (runtime routines, assembly) run after it, before another line's (what it called).
struct SAsm816LineProfile
{
	uint64	ulliEntries;
	uint64	ulliInstructions;
	uint64	ulliCycles;
	uint64	ulliRuntimeCycles;
};


//The functions' profiles from runs of linked programs on CFast65C816 (added up over runs), and the cycles they ran
//for, and with the program's line table the profile of each line of each function; read and written as text (Write).
//The W65C816 back end gives a function a page aligned frame (a cycle less each direct page access) only when its
//profile says that is worth it (CIR816Backend::AlignFrame).
class CAsm816Profile
{
protected:
	CMapStringTemplate<SAsm816FunctionProfile>	mcFunctions;
	CMapStringTemplate<SAsm816LineProfile>		mcLines;		//By "label\tline\tfile".
	CMapStringTemplate<uint64>					mcLineEntries;	//By "line\tfile": the entries in all functions (LineEntries), made when first needed.
	bool										mbLineEntries;
	uint64										mulliCycles;

public:
	void					Init(void);
	void					Kill(void);

	size					Run(CAsm65Memory* pcMemory, CFast65C816* pcFast, const char* szMap, size uiMaxSteps, CAsm65LineTable* pcLines = NULL);

	bool					Read(const char* szFilename);
	bool					Write(const char* szFilename);
	bool					WriteReport(const char* szFilename, size uiMost);

	SAsm816FunctionProfile*	Get(const char* szLabel);
	SAsm816LineProfile*		GetLine(const char* szLabel, const char* szFile, uint32 uiLine);
	uint64					LineEntries(const char* szFile, uint32 uiLine);
	uint64					NumCycles(void);

protected:
	SAsm816FunctionProfile*	PutFunction(const char* szLabel);
	SAsm816LineProfile*		PutLine(const char* szLabel, const char* szFile, uint32 uiLine);
	void					ForgetLines(const char* szLabel);
	void					Fingerprints(CAsm65LineTable* pcLines);
	void					LineKey(CChars* pszKey, const char* szLabel, const char* szFile, uint32 uiLine);
};


#endif // __ASM816_PROFILE_H__

