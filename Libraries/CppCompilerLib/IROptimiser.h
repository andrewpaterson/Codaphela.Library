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
#ifndef __I_R_OPTIMISER_H__
#define __I_R_OPTIMISER_H__
#include "IRPasses.h"
#include "IROptimiserAddresses.h"
#include "IROptimiserBlocks.h"
#include "IROptimiserCounters.h"
#include "IROptimiserDivisions.h"
#include "IROptimiserFolding.h"
#include "IROptimiserIdioms.h"
#include "IROptimiserLoops.h"
#include "IROptimiserMemory.h"
#include "IROptimiserPromotion.h"
#include "IROptimiserRanges.h"


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
//
//The passes are in groups, a class each (CIRBlockPasses, CIRLoopPasses ...: the IROptimiser*.h files), which share
//what is in a CIRPassContext.  The optimiser owns them and runs them.
class CIROptimiser
{
protected:
	CIRPassContext			mcContext;
	CIRAddressPasses		mcAddressPasses;
	CIRBlockPasses			mcBlockPasses;
	CIRCounterPasses		mcCounterPasses;
	CIRDivisionPasses		mcDivisionPasses;
	CIRFoldingPasses		mcFoldingPasses;
	CIRIdiomPasses			mcIdiomPasses;
	CIRLoopPasses			mcLoopPasses;
	CIRMemoryPasses			mcMemoryPasses;
	CIRPromotionPasses		mcPromotionPasses;
	CIRRangePasses			mcRangePasses;

public:
	void			Init(CIRModule* pcModule);
	void			Kill(void);
	void			SetLibraryInt(SIRType sInt);
	void			SetIndexedAddresses(bool bIndexedAddresses);
	void			SetWarnings(CChars* pszWarnings);
	void			SetNarrowMultiplies(bool bNarrowMultiplies);
	void			SetConstantDivisions(bool bConstantDivisions);
	void			SetDivideRemainders(uint32 uiDivideRemainders);
	void			SetProfile(CIRInlineProfile* pcProfile);
	void			SetNarrowCounters(bool bNarrowCounters);
	void			SetBiasedIndices(bool bBiasedIndices);
	bool			Optimise(CIRFunction* pcFunction);
	bool			WritesMemory(CIRFunction* pcFunction);

protected:
	void			Begin(CIRFunction* pcFunction);
};


#endif // __I_R_OPTIMISER_H__

