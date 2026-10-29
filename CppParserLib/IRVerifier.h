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
#ifndef __I_R_VERIFIER_H__
#define __I_R_VERIFIER_H__
#include "CPPErrors.h"
#include "IRModule.h"
#include "IRDominators.h"


//Where a temporary is defined: a parameter, a phi or an instruction of a block.
struct SIRVerifierDefinition
{
	CIRBlock*	pcBlock;		//NULL for a parameter.
	int32		iPosition;		//-1 for a phi, otherwise the instruction's index.
	bool		bDefined;
};


typedef CArrayTemplate<SIRVerifierDefinition>	CArrayIRVerifierDefinitions;


//Checks a module's functions are well formed (IR.txt Text form), using the opcodes' table entries.  Each problem is
//an error "@function block: message [detail]"; checking a function goes on after a problem except where later
//checks would only repeat it.
class CIRVerifier
{
protected:
	CIRModule*				mpcModule;
	CCPPErrors*				mpcErrors;
	CIRFunction*			mpcFunction;
	CIRBlock*				mpcBlock;
	CArrayIRVerifierDefinitions	masDefinitions;
	CIRDominators			mcDominators;
	CArrayTemplate<uint32>	mauiCounts;			//Per block index: edges into it.
	CArrayTemplate<uint32>	mauiStamps;			//Per block index: the last check that marked it.
	uint32					muiStamp;
	size					muiStartErrors;

public:
	void	Init(CIRModule* pcModule, CCPPErrors* pcErrors);
	void	Kill(void);

	bool	Verify(void);
	bool	VerifyFunction(CIRFunction* pcFunction);

protected:
	void	Error(const char* szMessage, const char* szDetail = NULL);
	char*	NameOf(SIRReference sReference);
	bool	CheckReference(SIRReference sReference);
	bool	CheckType(SIRType sType, bool bAllowVoid);
	bool	IsFunctionBlock(CIRBlock* pcBlock);
	bool	JumpsTo(CIRBlock* pcFrom, CIRBlock* pcTo);
	uint32	NewStamp(void);
	bool	CheckBlocks(void);
	bool	CheckDefinitions(void);
	void	Define(SIRReference sResult, CIRBlock* pcBlock, int32 iPosition);
	void	CheckPhis(CIRBlock* pcBlock);
	void	CheckInstruction(CIRBlock* pcBlock, size uiIndex);
	void	CheckConversion(SIRInstruction* psInstruction, SIROpcode* psOpcode);
	void	CheckResult(SIRInstruction* psInstruction, SIROpcode* psOpcode);
	void	CheckOperand(SIRReference sOperand, uint8 uiKind, SIRType sType, CIRBlock* pcBlock, int32 iPosition, const char* szWhat);
	void	CheckJump(CIRBlock* pcBlock);
	void	CheckUse(SIRReference sOperand, CIRBlock* pcBlock, int32 iPosition);
};


#endif // __I_R_VERIFIER_H__
