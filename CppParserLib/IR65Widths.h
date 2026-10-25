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
#ifndef __I_R_65_WIDTHS_H__
#define __I_R_65_WIDTHS_H__
#include "IR65Code.h"


//Removes width changes from a function's 65C64 code (IR.txt 65C64 back end, Widths).  The back end writes each
//instruction with A, B, X and Y 32 bits wide before and after it, so a narrow load is "wth X, 8 / cpy X, [m] /
//wthx X, 32" even when only the low byte is ever read.  A wthx whose extended bits are never read becomes a wth,
//then each register's width is only set straight before an instruction that uses the register, and only when it
//isn't already that width.  A loop's label starts with the widths its code first needs (so a loop keeps a narrow
//register narrow) and each jump to it sets them first; other labels start with the widths the code reaching them
//agrees on.  Calls and returns keep the 32 bit convention.
class CIR65Widths
{
protected:
	CIR65Code				mcCode;
	CArrayTemplate<uint8>	mauiDemand;		//Per line: the flags read after it before they are set.
	CArrayIR65Widths		masLive;		//Per line: the bits read from its start before they are written (CIR65Code::Bands).
	CArrayIR65Widths		masEntries;		//Per line: a label's widths as the new code reaches it (0 unknown or undecided).
	CArrayTemplate<uint8>	mauiArrived;	//Per line: 1 for a label the new code has reached (Arrive).
	SIR65Widths				msActual;		//The widths the new code has set.
	CChars*					mpszOut;

public:
	void	Init(void);
	void	Kill(void);

	void	Optimise(CChars* pszCode, size uiStart);

protected:
	void	NarrowExtensions(void);
	void	ComputeLive(void);
	void	LiveOut(size uiLine, SIR65Widths* psOut);
	bool	RemoveExtensions(void);
	uint8	NextNeed(size uiFirst, int iRegister);
	void	Anticipate(size uiLabel, size uiFallThrough);
	void	DecideEntry(size uiLabel, bool bFromActual);
	void	Arrive(size uiLabel);
	void	SetWidths(SIR65Widths* psTarget, size uiNext);
	void	Write(void);
};


#endif // __I_R_65_WIDTHS_H__
