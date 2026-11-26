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
#ifndef __I_R_816_BACKEND_H__
#define __I_R_816_BACKEND_H__
#include "BaseLib/ArrayChars.h"
#include "Assembler65Lib/Asm65Profile.h"
#include "BaseLib/Numbers.h"
#include "BaseLib/ScratchPadAllocator.h"
#include "IRBackend.h"
#include "IRBackendPasses.h"
#include "IRDominators.h"
#include "IRLoops.h"
#include "IRW65C816Places.h"
#include "IRW65C816Folds.h"
#include "IRW65C816Fusion.h"
#include "IRW65C816Calls.h"
#include "IRW65C816Selector.h"
#include "IRW65C816Runtime.h"


class CIRW65C816LiveRanges;
class CIRW65C816Code;
class CIRW65C816Groups;


//The frame's first bytes (direct page offsets) are scratch: a pointer loads and stores go through, a call's result,
//a shift's count and a value being worked on.
#define IR816_SCRATCH_POINTER	1
#define IR816_SCRATCH_RESULT	5
#define IR816_SCRATCH_COUNT		13
#define IR816_SCRATCH_VALUE		15
#define IR816_SCRATCH_ARGUMENT_A	15		//A scratch runtime routine's (IR816ScratchRoutine) operands: a, then b.
#define IR816_SCRATCH_ARGUMENT_B	19
#define IR816_SCRATCH_REMAINDER		5		//Where the divide routines leave the remainder (RemainderLeft).
#define IR816_MAX_FUSED			4
#define IR816_CARRY_PULL_BYTES	8		//A tested call's arguments are pulled, which keeps the carry, up to this many bytes (a ply each two).
#define IR816_FIRST_TEMPORARY	23

//The bytes between the frame and the arguments: the caller's D (phd) and the return address (jsl).
#define IR816_LINK_BYTES		5

//The stack is in bank 0 (from the target's stack top down) with the direct page: a frame may use most of it, but
//not all.
#define IR816_MAX_FRAME_BYTES	0xC000

//A direct page access costs a cycle more when D isn't on a page.  A function may get a page aligned frame (AlignFrame),
//costing each call IR816_ALIGN_CYCLES, IR816_ALIGN_WORD_CYCLES for each argument word copied into the frame and
//IR816_ALIGN_BIG_CYCLES more for a frame past the page, and up to 255 bytes of stack.  It never does if it may recurse.
//By default the estimate decides (the user: only when really worth it): it has a loop, and its direct page accesses,
//each weighted 8 times for each loop it is in (CIRLoops::Weight, up to IR816_ALIGN_MAX_DEPTH), are more than
//IR816_ALIGN_GAIN times the cost.  With a profile (CAsm65Profile, CCPP65Builder::SetFrameProfile) the measure
//decides: it was never seen recursing, its accesses are more than IR816_ALIGN_GAIN times the cost of its calls and
//they save at least one IR816_ALIGN_SHARE'th of the profiled cycles.
#define IR816_ALIGN_GAIN		3
#define IR816_ALIGN_MAX_DEPTH	3
#define IR816_ALIGN_SHARE		10000
#define IR816_ALIGN_CYCLES		6
#define IR816_ALIGN_WORD_CYCLES	10
#define IR816_ALIGN_BIG_CYCLES	12


//The registers arguments may be passed in (A, X and Y), and what a word in a register is called in a function's
//register label (CIRBackend::AppendRegisterSuffix).  An argument's units (SIRRegisterArgument) are its words, and
//any that fits the registers left goes in them (CIRBackend::AssignRegisters).
#define IR816_ARGUMENT_REGISTERS	3
#define IR816_REGISTER_LETTERS		"axy"


