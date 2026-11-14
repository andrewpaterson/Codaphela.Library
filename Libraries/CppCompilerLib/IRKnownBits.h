/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __I_R_KNOWN_BITS_H__
#define __I_R_KNOWN_BITS_H__
#include "IRFunction.h"


//How many low bits of each integer temporary may be set, read as unsigned (the bits above are always 0), and how
//many the instruction that defines it works out before its result is truncated to its type: an operation done in
//a 32 bit register (64 for i64) on operands held zero extended.  A back end that keeps narrow values zero extended
//needn't mask a result whose RawBits fit its type.  Phis are worked out to a fixed point, so a value carried round
//a loop is bounded too, where the loop doesn't grow it.
class CIRKnownBits
{
protected:
	CIRFunction*			mpcFunction;
	CArrayTemplate<uint8>	mauiBits;			//Per temporary.
	CArrayTemplate<uint8>	mauiRawBits;		//Per temporary defined by an instruction.

public:
	void	Init(void);
	void	Kill(void);

	void	Compute(CIRFunction* pcFunction);

	int		Bits(SIRReference sValue);
	int		RawBits(SIRReference sTemporary);
	bool	FitsType(SIRReference sTemporary);

protected:
	int		InstructionBits(SIRInstruction* psInstruction);
	int		FullBits(SIRType sType);
};


#endif // __I_R_KNOWN_BITS_H__

