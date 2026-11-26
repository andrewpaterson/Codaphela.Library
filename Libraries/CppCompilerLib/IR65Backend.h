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
#ifndef __I_R_65_BACKEND_H__
#define __I_R_65_BACKEND_H__
#include "CPPErrors.h"
#include "IRModule.h"
#include "IRLiveness.h"
#include "IRDominators.h"
#include "IRMost.h"
#include "IR65Flags.h"
#include "IR65Widths.h"
#include "IR65Peephole.h"
#include "IRBackend.h"
#include "IRBackendPasses.h"


//Where a temporary is kept.
enum EIR65PlaceKind
{
	IR65P_None,				//Nowhere: left in A for the next instruction (bInA), or not used.
	IR65P_Frame,			//In the frame: [1 + offset + V].
	IR65P_Argument,			//A parameter where the caller pushed it: [1 + frame + 8 + offset + V].
	IR65P_FrameAddress,		//A slot's address, V + 1 + offset; not stored.
	IR65P_ArgumentAddress,	//An argaddr's address, V + 1 + frame + 8 + offset; not stored.
	IR65P_Register,			//A register (offset is IR65R_X, IR65R_Y, IR65R_V, IR65R_W or IR65R_B).
	IR65P_Pseudo,			//A pseudo register: [offset + W].
	IR65P_Folded,			//A ptradd of sBase and offset only the next instruction uses, as its address.
};


//The registers a temporary can be kept in.  A and B are worked in.
enum EIR65Register
{
	IR65R_X,
	IR65R_Y,
	IR65R_V,				//Only in a function that needs no frame (SIR65Frame::bHomeV); 32 bit values.
	IR65R_W,				//Between calls, multiplies and divides, which change it; 32 bit values, never an address.
	IR65R_B,				//Where the code written doesn't work in B (CheckHomeB); 32 bits or less, not a parameter.
	IR65R_Count
};


//The code written for one or more of a block's instructions (or its jump and copies): the liveness positions of the
//first and last, and where the text is.  For checking that a value kept in B isn't lost (CheckHomeB).
struct SIR65Chunk
{
	int32	iFrom;
	int32	iTo;
	size	uiStart;
	size	uiEnd;
};


typedef CArrayTemplate<SIR65Chunk>	CArrayIR65Chunks;


//A temporary's home (or, for a slot or argaddr, the address it is), from register allocation.
struct SIR65Place
{
	uint8			uiKind;
	int32			iOffset;
	int32			iBytes;		//What is kept: 4 or 8, a parameter's own size where the caller pushed it, a slot's size.
	bool			bInA;		//Only the next instruction uses it, and takes it from A.
	SIRReference	sBase;		//What a folded ptradd adds offset to.
};


typedef CArrayTemplate<SIR65Place>	CArrayIR65Places;


//What is known of one temporary of the function being written.  Each pass sets what it finds for every temporary
//before it is read (CountUses, FindNarrowOnly, Layout, Allocate), so a record left from another function is never
//read.
struct SIR65Temporary
{
	SIR65Place	sPlace;			//Its home, or for a slot or argaddr the address it is (Layout, then Allocate).
	SIR65Place	sLaidOut;		//Its place as Layout left it, for Allocate again (GenerateFunction, Reallocate).
	int32		iNextInClass;	//The temporaries sharing a home are a list through this (Allocate, Coalesce)...
	int32		iLeader;		//...and this is the first of its list.
	uint32		uiUses;			//How many times it is used (CountUses).
	bool		bNarrowOnly;	//A narrow load only compared (FindNarrowOnly).
};


typedef CArrayTemplate<SIR65Temporary>	CArrayIR65Temporaries;


//The ranges of the temporaries given one home (a register, or four bytes of pseudo registers or frame), sorted.
struct SIR65Home
{
	CArrayIRRanges	asRanges;
	int32			iOffset;		//A frame unit's offset.
};