//A function's frame (D + offset), as Layout lays it out, lowest first: the temporaries from IR816_FIRST_TEMPORARY;
//where an aligned frame past the page keeps S; the slots; the register words, at the frame's
//top.  Above it the link (the caller's D and the return address), then the pushed arguments, the hidden result
//pointer first.  AlignFrame gets it from GenerateFunction (GetFrame), with how the function uses the direct page.
struct SIRW65C816Frame
{
	int32	iFrameBytes;		//D + 1 to D + iFrameBytes.
	int32	iSavedS;			//Where an aligned frame past the page keeps S, or -1.
	int32	iRegisterWords;		//The arguments' words that came in registers (pushed by the prologue to the frame's top).
	int32	iRegisterBase;		//Where they are in the frame, or -1.
	int32	iArgumentBytes;		//The pushed arguments' bytes (the hidden result pointer's too).
	int32	iHiddenResult;		//The hidden result pointer's place, -1 when the result isn't 64 bits.
	int32	iDirectPageWeight;	//The direct page accesses, weighted by the loops they are in.
	bool	bLoops;
	bool	bVariadic;
	bool	bNoResult;			//Nothing is returned in A (void, or through the hidden result pointer): its exits needn't keep A.

	void	Init(int32 iRegisterWordsIn, bool bVariadicIn);
	int32	Add(int32 iBytes);
	int32	Link(void) const { return iFrameBytes + 1; }
	int32	Arguments(void) const { return Link() + IR816_LINK_BYTES; }
	int32	Top(void) const { return Arguments() + iArgumentBytes - 1; }
	int32	SavedS(void) const;
	int32	RegisterWord(int32 iWord) const { return iRegisterBase + 2 * iWord; }
	int32	ArgumentWord(int32 iWord) const { return Arguments() + 2 * iWord; }
};


#define IR816_DIRECT_PAGE	256		//The bytes [D + n] reaches (IR816FindReads).


//One of a function's frame exits as Epilogue wrote it, in the printed code's lines: where S moves back (tdc, clc, adc
//frame; AlignFrame's) and its rtl.
struct SIRW65C816Exit
{
	int32	iMove;
	int32	iReturn;
	bool	bClearBefore;	//With exceptions a clc goes before the rtl once S's move back is a load (IR816ClearExitCarries).
};


typedef CArrayTemplate<SIRW65C816Exit>	CArrayIRW65C816Exits;


//What FinishFunction (AlignFrame) and DescribeFrame look for in a function's code, kept when it is written
//(KeepFacts) so they needn't read it.  Lines are the printed code's, from its label, without markers.
struct SIRW65C816Facts
{
	int32	iPrologue;			//The frame's entry as Prologue wrote it (not DropFrame's), or -1...
	int32	iPrologueLines;		//...and its lines.
	uint8	abRead[IR816_DIRECT_PAGE];	//The direct page bytes the code reads (IR816FindReads)...
	bool	bAllRead;			//...or any of them.
	bool	bCallsItself;		//A jsl of the function (of its body, for a register function).
	bool	bBody;				//A register function's body label was found (the name is CIRW65C816Written's).
	int32	iDescribeBody;		//The line after the function's label (after its body's, for a register function), or -1...
	int32	iDescribeAfter;		//...the line after its frame's entry when its code starts with one (phd to tcd), or -1...
	int32	iDescribeBelow;		//...and the bytes that entry pushes and moves S down.
	bool	bAligned;			//AlignFrame put the frame on a page (iDescribeAfter is then after S's store).
};


//What the W65C816 back end keeps about a function it wrote: its frame, for AlignFrame, per direct page byte the most
//bytes a place marker says a value there has (DebugPlace), for FinishFunction, and what FinishFunction and
//DescribeFrame look for in its code.
class CIRW65C816Written : public CIRWrittenFunction
{
public:
	SIRW65C816Frame			msFrame;
	uint8					mauiPlaceBytes[IR816_DIRECT_PAGE];
	SIRW65C816Facts			msFacts;
	CArrayIRW65C816Exits	masExits;
	CChars					mszBody;		//A register function's body label (its profile's name).

	void	Init(void);
	void	Kill(void) override;
};


//How much of its arena (Init's) the back end leaves in it from one function to the next.
#define IR816_ARENA_KEPT	(1 MB)



