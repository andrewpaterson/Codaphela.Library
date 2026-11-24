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
#ifndef __I_R_BLOCK_COPIER_H__
#define __I_R_BLOCK_COPIER_H__
#include "IRFunction.h"


//Copies a block's phis, instructions and jump into another block, each value and each block they name taken through
//what it is in the copy.  What is copying says what those are (CopyValue, CopyOf), and may say what an instruction's
//operands, a return and a throw become: an inlined function's returns go on to the code after the call.
class CIRBlockCopier
{
public:
	void					CopyBlock(CIRBlock* pcFrom, CIRBlock* pcCopy);
	void					CopyPhis(CIRBlock* pcFrom, CIRBlock* pcCopy);
	void					CopyInstructions(CIRBlock* pcFrom, CIRBlock* pcCopy);
	void					CopyJump(CIRBlock* pcFrom, CIRBlock* pcCopy);

protected:
	virtual SIRReference	CopyValue(SIRReference sValue) = 0;
	virtual CIRBlock*		CopyOf(CIRBlock* pcBlock) = 0;
	virtual void			CopyOperands(SIRInstruction* psFrom, SIRInstruction* psCopy);
	virtual void			CopyReturn(CIRBlock* pcFrom, CIRBlock* pcCopy);
	virtual void			CopyThrow(CIRBlock* pcFrom, CIRBlock* pcCopy);
	virtual bool			CopiesDebugValues(void);
	virtual uint32			CopyDebugSlot(uint32 uiSlot);
};


//A copier of some of a function's blocks into the same function.  A temporary is what the map gives for it, and a
//block is its copy when it is one of the blocks copied; everything else is itself in the copy.
class CIRMappedCopier : public CIRBlockCopier
{
public:
	CArrayIRReferences	masMap;			//Per temporary: what it is in the copy, IRNone() for itself.
	CArrayIRBlocks		mapcBlocks;		//The blocks copied...
	CArrayIRBlocks		mapcCopies;		//...and the copy of each.

	void			Init(void);
	void			Kill(void);
	void			Begin(CIRFunction* pcFunction);
	void			AddBlock(CIRBlock* pcBlock, CIRBlock* pcCopy);
	void			SetValue(SIRReference sValue, SIRReference sCopy);
	SIRReference	Value(SIRReference sValue);
	CIRBlock*		Block(CIRBlock* pcBlock);

protected:
	SIRReference	CopyValue(SIRReference sValue) override;
	CIRBlock*		CopyOf(CIRBlock* pcBlock) override;
};


#endif // __I_R_BLOCK_COPIER_H__
