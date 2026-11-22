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
#ifndef __I_R_PASSES_H__
#define __I_R_PASSES_H__
#include "IRModule.h"
#include "IRDominators.h"
#include "IRLoops.h"
#include "IRRanges.h"
#include "IRAddresses.h"


class CIRSSAUpdater;


//A slot (or argaddr) being promoted: its accesses' type and its value where nothing has been stored.
struct SIRPromotion
{
	SIRReference	sAddress;
	SIRType			sType;
	SIRReference	sInitial;
};


typedef CArrayTemplate<SIRPromotion>	CArrayIRPromotions;


//A block on the dominator tree walk and how far through its children it is.
struct SIRRenameFrame
{
	CIRBlock*	pcBlock;
	size		uiNextChild;
	size		uiPushed;		//Values pushed in this block, to pop when it is left.
};


typedef CArrayTemplate<SIRRenameFrame>	CArrayIRRenameFrames;


//Where the values a block leaves known start in RemoveRedundantLoads' list, and how many; -1 for a block not walked.
struct SIRBlockEnd
{
	int32	iStart;
	int32	iCount;
};




//A load or store of a slot at a constant offset, and the slot of its own it gets when the slot is split.
struct SIRSlotAccess
{
	SIRReference	sSlot;
	int64			lliOffset;
	SIRType			sType;
	SIRReference	sNew;
};


//A value an unswitched loop defines: its block (an index in the loop's blocks) and whether that block and its copy
//can be reached after the branch is gone.
struct SIRUnswitchedValue
{
	SIRReference	sValue;
	size			uiBlock;
	bool			bReached;
	bool			bCopyReached;
};


//An address SplitLoopAddresses splits: where it is and the add's operands (the loop's invariant one and the other).
struct SIRSplitAddress
{
	CIRBlock*		pcBlock;
	size			uiInstruction;
	SIRReference	sInvariant;
	SIRReference	sVariant;
};


//A multiple NarrowMultiplies makes narrow: where it is, its narrow operand and what that is multiplied by.
struct SIRNarrowMultiply
{
	CIRBlock*		pcBlock;
	size			uiInstruction;
	SIRReference	sNarrow;
	uint16			uiMultiple;		//What it is multiplied by.
};


//An unsigned divide or remainder by a constant that BoundDivisions makes cheaper: the dividend is below the divisor
//(bOnce false) or below twice it.
struct SIRBoundDivision
{
	CIRBlock*		pcBlock;
	size			uiInstruction;
	bool			bOnce;
};


//A loop's counter (CIRPasses::MatchCounter): a phi of its head (sPhi) with two arguments, sInit from a block
//outside the loop (argument uiOutside) and sNext = sPhi + lliStep from the other.
struct SIRLoopCounter
{
	SIRReference	sPhi;
	size			uiOutside;
	SIRReference	sInit;
	SIRReference	sNext;
	int64			lliStep;
};


//The divides whose remainder a urem or srem of the same operands straight after can take (SetDivideRemainders).
#define IR_DIVIDE_REMAINDERS_8		0x01
#define IR_DIVIDE_REMAINDERS_16		0x02
#define IR_DIVIDE_REMAINDERS_32		0x04
#define IR_DIVIDE_REMAINDERS_64		0x08
#define IR_DIVIDE_REMAINDERS_SIGNED	0x10	//sdiv and srem too, not only udiv and urem.


#define IR_COUNTER_SUB		0x01	//MatchCounter: the step may be a sub of a constant (lliStep its negative).
#define IR_COUNTER_INSIDE	0x02	//MatchCounter: sNext's argument must come from a block in the loop.
#define IR_COUNTER_POINTER	0x04	//MatchCounter: the step may be a ptradd of a constant (a pointer stepped through memory).


//A loop's counter for ReplaceLoopIdioms: i = phi [init, i + 1], round while i + 1 < n (signed, unsigned or not equal).
struct SIRIdiomCounter
{
	SIRReference	sCounter;
	SIRReference	sNext;
	SIRReference	sInit;
	SIRReference	sBound;
	uint16			uiOpcode;		//IRO_SLt, IRO_ULt or IRO_Ne, true going round.
};


//An address a loop steps through for ReplaceLoopIdioms: a pointer phi stepped by its own ptradd, or base + i * scale
//(the index i, i extended, i << s or i * c).
struct SIRIdiomAddress
{
	SIRReference	sPhiStart;		//The pointer phi's value on the way in, IRNone for base + i * scale.
	SIRReference	sBase;
	int64			lliScale;
	uint16			uiExtension;	//IRO_SExt or IRO_ZExt of the counter in the index, IRO_None for none.
	bool			bNext;			//The index is the counter's next value (i + 1).
};


typedef CArrayTemplatePtr<SIRPhiArgument>	CArrayIRExitArguments;


class CIRInlineProfile;
class CIRDivisionCosts;
class CIRBlockPasses;
class CIRCounterPasses;
class CIRFoldingPasses;
class CIRMemoryPasses;
class CIRRangePasses;


