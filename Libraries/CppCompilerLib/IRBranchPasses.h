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
#ifndef __I_R_BRANCH_PASSES_H__
#define __I_R_BRANCH_PASSES_H__
#include "BaseLib/PrimitiveTypes.h"


#define IR_BRANCH_HOPS		8			//The most jumps IRThreadJumps follows a branch through (a loop of bras never ends).
#define IR_NO_LINE			((size)-1)


//How a line of a function's code goes on, as the branch passes see it.
enum EIRCodeFlow
{
	IRCF_None,			//Not run, and never taken out: blank, or taken out already.
	IRCF_Label,
	IRCF_Instruction,	//Runs and goes on to the next line.
	IRCF_Branch,		//Goes to its label or on to the next line (bras z, beq).
	IRCF_Jump,			//Goes to its label (bra).
	IRCF_End,			//Doesn't go on to the next line (ret, rtl, a jump out of the function; brl still has its label).
};


//A function's code as a back end's text passes keep it (CIR65Code, the W65C816's IR816BranchCode), for the passes
//over its branches both back ends share: IRThreadJumps, IRRemoveBranchesToNext, IRInvertBranches and
//IRRemoveUnreached.  Lines keep their numbers while a pass runs: Remove makes a line IRCF_None.
class CIRBranchCode
{
public:
	virtual size		NumLines(void) = 0;
	virtual EIRCodeFlow	Flow(size uiLine) = 0;
	virtual size		Target(size uiLine) = 0;
	virtual bool		IsEntry(size uiLine) = 0;
	virtual bool		Retarget(size uiLine, size uiLabel) = 0;
	virtual bool		Invert(size uiLine, size uiLabel) = 0;
	virtual void		Remove(size uiLine) = 0;
};


size	IRNextInstruction(CIRBranchCode* pcCode, size uiLine);
bool	IRThreadJumps(CIRBranchCode* pcCode);
bool	IRRemoveBranchesToNext(CIRBranchCode* pcCode);
bool	IRInvertBranches(CIRBranchCode* pcCode);
bool	IRRemoveUnreached(CIRBranchCode* pcCode);


#endif // __I_R_BRANCH_PASSES_H__
