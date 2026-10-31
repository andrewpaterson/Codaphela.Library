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
#ifndef __I_R_OPTIMISER_H__
#define __I_R_OPTIMISER_H__
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


//What the pointers at a root are based on (CIROptimiser::PointerBase): a pointer loaded from a restrict symbol
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


//An unsigned divide or remainder by a constant that BoundDivisions makes cheaper: the dividend is below the divisor
//(bOnce false) or below twice it.
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


#define IR_AVAILABLE_LIMIT	64
#define IR_SIGNED_RANGE_DEPTH	8	//How many definitions back SignedRange looks (through phis, adds and extensions).
#define IR_BASE_VALUES		16		//How many phis and their arguments PointerBase looks through.
#define IR_KNOWN_DEPTH		8		//How many single predecessors back KnownCondition and ImpliedComparison look for a branch.


//The mid level passes (IR.txt Passes), run on a function until nothing changes:
//
//	simplify cfg		removes unreachable blocks, jumps through empty blocks, joins a block to its only
//						successor when it is that block's only predecessor, folds brif on a constant, skips
//						an empty brif a predecessor knows the way of (or a brif its phi arguments decide),
//						and copies a block of only phis and a ret into the blocks that jump to it
//	slot splitting		a slot only loaded and stored at constant offsets that don't overlap (an object's
//						fields) becomes a slot per offset
//	slot promotion		slots and argaddrs only loaded and stored (all at one type, not volatile) become
//						temporaries and phis (as LLVM's mem2reg, with dominance frontiers)
//	trivial phis		a phi whose arguments are all one value (or itself) is that value; a phi taking the same
//						values as an earlier phi of its block is that phi
//	narrowing			trunc of add, sub, mul, and, or, xor or shl by a constant that only it uses, with an
//						extended or constant operand, is the operation done narrow (C's char promotions)
//	narrow counters		a 32 bit loop counter stepped by 1 towards a bound it is compared with, both it and its
//						start known to fit 16 bits, counts in 16 bits (NarrowCounters, for a target whose 32 bit
//						arithmetic takes more instructions)
//	constant divisions	x / c and x % c (c not a power of 2) with x's range known (ConstantDivisions, for a target
//						whose divides are calls) are a multiply by a reciprocal and a shift, checked exact for
//						every x in the range
//	bounded divisions	an unsigned x / c or x % c with x known to be below c (its most worked out from
//						constants, extensions, ands, shifts, adds and the branches into phis) is 0 or x,
//						and below 2c a compare and subtract
//	constant folding	integer instructions (and integer and pointer conversions) of constants; x * 2^n is
//						x << n and unsigned / and % by 2^n are >> and &; a comparison of two extended narrow
//						values compares the narrow values; a comparison a branch on the way in decides is a
//						constant (or eq, ne); an extension undone, or of a phi of constants, goes; a sext of
//						a value whose top bit is 0 is a zext; x + 0, x * 1 and the like are x, and so are
//						(x + y) - y and x + (y - x) (p + (q - p) is q)
//	local cse			an instruction without effects that a block already did is the earlier result
//	redundant loads		a load of a value known to be in memory (loaded or stored before it, with nothing
//						that may change it between) is that value; a load from a constant global or known
//						data (a vtable) is what is there
//	loop rotation		a loop whose head tests and whose latch jumps back tests at the bottom: the head is
//						copied to the preheader and the latch (the SSA updater joins the copies' values); a
//						loop entered from several blocks gets a preheader whose phis join what they pass
//	loop idioms			a loop that only fills (or copies) memory a byte or element at a time is a memset (or a
//						memmove, when a check on the way in finds the copy doesn't overlap the wrong way); one
//						that counts bytes until a 0 is strlen (what leaves it worked out from the length); one
//						that leaves at the first elements that differ is memcmp (as equal or not); one
//						that leaves at the first byte that is c is memchr (what found uses worked out from where
//						it is); one that steps a pointer to c, leaving at a 0 first, is strchr
//	less loop tests		a loop that goes round while a <= b (signed) goes round while a < b + 1 when b + 1 can't
//						wrap and b is invariant (or a - 1 < b): the 65C64 branches on less in one instruction
//	strength reduction	in a loop with a counter i (i + c each time round), an address p + (i << s) or p + i * k
//						with p unchanged by the loop becomes a pointer stepped by c * k
//	invariant code		loads and integer operations in a loop of what the loop doesn't change move to the
//						block that enters it (one put on the way when that block branches)
//	dead code			instructions without effects whose results are not used
//	sunk comparisons	an integer comparison only its block's brif uses moves to the block's end