//What the optimiser's passes share while a function is optimised: the function, the target's settings, the
//analyses (the dominators, each temporary's definition, the loops; and what is asked of them: the ranges of
//values, what addresses are of), the replacements a pass is making, and the groups of passes that have a
//function another group calls.
class CIRPipeline;
class CIRPassContext
{
public:
	CIRModule*				mpcModule;
	CIRFunction*			mpcFunction;
	bool					mbIndexedAddresses;	//Loads and stores add a pointer and an index for nothing (the 65C64's [R1 + R2]).
	bool					mbRestrict;			//The module has restrict symbols or the function restrict parameters (see PointerBase).
	CIRPipeline*			mpcPipeline;		//The passes the optimiser runs: a pass asks it whether another is one of them (Has).
	uint32					muiDivideRemainders;	//The IR_DIVIDE_REMAINDERS_ divides that leave their remainder for a urem or srem right after them.
	int						miUnswitched;		//Loops UnswitchLoops copied in this Optimise.
	SIRType					msLibraryInt;		//memset's int (16 or 32 bits); IRTK_None: loops aren't made library calls.
	SIRType					msLibrarySize;		//memset's size_t (16 or 32 bits): a 16 bit one has 32 suffixed functions for longer counts.
	CIRInlineProfile*		mpcProfile;			//How often lines ran (SetProfile), NULL for none.
	CIRDivisionCosts*		mpcDivisionCosts;	//What a divide by a constant and its reciprocal cost the target (SetDivisionCosts); NULL: not said.
	CChars*					mpszWarnings;		//Comparisons found always true or false (SetWarnings); NULL: not looked for.
	CIRDominators			mcDominators;
	CIRDefinitions			mcDefinitions;		//Each temporary's definition (FindDefinitions).
	CIRLoops				mcLoops;			//The loops (ComputeLoops).
	CIRRanges				mcRanges;			//How big a value can be: asked, not kept (it reads the definitions and the dominators).
	CIRAddresses			mcAddresses;		//What an address is of: asked, not kept.
	CArrayIRReferences		masArgAddresses;	//Per parameter: its first argaddr, IRNone for none.

	CIRBlockPasses*			mpcBlockPasses;
	CIRCounterPasses*		mpcCounterPasses;
	CIRFoldingPasses*		mpcFoldingPasses;
	CIRMemoryPasses*		mpcMemoryPasses;
	CIRRangePasses*			mpcRangePasses;

	void			Init(CIRModule* pcModule);
	void			Kill(void);
	void			Changed(void);

protected:
	friend class CIRPasses;

	//What every group keeps through CIRPasses, and none reaches for.
	CArrayIRReferences		masReplacements;	//Per temporary: what to use instead, IRNone to keep it.
	bool					mbDominatorsFresh;	//Nothing has changed the function since the dominators were worked out...
	bool					mbDefinitionsFresh;	//...or the definitions...
	bool					mbLoopsFresh;		//...or the loops (and the dominators and definitions with them: ComputeLoops).
};


//A group of passes: what every group has.  Its passes are its functions that take nothing and return whether they
//changed the function; they find what they share in the context, and these are the tools they all use: the
//replacements of one value by another, the walks of a value's uses, the definitions and the loops, and what may write
//memory.
//
//An analysis is worked out again by FindDefinitions, ComputeDominators and ComputeLoops, which a pass calls after it
//has changed the function.  NeedDefinitions and NeedLoops are for the start of a pass, before it has changed
//anything: they leave the analysis as it is when no pass has changed the function since it was worked out (a pass
//says so by what it returns; the optimiser tells the context, CIRPassContext::Changed).
class CIRPasses
{
protected:
	CIRPassContext*		mpcContext;
	CIRModule*			mpcModule;
	CIRFunction*		mpcFunction;

public:
	virtual void	Init(CIRPassContext* pcContext);
	virtual void	Kill(void);
	void			Begin(CIRFunction* pcFunction);
	bool			WritesMemory(void);
	void			Renumber(void);
	void			ComputeDominators(void);
	void			NeedDefinitions(void);
	void			NeedLoops(void);

	bool			IsHot(SIRInstruction* psInstruction, uint64 ulliSaving);
	void			ClearReplacements(void);
	void			Replace(SIRReference sOld, SIRReference sNew);
	SIRReference	Replacement(SIRReference sReference);
	void			ApplyReplacements(void);
	bool			MayWriteMemory(SIRInstruction* psInstruction);
	void			FindDefinitions(void);
	void			ComputeLoops(void);
	void			MoveInstruction(CIRBlock* pcBlock, size uiFrom, size uiTo);
	SIRReference	InsertInstruction(CIRBlock* pcBlock, size uiIndex, uint16 uiOpcode, SIRType sType, SIRReference sFirst, SIRReference sSecond);
	void			InsertInstruction(CIRBlock* pcBlock, size uiIndex, uint16 uiOpcode, SIRType sType, SIRReference sResult, SIRReference sFirst, SIRReference sSecond, uint32 uiSource);
	SIRReference	AddInstruction(CIRBlock* pcBlock, uint16 uiOpcode, SIRType sType, SIRReference sFirst, SIRReference sSecond, uint32 uiSource);
	bool			EdgeCondition(CIRBlock* pcFrom, CIRBlock* pcTo, SIREdgeCondition* psCondition);
	bool			LoopWays(size uiLoop, CIRBlock* pcBlock, CIRBlock** ppcInside, CIRBlock** ppcOutside);
	bool			MatchCounter(size uiLoop, SIRPhi* psPhi, uint8 uiFlags, SIRLoopCounter* psCounter);
	bool			SimpleLoop(size uiLoop, CIRBlock** ppcHead, CIRBlock** ppcEntering, CIRBlock** ppcLatch);
	SIRReference	InsertDivisorMask(CIRBlock* pcBlock, size uiIndex, SIRType sType, SIRReference sTest, SIRReference sDivisor);
	void			AddCounterPhi(CIRBlock* pcHead, SIRType sType, SIRReference sPhi, CIRBlock* pcEntering, SIRReference sStart, CIRBlock* pcLatch, SIRReference sStepped);
};


#endif // __I_R_PASSES_H__