//What the W65C816 back end knows of one temporary of the function it is writing.  Each part is found by its own
//pass over the function, which sets it for every temporary first.
struct SIRW65C816Temporary
{
	int32			iOffset;		//Its place in the frame (D + offset), or -1.
	int32			iSlot;			//Made by a slot: the slot's place, else -1.
	SIRReference	sFoldBase;		//The pointer a folded pointer adds a constant to, else none...
	int32			iFoldOffset;	//...and that constant.
	SIRReference	sFoldIndex;		//The i16 a folded pointer adds in Y instead (its offset is 0), else none.
	int32			iUses;			//How many times it is used...
	int32			iCountUses;		//...and how many of those are as the count of a shift by a count known when it runs.
	int32			iUseBlock;		//The block of its one use by an instruction or a jump, else -1.
	uint8			uiZeroWords;	//A bit for each of its words that is always 0 (FindZeroWords).
	bool			bLowByteOnly;	//An i8 whose every use reads its low byte alone (FindLowByteUses): not masked.
	bool			bInX;			//Kept in X instead of its frame place (AllocateX).
	bool			bDirectSlot;	//A slot only loaded and stored through (FindDirectSlots): addressed in place, never worked out.
};


typedef CArrayTemplate<SIRW65C816Temporary>	CArrayIRW65C816Temporaries;


//Writes W65C816 assembly (Assembler65816Syntax.txt) for IR functions, without optimising (W65C816.txt step 4).
//
//The direct page is the frame: the prologue saves D, makes room on the stack and points D at it, so every
//temporary, slot and argument is a direct page operand ([D + n], or [D + X] with X = n past 255).  Every temporary
//has its own place: i1, i8 and i16 a word (zero extended), i32, f32 and pointers two, i64 and f64 four.  Everything
//is worked out in A, 16 bits wide; an 8 bit load or store is the only time m is 8 bits.  Pointers are 4 bytes (24
//bit addresses, the top byte 0) and are followed with [[D + n]:24 + Y].
//
//Calls are jsl / rtl.  Arguments are pushed last first, a word for i1 to i16, two for 32 bits, four for 64, and
//the caller removes them.  A result of up to 16 bits is in A, of 32 bits in A (low) and X (high); a 64 bit result
//is written through a hidden pointer the caller pushes after the arguments (the function's first parameter).  A
//call through a pointer pushes the return address and the target then rtl's.  Multiply, divide and remainder are
//calls to runtime routines (R816_Mul16 ...): Libc65's IRW65C816Runtime-*.asm, linked as a library
//(CCPP65Builder::AddRuntime).  The 16 and 32 bit
//multiply and unsigned divide and remainder take their operands in the caller's scratch bytes (IR816_SCRATCH_ARGUMENT_A
//and _B) and run on the caller's D, using its other scratch bytes; nothing is pushed for them.  Float operations call
//Libc65's soft float functions (_SoftFloatAdd ..., softfloat.cpp compiled for the W65C816).  A, X and Y may change
//in a call; D and the widths are kept.
//
//Register arguments (W65C816.txt): a function whose symbol is marked (CIRModule::SetRegisterCall: C++ linkage, not
//variadic) takes its arguments, the hidden result pointer first, in A, X and Y by a fixed rule from their sizes
//alone: each takes the next registers when all its words fit, else it is pushed as before.  Its code is at its label
//and a suffix naming each argument word's place (_Add__Fii.ax; AppendRegisterSuffix), so a caller and a
//definition that don't agree don't link.  The prologue pushes the registers into the top of the frame.  The plain
//label is a stub for callers that push everything (function pointers, vtables, thunks, assembly): it loads the
//registers from the stack and goes on to the body.
class CIRW65C816Backend : public CIRBackend
{
friend class CIRW65C816Runtime;
friend class CIRW65C816Selector;
friend class CIRW65C816Calls;
friend class CIRW65C816Fusion;
friend class CIRW65C816Folds;
friend class CIRW65C816Places;
protected:
	CArrayIRRegisterArguments	masArguments;		//The function's hidden result pointer (if any) and parameters, as passed.
	SIRW65C816Frame			msFrame;			//Its frame (FunctionArguments, Layout and GenerateBody).
	CArrayIRW65C816Temporaries	masTemporaries;	//What is known of each temporary of the function being written (GrowTemporaries)...
	CIRFunction*			mpcTemporariesOf;	//...which is this one.
	CArrayIRCopies			masCopies;			//A jump's phi copies (EdgeCopies)...
	CArrayIRCopySteps		masCopySteps;		//...in the order they are written (ParallelCopy).
	CArrayTemplate<int32>	maiParameters;		//Per parameter: its place (D + offset).
	CArrayTemplate<size>	mauiPendingArguments;	//Instruction indices of the args before a call.
	bool					mbUnmasked;			//The instruction being written leaves such a result: Mask does nothing for an i8.
	int32					miBlock;			//The block being written.
	CIRDominators			mcDominators;		//The function's, worked out once (GenerateFunction)...
	CIRLoops				mcLoops;			//...and its loops: each block's depth (PackTemporaries, AllocateX, GenerateBody).
	CIRBackendPasses		mcBackendPasses;	//SinkSteps.
	CIRDominators*			mpcDominators;		//While the body is written: which jumps go round loops (JumpTo).
	uint8					mauiPlaceBytes[IR816_DIRECT_PAGE];	//As CIRW65C816Written's.
	SIRInstruction*			mapsFused[IR816_MAX_FUSED];	//Not written: the next instruction reads their words into A as it goes (CanFuse).
	int						miFused;
	SIRInstruction*			mpsDivided;			//The last udiv or sdiv that called a divide routine (RemainderLeft).
	SIRInstruction*			mpsWritten;			//The instruction GenerateBody wrote last in the block, NULL at its start or after a fused one.
	bool					mbMathTables;		//An i8 multiply reads MathMult in place (SetMathTables), else it calls R816_Mul16.
	CScratchPadAllocator*	mpcArena;			//Init's arena, this thread's scratch (SetScratch) until Kill, or NULL...
	CMallocator*			mpcOldScratch;		//...and what the thread's scratch was before.