typedef CArrayTemplatePtr<SIRPhiArgument>	CArrayIRExitArguments;


class CIRInlineProfile;


class CIROptimiser
{
protected:
	CIRModule*				mpcModule;
	CIRFunction*			mpcFunction;
	bool					mbIndexedAddresses;	//Loads and stores add a pointer and an index for nothing (the 65C64's [R1 + R2]).
	bool					mbRestrict;			//The module has restrict symbols or the function restrict parameters (see PointerBase).
	bool					mbNarrowMultiplies;	//Multiplies are cheaper narrow (the W65C816's: NarrowMultiplies).
	bool					mbConstantDivisions;	//Divides by constants are multiplies (SetConstantDivisions).
	bool					mbNarrowCounters;	//32 bit loop counters that fit count in 16 bits (SetNarrowCounters).
	bool					mbBiasedIndices;	//Signed i16 indices are biased unsigned ones (SetBiasedIndices).
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
	CArrayIRPromotions		masPromotions;
	CArrayTemplate<int32>	maiPromotion;		//Per temporary: the promotion its address is, -1 for none.
	CArrayTemplate<int32>	maiPhiPromotion;	//Per temporary: the promotion a phi made by it is for, -1 for none.
	CArrayIRReferences		masValues;			//The values being renamed, one stack per promotion...
	CArrayTemplate<int32>	maiValueLinks;		//...as a list through this (the previous value's index, -1).
	CArrayTemplate<int32>	maiTops;			//Per promotion: the top value's index, -1 for none.
	CArrayTemplate<int32>	maiPushedPromotions;	//The promotion of each value pushed, in order.
	CArrayIRReferences		masArgAddresses;	//Per parameter: its first argaddr, IRNone for none.

public:
	void			Init(CIRModule* pcModule);
	void			Kill(void);

	bool			Optimise(CIRFunction* pcFunction);
	void			SetIndexedAddresses(bool bIndexedAddresses);
	void			SetNarrowMultiplies(bool bNarrowMultiplies);
	void			SetConstantDivisions(bool bConstantDivisions);
	void			SetNarrowCounters(bool bNarrowCounters);
	void			SetBiasedIndices(bool bBiasedIndices);
	void			SetWarnings(CChars* pszWarnings);
	void			SetLibraryInt(SIRType sInt);
	void			SetProfile(CIRInlineProfile* pcProfile);