typedef CArrayTemplate<SIR65Home>	CArrayIR65Homes;


//How a value is extended to 32 bits when it is loaded.
enum EIR65Extend
{
	IR65E_Zero,
	IR65E_Sign,
};


//A copy a block's jump makes for a successor's phi (IR65Backend.cpp Edge).




//An edge's phi copies written after the function's ret: a branch goes to iLabel, which copies then jumps on.
struct SIR65Stub
{
	int32		iLabel;
	CIRBlock*	pcFrom;
	CIRBlock*	pcTo;
	int32		iRemove;	//Bytes of a call's arguments still on the stack on the way in (the way a tested call threw).
	bool		bTested;	//A tested call's way when it threw: one that copies nothing shares its code with those like it (Epilogue).
	bool		bWritten;	//Written already, as one of the stubs that share their code.
};


typedef CArrayTemplate<SIR65Stub>	CArrayIR65Stubs;


//How an instruction not written in its turn (CanFuse) is written by the next one written, its consumer, as the
//W65C816 back end's fused instructions are.
enum EIR65FusedKind
{
	IR65FU_InA,			//Written at the start of its consumer, which takes its result from A.
	IR65FU_Deferred,	//A load its consumer reads as a memory operand (mszDeferred): add A, [8 + X].
	IR65FU_Extended,	//A narrow load the sext after it writes, sign extending it as it widens it.
	IR65FU_Paired,		//A udiv or urem its partner after it writes with itself (DividePartner): one div gives both.
};


struct SIR65Fused
{
	SIRInstruction*	psInstruction;
	size			uiIndex;		//In its block.
	uint8			uiKind;			//EIR65FusedKind.
};


typedef CArrayTemplate<SIR65Fused>	CArrayIR65Fused;


//A class of temporaries sharing a home and how much they are used, for the order homes are given in.
struct SIR65Weight
{
	int32	iLeader;
	uint32	uiWeight;
};


typedef CArrayTemplate<SIR65Weight>	CArrayIR65Weights;


#define IR65_PSEUDO_UNITS		32		//Four byte units of [$00 + W] to [$7F + W], and again from [$80 + W].
#define IR65_PSEUDO_UNIT_BYTES	4
#define IR65_PSEUDO_SAVED		0x80	//The units from here are callee saved: the prologue pushes those used.
#define IR65_SAVED_UNIT(iUnit)	(IR65_PSEUDO_SAVED + IR65_PSEUDO_UNIT_BYTES * (iUnit))
#define IR65_REGISTER_GAIN		3			//The cycles a use of a value in a register saves over one in a pseudo register or the frame.
#define IR65_KEPT_COST			40			//What a kept pseudo register costs in a function with V as a home, as weight (PaysV): its push and
//pull are 10 cycles, but a weight counts a loop as 8 passes and the loops that hold such values (calls
//in them) make far fewer; 40 is the measured best across the three suites, 2026-11-13.
#define IR65_V_WEIGHT			3			//The least weight of a class not live across a call that is worth saving V for.
#define IR65_INLINE_RETURN		3			//The most instructions an epilogue has to be written at each return rather than jumped to.
#define IR65_HOME_B_TRIES		3			//The most times a function's code is written again with B as a home (GenerateFunction).
#define IR65_HOME_PSEUDO		IR65R_Count
#define IR65_HOME_SAVED			(IR65_HOME_PSEUDO + IR65_PSEUDO_UNITS)
#define IR65_HOME_FRAME			(IR65_HOME_SAVED + IR65_PSEUDO_UNITS)


//The registers a function's first arguments are passed in (A, B and Y; a thiscall's this is in X besides), and what
//each is called in its register label (CIRBackend::AppendRegisterSuffix).  An argument's unit (SIRRegisterArgument)
//is 32 bits, and only an argument of one unit goes in a register (CIRBackend::AssignRegisters).
#define IR65_ARGUMENT_REGISTERS	3
#define IR65_REGISTER_LETTERS		"aby"


