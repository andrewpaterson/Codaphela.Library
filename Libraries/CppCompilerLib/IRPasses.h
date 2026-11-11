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


//What the pointers at a root are based on (CIRMemoryPasses::PointerBase): a pointer loaded from a restrict symbol
//(uiIndex the symbol), a parameter (uiIndex the parameter, bRestrict if it is restrict) or something unknown.
enum EIRPointerBase
{
	IRPB_Unknown,
	IRPB_RestrictSymbol,
	IRPB_Parameter,
};


struct SIRPointerBase
{
	EIRPointerBase	eKind;
	uint32			uiIndex;
	bool			bRestrict;
};


//A value known to be in memory: what a load read or a store wrote at sRoot + lliOffset.
struct SIRAvailable
{
	SIRReference	sRoot;
	int64			lliOffset;
	SIRType			sType;
	SIRReference	sValue;
};


typedef CArrayTemplate<SIRAvailable>	CArrayIRAvailables;


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


//A mul NarrowMultiplies makes narrow: where it is and its narrow operand.
struct SIRNarrowMultiply
{
	CIRBlock*		pcBlock;
	size			uiInstruction;
	SIRReference	sNarrow;
};


//An unsigned divide or remainder by a constant that BoundDivisions makes cheaper: the dividend is below the divisor
//(bOnce false) or below twice it.
struct SIRBoundDivision
{
	CIRBlock*		pcBlock;
	size			uiInstruction;
	bool			bOnce;
	bool			bTable;		//ConstantDivisions: a hot one of a byte's range, made an i8 one for the target's table.
	bool			bSplit;		//ConstantDivisions: a wide divisor 2^k * m, divided by 2^k then m.
	int64			lliMin;		//ConstantDivisions: the dividend's range.
	int64			lliMax;
};


//What a use of a value is (SIRUse): a phi's argument, an instruction's operand or a block's jump's argument.
enum EIRUseKind
{
	IRUK_Phi,
	IRUK_Instruction,
	IRUK_Jump,
};


//Where a walk of a value's uses is (CIRPasses::FirstUse, FirstTemporaryUse and NextUse): block by block, each
//block's phis' arguments, then its instructions' operands, then its jump.  Each step fetches the block, phi and
//instruction again by their indices, so a walk may add phis and instructions (at the ends of the blocks) as it
//goes; SetUse writes a use the same way.
struct SIRUse
{
	SIRReference		sValue;			//The value whose uses are walked...
	bool				bTemporaries;	//...or, when true, every temporary's.
	CArrayIRBlocks*		papcBlocks;		//The blocks walked, NULL for all the function's.
	size				uiBlock;		//The use: its block (in papcBlocks or the function)...
	EIRUseKind			eKind;
	size				uiIndex;		//...the phi or instruction...
	size				uiArgument;		//...and the phi's argument or the instruction's operand.
	CIRBlock*			pcBlock;		//Fetched by NextUse: the use's block...
	SIRPhi*				psPhi;			//...a phi argument's phi and argument...
	SIRPhiArgument*		psArgument;
	SIRInstruction*		psInstruction;	//...an operand's instruction...
	SIRReference*		psSlot;			//...and the use itself (until the blocks change).
};