	bool			SimplifyCFG(void);
	bool			SplitSlots(void);
	bool			UnifySlotTypes(void);
	SIRType			SlotOffsetType(SIRReference sSlot, int64 lliOffset);
	bool			PromoteSlots(void);
	bool			RemoveTrivialPhis(void);
	bool			FoldConstants(void);
	bool			RemoveCommonSubexpressions(void);
	bool			RemoveRedundantLoads(void);
	bool			LessLoopTests(void);
	bool			ReplaceLoopCounters(void);
	bool			ReplaceExitValues(void);
	bool			ReduceMultiplies(void);
	bool			DeleteDeadLoops(void);
	bool			SingleExit(size uiLoop, CIRBlock** ppcFrom, CIRBlock** ppcTo);
	bool			UsedOutside(size uiLoop, SIRReference sValue);
	void			ReplaceOutside(size uiLoop, SIRReference sValue, SIRReference sNew);
	bool			SinkLoads(void);
	bool			HoistCommon(void);
	bool			SinkLoad(CIRBlock* pcFrom, size uiLoad, CIRBlock* pcTo, size uiUse);
	bool			WritesBetween(CIRBlock* pcBlock, size uiStart, size uiEnd, SIRReference sRoot, int64 lliOffset, int iBytes);
	bool			StepsByOne(SIRReference sStepped, SIRReference sValue);
	bool			LoopGuarded(size uiLoop, uint16 uiOpcode, SIRReference sInitial, SIRReference sBound);
	bool			ReduceStrength(void);
	SIRReference	ExtendedCounter(SIRReference sValue, SIRReference sCounter, bool bExtended, bool bZeroExtends, uint16* puiExtend);
	bool			CounterUnwrapped(size uiLoop, CIRBlock* pcLatch, SIRReference sCounter, SIRReference sInit, SIRReference sNext, int64 lliStep, uint64 ulliMask);
	bool			UnsignedMaximum(CIRBlock* pcBlock, CIRBlock* pcStay, SIRReference sValue, uint64 ulliMask, uint64* pulliMax, SIRReference* psBound, uint16* puiCompare);
	bool			HoistInvariants(void);
	bool			RotateLoops(void);
	bool			NarrowTruncations(void);
	bool			NarrowMultiplies(void);
	bool			SplitLoopAddresses(void);
	bool			UnswitchLoops(void);
	void			RewriteOutsideUses(SIRReference sValue, CArrayIRBlocks* papcBlocks, CArrayIRBlocks* papcCopies, CIRSSAUpdater* pcUpdater);
	void			DominatingRange(CIRBlock* pcBlock, SIRReference sValue, int64* plliMin, int64* plliMax);
	void			BranchRange(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue, int64* plliMin, int64* plliMax);
	bool			RangeAt(CIRBlock* pcBlock, SIRReference sValue, int64* plliMin, int64* plliMax, int iDepth = IR_SIGNED_RANGE_DEPTH, CIRBlock* pcTo = NULL);
	bool			DivisionRange(SIRInstruction* psInstruction, int64* plliMin, int64* plliMax, int iDepth);
	bool			ArithmeticRange(SIRInstruction* psInstruction, int64* plliMin, int64* plliMax, int iDepth);
	bool			SplitDivisor(SIRInstruction* psInstruction, int64 lliMin, int64 lliMax);
	bool			FindReciprocal(int64 lliMin, int64 lliMax, int64 lliDivisor, int iBits, bool bSigned, uint64* pulliMultiplier, int* piShift, int* piBits, int* piPreShift);
	bool			HasDivision(CIRBlock* pcBlock, uint16 uiOpcode, SIRReference sValue, SIRReference sDivisor);
	bool			NotNegative(CIRBlock* pcBlock, SIRReference sValue);
	bool			BoundDivisions(void);
	bool			BranchNonNull(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue);
	bool			NonNullAt(CIRBlock* pcBlock, SIRReference sValue, int iDepth);
	bool			FoldNullTests(void);
	bool			ConstantDivisions(void);
	bool			DivisibilityTests(void);
	bool			BiasSignedIndices(void);
	bool			MergeAddressPhis(void);
	SIRReference	AddressPhiIndex(SIRReference sValue, SIRReference sBase);
	void			CountAddressUses(CArrayTemplate<uint32>* pauiAddressUses);
	bool			IsHot(SIRInstruction* psInstruction, uint64 ulliSaving);
	bool			IsByteTableDivision(int64 lliMin, int64 lliMax, int64 lliDivisor);
	bool			NarrowCounters(void);
	bool			NarrowCounter(size uiLoop, CIRBlock* pcBlock);
	bool			NarrowLoopGuarded(size uiLoop, uint16 uiOpcode, SIRReference sInitial, SIRReference sBound);
	bool			SteppedExitUses(size uiLoop, CIRBlock* pcBlock, SIRPhi* psCounter, SIRInstruction* psCompare, SIRReference sStepped, CArrayIRExitArguments* papsExits);
	bool			NarrowGuardedAs(size uiLoop, uint16 uiOpcode, SIRReference sA, SIRReference sB);
	bool			SignedRange(SIRReference sValue, int64* plliMin, int64* plliMax, int iDepth = IR_SIGNED_RANGE_DEPTH);
	bool			DivisionReciprocal(int64 lliMin, int64 lliMax, int64 lliDivisor, int iBits, bool bSigned, uint64* pulliMultiplier, int* piShift);
	bool			SignedPowerDivisions(void);
	bool			ReplaceLoopIdioms(void);
	bool			RemoveDeadCode(void);
	bool			SinkComparisons(void);

protected:
	void			ClearReplacements(void);
	void			Replace(SIRReference sOld, SIRReference sNew);
	SIRReference	Replacement(SIRReference sReference);
	void			ApplyReplacements(void);
	bool			HasEffect(SIRInstruction* psInstruction);
	void			RemovePhiArguments(CIRBlock* pcBlock, CIRBlock* pcPredecessor);
	void			RenamePhiArguments(CIRBlock* pcBlock, CIRBlock* pcOld, CIRBlock* pcNew);
	void			RedirectJump(CIRBlock* pcBlock, CIRBlock* pcOld, CIRBlock* pcNew);
	void			Bypass(CIRBlock* pcPredecessor, CIRBlock* pcBlock, CIRBlock* pcTarget);
	bool			RemoveUnreachable(void);
	bool			FoldBranches(void);
	bool			SkipEmptyBlocks(void);
	int				KnownCondition(CIRBlock* pcPredecessor, CIRBlock* pcBlock, SIRReference sCondition);
	bool			ThreadBranches(void);
	bool			OnlyUsedInBlock(CIRBlock* pcBlock, bool bSuccessorPhis = false);
	void			CountSuccessorPhiUses(CIRBlock* pcBlock, CIRBlock* pcSuccessor, CArrayTemplate<uint32>* pauiInside);
	CIRBlock*		KnownTarget(CIRBlock* pcPredecessor, CIRBlock* pcBlock);
	bool			ThreadPhis(void);
	bool			DuplicateReturns(void);
	bool			JoinBlocks(void);
	int32			PromotionOf(SIRReference sAddress);
	void			FindPromotions(void);
	void			PlacePhis(void);
	void			PushValue(int32 iPromotion, SIRReference sValue);
	void			PopValue(void);
	SIRReference	CurrentValue(int32 iPromotion);
	void			Rename(void);
	void			RenameBlock(CIRBlock* pcBlock, size* puiPushed);
	void			FillSuccessorPhis(CIRBlock* pcBlock);
	void			RemovePromoted(void);
	bool			Fold(SIRInstruction* psInstruction, uint64* pulliResult);
	bool			UndoesExtension(SIRInstruction* psInstruction, SIRReference* psValue);
	bool			ConvertPhi(SIRInstruction* psInstruction, SIRReference* psValue);
	bool			IsIdentity(SIRInstruction* psInstruction, SIRReference* psValue);
	bool			Cancels(SIRInstruction* psInstruction, SIRReference* psValue);
	bool			PointerOffset(SIRInstruction* psInstruction);
	bool			Extended(SIRReference sValue, int iBits, bool bSigned, int iDepth);
	int				EntryRelations(CIRBlock* pcPredecessor, CIRBlock* pcBlock, SIRInstruction* psInstruction);
	bool			ImpliedComparison(CIRBlock* pcBlock, SIRInstruction* psInstruction, uint64* pulliResult, bool* pbChanged);
	void			FindDefinitions(void);
	void			ComputeLoops(void);
	SIRReference	ConstantData(SIRReference sRoot, int64 lliOffset, SIRType sType);
	SIRReference	SamePhi(CIRBlock* pcBlock, size uiPhi);
	void			MergeAvailable(CIRBlock* pcBlock, CArrayIRAvailables* pasAvailable, CArrayIRAvailables* pasEnds, CArrayTemplate<SIRBlockEnd>* pasBlockEnds);
	bool			CanStep(SIRReference sValue, int iStep);
	void			SplitAddress(SIRReference sAddress, SIRReference* psRoot, int64* plliOffset);
	bool			IsFrameRoot(SIRReference sRoot);
	void			PointerBase(SIRReference sRoot, SIRPointerBase* psBase);
	bool			MayAlias(SIRAvailable* psAvailable, SIRReference sRoot, int64 lliOffset, int iBytes);
	void			AddAvailable(CArrayIRAvailables* pasAvailable, SIRReference sRoot, int64 lliOffset, SIRType sType, SIRReference sValue);
	SIRReference	Unextended(SIRReference sValue, uint16 uiOpcode, SIRType* psType);
	bool			NarrowComparison(SIRInstruction* psInstruction);
	void			ComparisonRange(SIRInstruction* psInstruction);
	bool			SlotOffset(SIRReference sAddress, SIRReference* psSlot, int64* plliOffset);
	CIRBlock*		AddPreheader(size uiLoop);
	CIRBlock*		AddMergedPreheader(size uiLoop);
	bool			CanRotate(size uiLoop, CIRBlock** ppcBody, CIRBlock** ppcExit);
	void			CopyHead(CIRBlock* pcHead, CIRBlock* pcBlock, CArrayIRReferences* pasMap);
	void			SplitPhiArguments(CIRBlock* pcBlock, CIRBlock* pcHead, CIRBlock* pcPreheader, CArrayIRReferences* pasPreheader, CIRBlock* pcLatch, CArrayIRReferences* pasLatch);
	void			RewriteUses(SIRReference sValue, CIRSSAUpdater* pcUpdater);
	bool			MatchIdiomCounter(size uiLoop, CIRBlock* pcHead, CIRBlock* pcLatch, SIRIdiomCounter* psCounter);
	bool			MatchIdiomStep(size uiLoop, CIRBlock* pcHead, CIRBlock* pcLatch, SIRReference sValue, SIRIdiomCounter* psCounter);
	bool			MatchIdiomAddress(size uiLoop, CIRBlock* pcHead, CIRBlock* pcLatch, SIRReference sAddress, SIRIdiomCounter* psCounter, int64 lliBytes, SIRIdiomAddress* psAddress);
	bool			OnlyUsedInLoop(size uiLoop);
	bool			ReplaceStrlen(size uiLoop);
	bool			ReplaceMemcmp(size uiLoop);
	bool			ReplaceMemchr(size uiLoop);
	bool			ReplaceStrchr(size uiLoop);
	bool			IsLoopValue(size uiLoop, SIRReference sValue);
	bool			FoundValues(size uiLoop, CIRBlock* pcFrom, CIRBlock* pcFound, CArrayIRReferences* pasUsed);
	void			FoundAt(size uiLoop, CIRBlock* pcBlock, SIRReference sTimes, CIRBlock* pcFrom, CIRBlock* pcFound, CArrayIRReferences* pasUsed);
	bool			MatchLinear(size uiLoop, SIRReference sValue, SIRReference* psStart, int64* plliStep, bool* pbNext);
	SIRReference	IdiomStart(CIRBlock* pcBlock, SIRIdiomAddress* psAddress, SIRIdiomCounter* psCounter);
	SIRReference	IdiomBytes(CIRBlock* pcBlock, SIRIdiomCounter* psCounter, int64 lliBytes);
	bool			OnlyLoads(CIRBlock* pcBlock, int iLoads, SIRInstruction** apsLoads);
	bool			GuardedBy(size uiLoop, uint16 uiOpcode, SIRReference sLeft, SIRReference sRight);
	SIRReference	LibraryCall(CIRBlock* pcBlock, const char* szName, SIRType sResultType, SIRReference sFirst, SIRType sFirstType, SIRReference sSecond = IRNone(), SIRType sSecondType = IRType(IRTK_I32), SIRReference sThird = IRNone(), SIRType sThirdType = IRType(IRTK_I32));
	SIRReference	FreeNarrow(SIRReference sValue, SIRType sType);
	uint64			UpperBound(SIRReference sValue, int iDepth);
	uint64			EdgeBound(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue);
	SIRReference	InsertInstruction(CIRBlock* pcBlock, size uiIndex, uint16 uiOpcode, SIRType sType, SIRReference sFirst, SIRReference sSecond);
	SIRReference	Narrowed(SIRReference sValue, SIRType sType, CArrayIRInstructions* pasNew, int iDepth);
};


#endif // __I_R_OPTIMISER_H__