//A function's frame: [1 + offset + V] for offset from 0 to iFrameBytes - 1, then the link (8 bytes), then the
//arguments.  A frameless function (no frame bytes, no this kept) doesn't set V up and finds its arguments from S.
struct SIR65Frame
{
	int32	iFrameBytes;
	int32	iThisOffset;	//A thiscall's this in the frame, for an argaddr of it; -1 for none.
	uint32	uiSaved;		//The [$80 + W] units used, a bit each: the prologue pushes them.
	bool	bHomeV;		//V is a home (IR65R_V) and not the frame's register: the function has no frame and saves V (CanUseV).
	bool	bFrameless;		//No frame: V isn't set up, the arguments are from S...
	char	cRegister;		//...so the frame register is S, otherwise V.
	int32	iArguments;		//Where the arguments start: 1 + frame + 8, or from S 1 + 4 + 4 per saved unit (and 4 for a saved V).
	int32	aiRegisterOffset[IR65_ARGUMENT_REGISTERS];	//A register parameter in the frame, for an argaddr of it; -1 for none.

	void	Init(void);
	int32	Add(int32 iBytes);
	int		NumSaved(void) const;
	void	SetFrameless(bool bFramelessIn);
};


#include "IR65Places.h"
#include "IR65Folds.h"
#include "IR65Fusion.h"
#include "IR65Calls.h"
#include "IR65Selector.h"


//Writes 65C64 assembly for IR functions.  Temporaries are given homes by register allocation (IR.txt 65C64 back
//end): X or Y, W between calls, B between calls where the code doesn't work in it (CheckHomeB), V in a function that
//needs no frame (CanUseV: calls keep it), a pseudo register ([$00 + W] to
//[$7F + W] if no call is made while it is live, [$80 + W] to [$FF + W] if one is: the function keeps those), or
//the frame.  A phi shares its home with the values it takes
//when their live ranges allow, and a jump copies the others.  Instructions work in A (and B), or in a result's
//register or pseudo register when that is shorter.  The calling convention is the generator's
//(CPP65Generator.txt): a function called by name gets its first three arguments of 32 bits or less in A, B and Y
//at its register label (FunctionArguments, Call), the rest and every argument of a call through a pointer are
//pushed last first at their own size and the caller removes them, this in X for a thiscall, the result in A (all 8
//bytes of a 64 bit one).  Labels are L and a number from the generator's
//counter; the soft float routines called are added to its runtime list.
//What the 65C64 back end keeps of a function it wrote (CIRBackend::TakeWritten): its frame, for a debugger, and
//whether the prologue that makes it is after the body's label as written (miPrologueLines lines) or the passes
//changed it.
class CIR65Written : public CIRWrittenFunction
{
public:
	SIR65Frame	msFrame;
	bool		mbPrologue;
	int32		miPrologueLines;

	void	Init(void);
};


