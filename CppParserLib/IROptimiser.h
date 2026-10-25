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


typedef CArrayTemplatePtr<SIRInstruction>	CArrayIRInstructionPtrs;


//A load or store of a slot at a constant offset, and the slot of its own it gets when the slot is split.
struct SIRSlotAccess
{
	SIRReference	sSlot;
	int64			lliOffset;
	SIRType			sType;
	SIRReference	sNew;
};


#define IR_AVAILABLE_LIMIT	64


//The mid level passes (IR.txt Passes), run on a function until nothing changes:
//
//	simplify cfg		removes unreachable blocks, jumps through empty blocks, joins a block to its only
//						successor when it is that block's only predecessor, and folds brif on a constant
//	slot splitting		a slot only loaded and stored at constant offsets that don't overlap (an object's
//						fields) becomes a slot per offset
//	slot promotion		slots and argaddrs only loaded and stored (all at one type, not volatile) become
//						temporaries and phis (as LLVM's mem2reg, with dominance frontiers)
//	trivial phis		a phi whose arguments are all one value (or itself) is that value
//	constant folding	integer instructions (and integer and pointer conversions) of constants; x * 2^n is
//						x << n; a comparison of two extended narrow values compares the narrow values
//	local cse			an instruction without effects that a block already did is the earlier result
//	redundant loads		a load of a value known to be in memory (loaded or stored before it, with nothing
//						that may change it between) is that value
//	invariant code		loads and integer operations in a loop of what the loop doesn't change move to the
//						block that enters it
//	dead code			instructions without effects whose results are not used
class CIROptimiser
{
protected:
	CIRModule*				mpcModule;
	CIRFunction*			mpcFunction;
	CIRDominators			mcDominators;
	CArrayIRReferences		masReplacements;	//Per temporary: what to use instead, IRNone to keep it.
	CArrayTemplate<uint32>	mauiUses;			//Per temporary.
	CArrayIRPromotions		masPromotions;
	CArrayTemplate<int32>	maiPromotion;		//Per temporary: the promotion its address is, -1 for none.
	CArrayTemplate<int32>	maiPhiPromotion;	//Per temporary: the promotion a phi made by it is for, -1 for none.
	CArrayIRReferences		masValues;			//The values being renamed, one stack per promotion...
	CArrayTemplate<int32>	maiValueLinks;		//...as a list through this (the previous value's index, -1).
	CArrayTemplate<int32>	maiTops;			//Per promotion: the top value's index, -1 for none.
	CArrayTemplate<int32>	maiPushedPromotions;	//The promotion of each value pushed, in order.
	CArrayIRBlocks			mapcFrontiers;		//Dominance frontiers, per block from maiFrontierStarts.
	CArrayTemplate<int32>	maiFrontierStarts;
	CArrayTemplate<int32>	maiChildren;		//The dominator tree, per block from maiChildStarts.
	CArrayTemplate<int32>	maiChildStarts;
	CArrayIRInstructionPtrs	mapsDefinitions;	//Per temporary: the instruction that defines it, NULL for none.
	CArrayIRReferences		masArgAddresses;	//Per parameter: its first argaddr, IRNone for none.

public:
	void			Init(CIRModule* pcModule);
	void			Kill(void);

	bool			Optimise(CIRFunction* pcFunction);

	bool			SimplifyCFG(void);
	bool			SplitSlots(void);
	bool			PromoteSlots(void);
	bool			RemoveTrivialPhis(void);
	bool			FoldConstants(void);
	bool			RemoveCommonSubexpressions(void);
	bool			RemoveRedundantLoads(void);
	bool			HoistInvariants(void);
	bool			RemoveDeadCode(void);

protected:
	void			ClearReplacements(void);
	void			Replace(SIRReference sOld, SIRReference sNew);
	SIRReference	Replacement(SIRReference sReference);
	void			ApplyReplacements(void);
	bool			HasEffect(SIRInstruction* psInstruction);

	bool			RemoveUnreachable(void);
	bool			FoldBranches(void);
	bool			SkipEmptyBlocks(void);
	bool			JoinBlocks(void);
	void			RemovePhiArguments(CIRBlock* pcBlock, CIRBlock* pcPredecessor);
	void			RenamePhiArguments(CIRBlock* pcBlock, CIRBlock* pcOld, CIRBlock* pcNew);
	void			RedirectJump(CIRBlock* pcBlock, CIRBlock* pcOld, CIRBlock* pcNew);

	void			FindPromotions(void);
	void			ComputeFrontiers(void);
	void			ComputeChildren(void);
	void			PlacePhis(void);
	void			Rename(void);
	void			RenameBlock(CIRBlock* pcBlock, size* puiPushed);
	void			FillSuccessorPhis(CIRBlock* pcBlock);
	void			PushValue(int32 iPromotion, SIRReference sValue);
	void			PopValue(void);
	SIRReference	CurrentValue(int32 iPromotion);
	int32			PromotionOf(SIRReference sAddress);
	bool			Fold(SIRInstruction* psInstruction, uint64* pulliResult);
	void			FindDefinitions(void);
	SIRInstruction*	Definition(SIRReference sValue);
	void			SplitAddress(SIRReference sAddress, SIRReference* psRoot, int64* plliOffset);
	bool			IsFrameRoot(SIRReference sRoot);
	bool			MayAlias(SIRAvailable* psAvailable, SIRReference sRoot, int64 lliOffset, int iBytes);
	void			AddAvailable(CArrayIRAvailables* pasAvailable, SIRReference sRoot, int64 lliOffset, SIRType sType, SIRReference sValue);
	SIRReference	Unextended(SIRReference sValue, uint16 uiOpcode, SIRType* psType);
	bool			NarrowComparison(SIRInstruction* psInstruction);
	bool			SlotOffset(SIRReference sAddress, SIRReference* psSlot, int64* plliOffset);
	void			RemovePromoted(void);
};


#endif // __I_R_OPTIMISER_H__
