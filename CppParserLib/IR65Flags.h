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
#ifndef __I_R_65_FLAGS_H__
#define __I_R_65_FLAGS_H__
#include "IR65Code.h"


//What the flags are known to hold.
enum EIR65FlagKind
{
	IR65FK_Unknown,		//Not yet worked out (a line not reached yet).
	IR65FK_None,		//Nothing known.
	IR65FK_Value,		//n and z are iRegister's value at uiBits (and iCopy's, which a cpy copied it from).
	IR65FK_Compare,		//All four are what the cmp at line iCompare left, its operands unchanged (and n and z are
						//iRegister's value at uiBits when it was cmp R, 0).
};


struct SIR65FlagState
{
	uint8	uiKind;
	int8	iRegister;
	int8	iCopy;
	uint8	uiBits;
	int32	iCompare;
};


typedef CArrayTemplate<SIR65FlagState>	CArrayIR65FlagStates;


//Removes compares whose flags are already set, from a function's 65C64 code (IR.txt 65C64 back end, Flags): the
//flags are followed through the code (and across labels, where the ways in agree) as either n and z of a register's
//value (cpy, and, or, xor, add, sub and neg set them from what they write) or everything a cmp left while its
//operands are unchanged.  A cmp the flags already hold goes, and so does a cmp R, 0 after n and z were set from R
//when the branches after it only look at n and z.
class CIR65Flags
{
protected:
	CIR65Code				mcCode;
	CArrayIR65FlagStates	masStates;		//Per line: what the flags hold as it starts.
	CArrayTemplate<uint8>	mauiDemand;		//Per line: the flags read after it before they are set (IR65_FLAG_ bits).

public:
	void	Init(void);
	void	Kill(void);

	void	Optimise(CChars* pszCode, size uiStart);

protected:
	void	ComputeStates(void);
	void	Transfer(size uiLine, SIR65FlagState* psState);
	bool	RemoveCompares(void);
	bool	IsCompareWithZero(size uiLine);
	bool	SameCompare(size uiLine1, size uiLine2);
};


#endif // __I_R_65_FLAGS_H__
