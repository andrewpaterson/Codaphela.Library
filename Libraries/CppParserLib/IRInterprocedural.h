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
#ifndef __I_R_INTERPROCEDURAL_H__
#define __I_R_INTERPROCEDURAL_H__
#include "BaseLib/ArrayTemplate.h"
#include "IRModule.h"


class CIROptimiser;


#define IR_SPECIALISE_ROUNDS		3		//Interprocedural constant propagation (SpecialiseConstantArguments): rounds,
#define IR_SPECIALISE_VALUES		3		//the most values a parameter is specialised for (a copy each),
#define IR_SPECIALISE_INSTRUCTIONS	4000	//and the biggest function copied.
#define IR_PARAMETER_BITS			32		//The parameters a pass keeps a bit for: those after them are left as they are.


//A call of a function: the unit function it is in, its block and its index there.
struct SIRCallSite
{
	size		uiFunction;
	CIRBlock*	pcBlock;
	size		uiCall;
};


typedef CArrayTemplate<SIRCallSite>	CArrayIRCallSites;


//What CIRInterprocedural asks of whoever keeps the unit's functions.  A function has the same index until the end of
//the passes, NULL once it has gone; a new one is added at the end.
class CIRUnitFunctions
{
public:
	virtual size			NumUnitFunctions(void) =0;
	virtual CIRFunction*	GetUnitFunction(size uiFunction) =0;
	virtual void			InitOptimiser(CIROptimiser* pcOptimiser) =0;
	virtual void			AddSpecialised(size uiFunction, CIRFunction* pcCopy) =0;	//pcCopy is a new function, a copy of uiFunction: it gets the next index.
	virtual bool			RemoveSpecialised(size uiFunction) =0;						//Nothing in the IR calls it now: true if it has gone (nothing else names it).
};


//The passes over a whole unit that work on what its functions do with each other (IR.txt Passes): specialising
//functions for constant arguments, removing arguments never read, and finding the functions that write no memory.
//They run before inlining.  What they changed is asked for afterwards, by the function's index: IsChanged for code
//that must be written again, CallsNoWrites for a function worth optimising again.
class CIRInterprocedural
{
protected:
	CIRModule*				mpcModule;
	CIRUnitFunctions*		mpcUnit;
	CArrayTemplate<bool>	mabChanged;			//Per unit function.
	CArrayTemplate<bool>	mabCallsNoWrites;	//Per unit function, once MarkNoWrites has run.

public:
	void			Init(CIRModule* pcModule, CIRUnitFunctions* pcUnit);
	void			Kill(void);

	void			SpecialiseConstantArguments(void);
	void			RemoveDeadArguments(void);
	void			MarkNoWrites(void);
	void			Optimise(CIRFunction* pcFunction);

	bool			IsChanged(size uiFunction);
	bool			CallsNoWrites(size uiFunction);

	static bool		IsRecursive(CIRModule* pcModule, CIRFunction* pcFunction);
	static uint32	DeadParameters(CIRFunction* pcFunction);

protected:
	uint32			StrongParameters(CIRFunction* pcFunction, CArrayTemplate<uint32>* pauiStrong);
	bool			IsStrong(CIRFunction* pcFunction, CArrayTemplate<uint8>* paucDerived, CArrayTemplate<uint32>* pauiStrong);
	int				FunctionIndex(uint32 uiSymbol);
	bool			OnlyTheUnitCalls(CIRFunction* pcFunction);
	bool			FindCalls(uint32 uiSymbol, CArrayIRCallSites* pasSites);
	bool			SpecialiseCallee(size uiCallee, uint32 uiStrong);
	bool			ConstantArguments(CArrayIRCallSites* pasSites, size uiParameter, CArrayTemplate<uint64>* paulliValues);
	void			CopyForValues(size uiCallee, size uiParameter, CArrayIRCallSites* pasSites, CArrayTemplate<uint64>* paulliValues);
	uint64			SiteConstant(SIRCallSite* psSite, size uiParameter);
	void			RemoveCalleeArguments(CIRFunction* pcCallee, uint32 uiDead, CArrayIRCallSites* pasSites, CArrayTemplate<bool>* pabOptimise);
};


#endif // __I_R_INTERPROCEDURAL_H__