class CIR65Backend : public CIRBackend
{
friend class CIR65Selector;
friend class CIR65Calls;
friend class CIR65Fusion;
friend class CIR65Folds;
friend class CIR65Places;
protected:
	CIRDominators			mcDominators;
	CIRLiveness				mcLiveness;
	CArrayIR65Temporaries	masTemporaries;		//What is known of each temporary of the function being written (GrowTemporaries).
	CArrayTemplate<uint8>	mauiAddressed;		//Per parameter: 1 if an argaddr of it is taken.
	CArrayIRRegisterArguments	masArguments;	//The function's parameters as its callers pass them, less a thiscall's this (FunctionArguments).
	int						miRegisterArguments;	//How many of them arrive in A, B and Y.
	CArrayIR65Homes			masHomes;			//X, Y, the pseudo registers, the saved pseudo registers, the frame.
	CArrayTemplate<int32>	maiCalls;			//Positions of instructions that change X, Y and [$00 + W] to [$7F + W]...
	CArrayTemplate<int32>	maiChangesXY;		//...that change X and Y...
	CArrayTemplate<int32>	maiChangesY;		//...and that change Y.
	CArrayTemplate<int32>	maiChangesB;		//Positions whose code wrote B when the function was last written: a class live across one isn't kept in B...
	CArrayTemplate<int32>	maiLosesB;			//...nor one live at a position whose code lost a value kept in B (CheckHomeB).
	CArrayIR65Chunks		masChunks;			//The code of each instruction or jump, as it is written (Block).
	bool					mbHomeB;			//B may be a home in this Allocate.
	CIRMost					mcMost;				//The largest each temporary can be: a result that fits its type isn't masked.
	CArrayIRCopies			masCopies;			//A jump's phi copies (Edge)...
	CArrayIRCopySteps		masCopySteps;		//...in the order they are written (ParallelCopy).
	CArrayIR65Stubs			masStubs;
	SIR65Frame				msFrame;			//The function's frame (Layout, Allocate and GenerateFunction).
	bool					mbCopyAddress;		//The memory operand being written is a cpy's, which may address with W ([W]): MemoryOperand.
	uint32					muiWeightV;			//The weights of the classes whose home is V (PaysV).
	bool					mbFrameOnly;		//The function will have a frame: no kept pseudo registers (Allocate).
	int32					miReturnLabel;
	SIRReference			msInA;				//The temporary the last instruction left in A (or IRNone).
	SIRReference			msAvailable;		//What A holds as the current instruction starts.
	CArrayIR65Fused			masFused;			//The instructions the next one written writes (CanFuse), in order.
	CChars					mszDeferred;		//The memory operand of the IR65FU_Deferred load among them.
	CChars					mszBodyLabel;		//A register function's body's label (after its stub), "" for none.
	SIRInstruction*			mpsFlags;			//The last integer comparison a block (mpcFlags) jumped on: the flags its successors start with.
	CIRBlock*				mpcFlags;
	bool					mbFlagsKnown;		//The block's comparison uses the flags it is entered with (FlagsKnown).
	bool					mbRightImmediate;	//LoadOperands left the second operand a constant, for "op A, value".
	const char*				mszAddress;			//Where MemoryOperand puts an address it must load: B, or for a cpyx a free X or Y (not A).
	CIR65Code				mcCode;				//The function's body (from its label) as it is written and the passes change it (RunPasses).
	CIR65Code				mcHead;				//A register function's label, stub and directive, before the body.
	CIR65Code*				mpcWriting;			//The one of them being written.
	bool					mbPrologue;			//The frame's prologue is after the body's label as written, after the passes...
	int32					miPrologueLines;	//...in this many lines (KeepPrologue).
	uint8					muiPending;			//The instruction being written (Line), not in the code yet (IR65M_Unknown for none)...
	CChars					mszPending;			//...and its operands as far as they are written.
	CIR65Flags				mcFlags;
	CIR65Widths				mcWidths;
	CIR65Peephole			mcPeephole;
	CIRBackendPasses		mcBackendPasses;

	CIR65Places					mcPlaces;
	CIR65Folds					mcFolds;
	CIR65Fusion					mcFusion;
	CIR65Calls					mcCalls;
	CIR65Selector				mcSelector;
public:
	void				Init(CIRModule* pcModule, int* piLabels, CScratchPadAllocator* pcArena = NULL) override;
	void				Kill(void) override;

	bool				GenerateFunction(CIRFunction* pcFunction, const char* szLabel, CChars* pszDest, CCPPErrors* pcErrors) override;
	CIRWrittenFunction*	TakeWritten(void) override;
	void				DescribeFrame(CChars* pszCode, const char* szLabel, CIRWrittenFunction* pcWritten) override;
	static void			AppendFramePrologue(CChars* pszDest, SIR65Frame* psFrame);
	static void			FramePrologue(CIR65Code* pcCode, SIR65Frame* psFrame);
	static bool			PeepholeCode(CIR65Code* pcCode, CArrayTemplate<int32>* paiBlockLabels, bool bHomeV);
	static void			PeepholeCode(CChars* pszCode, size uiStart, CArrayTemplate<int32>* paiBlockLabels, bool bHomeV);

