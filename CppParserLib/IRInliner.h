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
#ifndef __I_R_INLINER_H__
#define __I_R_INLINER_H__
#include "BaseLib/ArrayChars.h"
#include "IRModule.h"
#include "IRImports.h"


//A return of the function being inlined: the copy of its block and the value it returns.
struct SIRInlineReturn
{
	CIRBlock*		pcBlock;
	SIRReference	sValue;
};


typedef CArrayTemplate<SIRInlineReturn>	CArrayIRInlineReturns;


#define IR_INLINE_CONSTANT_BONUS	8		//More instructions inlined for each constant argument the callee compares.


//Inlining (IR.txt Passes): a call of a small function of the same module is replaced by a copy of the function's
//blocks.  The block the call is in is split after it; the arguments become the copy's parameters, its returns
//jump to the rest of the block, and the call's result is a phi there of the values returned.  A function is
//small if it has at most muiMaxInstructions instructions (muiMaxLoopInstructions for a call in a loop, and
//IR_INLINE_CONSTANT_BONUS more for each constant argument whose parameter it compares or branches on); variadic
//functions, functions that take their parameters' addresses (argaddr) and calls of the function itself aren't
//inlined.  The copies' own calls are inlined too, up to muiMaxInlines in one caller.  The caller should be optimised
//again afterwards.  A callee with no body in the module may be another unit's, from SetImports' CIRImports.
class CIRInliner
{
protected:
	CIRModule*				mpcModule;
	CIRFunction*			mpcCaller;
	size					muiMaxInstructions;
	size					muiMaxLoopInstructions;		//For a call in a loop.
	size					muiMaxInlines;
	CArrayIRReferences		masMap;						//Per callee temporary: what it is in the caller, IRNone until it is made.
	CArrayIRBlocks			mapcCopies;					//Per callee block index: its copy.
	CArrayIRInlineReturns	masReturns;
	CArrayTemplate<bool>	mabInLoop;					//Per caller block index.
	CIRImports*				mpcImports;					//Other units' functions, NULL for none.
	CArrayChars*			mpaszMissing;				//Named callees with no body here or in mpcImports, NULL to not keep.

public:
	void			Init(CIRModule* pcModule, size uiMaxInstructions, size uiMaxLoopInstructions, size uiMaxInlines);
	void			Kill(void);

	static size		NumInstructions(CIRFunction* pcFunction);
	bool			CanInline(CIRFunction* pcCallee, bool bInLoop, CIRBlock* pcCallBlock = NULL, size uiCall = 0);
	bool			InlineCalls(CIRFunction* pcCaller);
	void			SetImports(CIRImports* pcImports, CArrayChars* paszMissing);

protected:
	void			FindLoops(void);
	CIRFunction*	Callee(SIRReference sSymbol, bool bMissing);
	size			NumComparedConstants(CIRFunction* pcCallee, CIRBlock* pcCallBlock, size uiCall);
	bool			HasLoopCandidate(void);
	SIRReference	Map(SIRReference sValue, CIRFunction* pcCallee);
	bool			InlineCall(CIRBlock* pcBlock, size uiCall, CIRFunction* pcCallee);
	void			CopyBlock(CIRBlock* pcFrom, CIRFunction* pcCallee, CIRBlock* pcRest);
};


#endif // __I_R_INLINER_H__
