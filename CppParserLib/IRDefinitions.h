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
#ifndef __I_R_DEFINITIONS_H__
#define __I_R_DEFINITIONS_H__
#include "IRFunction.h"


//Where a temporary is defined and how many times it is used.
struct SIRDefinition
{
	CIRBlock*	pcBlock;			//NULL for a parameter, or a temporary nothing defines.
	int32		iInstruction;		//Its index in the block's instructions, -1 for a phi.
	int32		iPhi;				//Its index in the block's phis, -1 for an instruction.
	uint32		uiUses;				//By instructions, phis and jumps.
};


typedef CArrayTemplate<SIRDefinition>	CArrayIRDefinitions;


//Each temporary's definition and number of uses, as Compute finds them.  Instructions are kept as their block and
//index, so they stay right while instructions are only changed in place; Compute again after adding or removing
//any.
class CIRDefinitions
{
protected:
	CIRFunction*			mpcFunction;
	CArrayIRDefinitions		masDefinitions;		//Per temporary.

public:
	void			Init(void);
	void			Kill(void);

	void			Compute(CIRFunction* pcFunction);

	SIRInstruction*	Instruction(SIRReference sValue);
	SIRPhi*			Phi(SIRReference sValue);
	CIRBlock*		Block(SIRReference sValue);
	int32			Index(SIRReference sValue);
	uint32			NumUses(SIRReference sValue);
	bool			IsDefinedBy(SIRReference sValue, uint16 uiOpcode);

protected:
	SIRDefinition*	Get(SIRReference sValue);
	void			Use(SIRReference sValue);
};


#endif // __I_R_DEFINITIONS_H__