	CIRW65C816Code*			mpcCode;			//The function's code as it is written (GenerateFunction).
	SIRW65C816Facts			msFacts;			//What FinishFunction and DescribeFrame look for in it (KeepFacts)...
	CArrayIRW65C816Exits	masExits;
	CChars					mszBody;

	CIRW65C816Places			mcPlaces;
	CIRW65C816Folds				mcFolds;
	CIRW65C816Fusion			mcFusion;
	CIRW65C816Calls				mcCalls;
	CIRW65C816Selector			mcSelector;
	CIRW65C816Runtime			mcRuntime;
public:
	void			Init(CIRModule* pcModule, int* piLabels, CScratchPadAllocator* pcArena = NULL) override;
	void			SetOptions(CCPPCompileOptions* pcOptions) override;

	static void		IndexMnemonics(void);
	static CMallocator*	SetScratch(CMallocator* pcScratch);
	static CMallocator*	GetScratch(void);
	void			SetMathTables(bool bMathTables);
	void			GetFrame(SIRW65C816Frame* psFrame);
	CIRWrittenFunction*	TakeWritten(void) override;
	int32			VariableSlotPlace(SIRInstruction* psSlot) override;
	bool			DebugPlace(SIRReference sValue, CChars* pszPlace) override;
	void			UnreadPlaces(CChars* pszCode, CIRW65C816Written* pcWritten, uint8* abRead);
	int32			VariableSlotBytes(SIRInstruction* psSlot);
	void			FinishFunction(CChars* pszCode, const char* szLabel, CIRWrittenFunction* pcWritten, bool bRecursive, CAsm65Profile* pcProfile) override;
	void			MarkExits(void);
	void			KeepFacts(const char* szLabel);
	void			KeepDescribeFacts(const char* szLabel);
	void			DescribeFrame(CChars* pszCode, const char* szLabel, CIRWrittenFunction* pcWritten) override;
	bool			AlignFrame(CChars* pszCode, const char* szLabel, CIRW65C816Written* pcWritten, bool bRecursive, CAsm65Profile* pcProfile);
	void			Kill(void) override;
	bool			GenerateFunction(CIRFunction* pcFunction, const char* szLabel, CChars* pszDest, CCPPErrors* pcErrors) override;
	void			GenerateBody(const char* szLabel);
	void			RunPasses(void);
	void			Label(int32 iLabel);
	void			LabelLine(const char* szName);
	void			Directive(const char* szName, const char* szOperand = NULL);
	void			Lines(CIRW65C816Code* pcFrom);
	void			KeepSlots(CIRW65C816Code* pcCode);
	void			DropFrame(void);
	bool			ReturnsNothingInA(void);
	void			GrowTemporaries(void);
	void			FindSingleUses(void);
	void			FindZeroWords(void);
	void			FindLowByteUses(void);
	bool			IsLowByteOnly(SIRReference sValue);
	bool			IsZeroWord(SIRReference sValue, int iWord);
	int				MultiplyCost(SIRType sType, uint64 ulliMultiplier) override;
	int				ShiftAddCost(SIRType sType, int* aiPlaces, int* aiSigns, int iDigits) override;