//A loop's counter (CIRCounterPasses::MatchCounter): a phi of its head with two arguments, sInit from a block outside
//the loop (argument uiOutside) and sNext = phi + lliStep from the other.
struct SIRLoopCounter
{
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


#define IR_SIGNED_RANGE_DEPTH	8	//How many definitions back SignedRange looks (through phis, adds and extensions); the other limits are in IROptimiserInternal.h.


typedef CArrayTemplatePtr<SIRPhiArgument>	CArrayIRExitArguments;


class CIRInlineProfile;
class CIRAddressPasses;
class CIRBlockPasses;
class CIRCounterPasses;
class CIRDivisionPasses;
class CIRFoldingPasses;
class CIRIdiomPasses;
class CIRLoopPasses;
class CIRMemoryPasses;
class CIRPromotionPasses;
class CIRRangePasses;


//What the optimiser's passes share while a function is optimised: the function, the target's settings, the
//analyses (the dominators, each temporary's definition, the loops), the replacements a pass is making, the uses
//counted, and the groups of passes themselves, for the few functions one group has that another calls.
class CIRPassContext
{
public:
	CIRModule*				mpcModule;
	CIRFunction*			mpcFunction;
	bool					mbIndexedAddresses;	//Loads and stores add a pointer and an index for nothing (the 65C64's [R1 + R2]).
	bool					mbRestrict;			//The module has restrict symbols or the function restrict parameters (see PointerBase).
	bool					mbConstantDivisions;	//Divides by constants are multiplies (SetConstantDivisions).
	uint32					muiDivideRemainders;	//The IR_DIVIDE_REMAINDERS_ divides that leave their remainder for a urem or srem right after them.
	bool					mbBranchRanges;		//SignedRange narrows by the branches to a value's block (predecessors found).
	int						miUnswitched;		//Loops UnswitchLoops copied in this Optimise.
	SIRType					msLibraryInt;		//memset's int (16 or 32 bits); IRTK_None: loops aren't made library calls.
	CIRInlineProfile*		mpcProfile;			//How often lines ran (SetProfile), NULL for none.
	CChars*					mpszWarnings;		//Comparisons found always true or false (SetWarnings); NULL: not looked for.
	CIRDominators			mcDominators;
	CIRDefinitions			mcDefinitions;		//Each temporary's definition (FindDefinitions).
	CIRLoops				mcLoops;			//The loops (ComputeLoops).
	CArrayIRReferences		masReplacements;	//Per temporary: what to use instead, IRNone to keep it.
	CArrayTemplate<uint32>	mauiUses;			//Per temporary.
	CArrayIRReferences		masArgAddresses;	//Per parameter: its first argaddr, IRNone for none.

	CIRAddressPasses*		mpcAddressPasses;
	CIRBlockPasses*			mpcBlockPasses;
	CIRCounterPasses*		mpcCounterPasses;
	CIRDivisionPasses*		mpcDivisionPasses;
	CIRFoldingPasses*		mpcFoldingPasses;
	CIRIdiomPasses*			mpcIdiomPasses;
	CIRLoopPasses*			mpcLoopPasses;
	CIRMemoryPasses*		mpcMemoryPasses;
	CIRPromotionPasses*		mpcPromotionPasses;
	CIRRangePasses*			mpcRangePasses;

	void			Init(CIRModule* pcModule);
	void			Kill(void);
};


//A group of passes: what every group has.  Its passes are its functions that take nothing and return whether they
//changed the function; they find what they share in the context, and these are the tools they all use: the
//replacements of one value by another, the walks of a value's uses, the definitions and the loops, and what may write
//memory.
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

	bool			IsHot(SIRInstruction* psInstruction, uint64 ulliSaving);
	void			ClearReplacements(void);
	void			Replace(SIRReference sOld, SIRReference sNew);
	SIRReference	Replacement(SIRReference sReference);
	void			ApplyReplacements(void);
	bool			FirstUse(SIRUse* psUse, SIRReference sValue, CArrayIRBlocks* papcBlocks = NULL, bool bTemporaries = false);
	bool			FirstTemporaryUse(SIRUse* psUse, CArrayIRBlocks* papcBlocks = NULL);
	bool			NextUse(SIRUse* psUse);
	void			SetUse(SIRUse* psUse, SIRReference sNew);
	bool			MayWriteMemory(SIRInstruction* psInstruction);
	bool			IsOwnFrame(SIRReference sAddress);
	void			FindDefinitions(void);
	void			ComputeLoops(void);
	void			MoveInstruction(CIRBlock* pcBlock, size uiFrom, size uiTo);
	SIRReference	InsertInstruction(CIRBlock* pcBlock, size uiIndex, uint16 uiOpcode, SIRType sType, SIRReference sFirst, SIRReference sSecond);
};


#endif // __I_R_PASSES_H__

