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
#include "IRModule.h"


//A return of the function being inlined: the copy of its block and the value it returns.
struct SIRInlineReturn
{
	CIRBlock*		pcBlock;
	SIRReference	sValue;
};


typedef CArrayTemplate<SIRInlineReturn>	CArrayIRInlineReturns;


//Inlining (IR.txt Passes): a call of a small function of the same module is replaced by a copy of the function's
//blocks.  The block the call is in is split after it; the arguments become the copy's parameters, its returns
//jump to the rest of the block, and the call's result is a phi there of the values returned.  A function is
//small if it has at most muiMaxInstructions instructions; variadic functions, functions that take their
//parameters' addresses (argaddr) and calls of the function itself aren't inlined.  The copies' own calls are
//inlined too, up to muiMaxInlines in one caller.  The caller should be optimised again afterwards.
class CIRInliner
{
protected:
	CIRModule*				mpcModule;
	CIRFunction*			mpcCaller;
	size					muiMaxInstructions;
	size					muiMaxInlines;
	CArrayIRReferences		masMap;			//Per callee temporary: what it is in the caller, IRNone until it is made.
	CArrayIRBlocks			mapcCopies;		//Per callee block index: its copy.
	CArrayIRInlineReturns	masReturns;

public:
	void			Init(CIRModule* pcModule, size uiMaxInstructions, size uiMaxInlines);
	void			Kill(void);

	bool			InlineCalls(CIRFunction* pcCaller);
	bool			CanInline(CIRFunction* pcCallee);
	static size		NumInstructions(CIRFunction* pcFunction);

protected:
	bool			InlineCall(CIRBlock* pcBlock, size uiCall, CIRFunction* pcCallee);
	SIRReference	Map(SIRReference sValue, CIRFunction* pcCallee);
	void			CopyBlock(CIRBlock* pcFrom, CIRFunction* pcCallee, CIRBlock* pcRest);
};


#endif // __I_R_INLINER_H__