	static void		Peephole(CChars* pszDest, size uiStart);
	static void		Peephole(CIRW65C816Code* pcCode);
	static void		RemoveDeadStores(CChars* pszDest, size uiStart);
	static void		RemoveDeadStores(CIRW65C816Code* pcCode);
	static bool		RemoveFlowDeadStores(CChars* pszDest, size uiStart);
	static bool		RemoveFlowDeadStores(CIRW65C816Code* pcCode);
	static bool		PeepholeFlowRegisters(CIRW65C816Code* pcCode);
	static void		PeepholeBranches(CChars* pszDest, size uiStart);
	static void		PeepholeBranches(CIRW65C816Code* pcCode);
	static void		PeepholeMasks(CChars* pszDest, size uiStart);
	static void		PeepholeMasks(CIRW65C816Code* pcCode);
	static void		PeepholeStores(CChars* pszDest, size uiStart);
	static void		PeepholeStores(CIRW65C816Code* pcCode);
	static void		PeepholeTransfers(CChars* pszDest, size uiStart);
	static void		PeepholeTransfers(CIRW65C816Code* pcCode);
	static void		PeepholeRegisters(CChars* pszDest, size uiStart);
	static void		PeepholeRegisters(CIRW65C816Code* pcCode);
	static void		RemoveUnreadStores(CChars* pszDest, size uiStart);
	static void		RemoveUnreadStores(CIRW65C816Code* pcCode);

protected:
	void			Line(uint16 uiMnemonic, const char* szOperand = NULL);
	void			Line(uint16 uiMnemonic, int64 lliOperand);
	void			Line(uint16 uiMnemonic, const char* szOperand, bool bMarked);

	int				ValueBytes(SIRType sType);
	int				Words(SIRType sType);
	bool			IsHiddenResult(SIRType sType);

	int32			ArgumentPlace(SIRRegisterArgument* psArgument, int32* piPushed);

	void			Branch(uint16 uiMnemonic, int32 iLabel);
	bool			HasEdgeCopies(CIRBlock* pcFrom, CIRBlock* pcTo);
	void			BranchWhen(CIRBlock* pcBlock, bool bWhen, int32 iLabel);
	void			FunctionArguments(void);
	void			CopyAddressedParameters(void);

	void			Prologue(const char* szLabel);
	void			Epilogue(void);
	void			FrameExit(void);
	void			Return(CIRBlock* pcBlock);
	void			EdgeCopies(CIRBlock* pcFrom, CIRBlock* pcTo);
	void			AddCopy(SIRPhi* psPhi, SIRPhiArgument* psArgument);
	bool			CopyOverlaps(SIRReference sSource, SIRReference sDestination);
	bool			CopySamePlace(SIRReference sValue1, SIRReference sValue2);
	void			JumpTo(CIRBlock* pcFrom, CIRBlock* pcTo, CIRBlock* pcNext);
	void			Jump(CIRBlock* pcBlock, CIRBlock* pcNext);
};


#endif // __I_R_816_BACKEND_H__