	bool				RunStoreLoads(void);
	bool				RunFlags(void);
	bool				RunWidths(void);
	bool				RunPeephole(void);
	bool				RunPseudoWithoutW(void);

protected:
	void				GrowTemporaries(void);
	void				CountUses(void);
	SIR65Temporary*		GetTemporary(size uiTemporary);
	void				AdjustComparisons(void);
	void				PreferSingleBranches(void);
	bool				ConstantPtrAdd(SIRInstruction* psInstruction);
	void				CopyAddresses(void);
	void				RunPasses(void);
	void				KeepPrologue(void);
	void				CountLoaded(bool bLoaded);
	void				Prologue(const char* szLabel);
	int32				VariableSlotPlace(SIRInstruction* psSlot) override;
	bool				DebugPlace(SIRReference sValue, CChars* pszPlace) override;
	void				Epilogue(void);
	void				ReturnCode(bool bThrow = false);
	int					ReturnInstructions(void);
	CChars*				Line(uint8 uiMnemonic, const char* szOperands = NULL);
	void				Branch(uint8 uiMnemonic, const char* szCondition, int32 iLabel);
	void				Label(int32 iLabel);
	void				TextLine(const char* szText);
	void				Commit(void);
	void				Write(CIR65Code* pcCode);
	size				CodeLines(void);
	void				CutCode(size uiLines);
	void				MarkerLine(const char* szMarker) override;
	int					ValueBytes(SIRType sType);
	bool				IsWide(SIRType sType);
	bool				IsNarrow(SIRType sType);
	bool				EdgeCopies(CIRBlock* pcFrom, CIRBlock* pcTo);
	void				StoreRegisterParameters(void);
	void				StoreArrivedRegister(const char* szRegister, SIR65Place* psPlace, SIRType sType, const char** aszSources, SIR65Place** apsPlaces, int* piMoves);
	bool				WriteBody(const char* szLabel, size uiStart);
	void				Reallocate(SIR65Frame* psLaidOut, bool bHomeB);
	bool				HasMoves(CIRBlock* pcFrom, CIRBlock* pcTo);
	bool				Skipped(SIRInstruction* psInstruction);
	size				NextWritten(CIRBlock* pcBlock, size uiAfter, size uiEnd);
	bool				Block(CIRBlock* pcBlock, CIRBlock* pcNext);
	bool				WriteInstruction(CIRBlock* pcBlock, size uiIndex);
	bool				WriteFused(CIRBlock* pcBlock, size uiCount);
	void				NextInstruction(void);
	bool				BranchOn(SIRInstruction* psFused, SIRReference sCondition, bool bJumpIf, int32 iLabel);
	bool				EnteredOnlyFrom(CIRBlock* pcBlock, CIRBlock* pcFrom);
	bool				IsZero(SIRReference sValue);
	bool				FlagsKnown(CIRBlock* pcBlock, SIRInstruction* psFused);
	void				Edge(CIRBlock* pcFrom, CIRBlock* pcTo);
	void				Move(SIRReference sDestination, SIRReference sSource, SIRType sType, bool bFromB);
	bool				CopyOverlaps(SIRReference sSource, SIRReference sDestination);
	bool				CopySamePlace(SIRReference sValue1, SIRReference sValue2);
	void				TestZero(SIRReference sValue);
	bool				Jump(CIRBlock* pcBlock, CIRBlock* pcNext, SIRInstruction* psFused);
	int					MultiplyCost(SIRType sType, uint64 ulliMultiplier) override;
	int					ShiftAddCost(SIRType sType, int* aiPlaces, int* aiSigns, int iDigits) override;
};


#endif // __I_R_65_BACKEND_H__
