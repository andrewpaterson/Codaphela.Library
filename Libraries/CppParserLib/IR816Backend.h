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
#ifndef __I_R_816_BACKEND_H__
#define __I_R_816_BACKEND_H__
#include "BaseLib/ArrayChars.h"
#include "Assembler65Lib/Asm65816Profile.h"
#include "IRBackend.h"
#include "IRBackendPasses.h"
#include "IRDominators.h"
#include "IRLoops.h"


class CIR816LiveRanges;
class CIR816Groups;


//The frame's first bytes (direct page offsets) are scratch: a pointer loads and stores go through, a call's result,
//a shift's count and a value being worked on.
#define IR816_SCRATCH_POINTER	1
#define IR816_SCRATCH_RESULT	5
#define IR816_SCRATCH_COUNT		13
#define IR816_SCRATCH_VALUE		15
#define IR816_SCRATCH_ARGUMENT_A	15		//A scratch runtime routine's (IR816ScratchRoutine) operands: a, then b.
#define IR816_SCRATCH_ARGUMENT_B	19
#define IR816_MAX_FUSED			4
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
//IR816_ALIGN_GAIN times the cost.  With a profile (CAsm65816Profile, CCPP65Builder::SetFrameProfile) the measure
//decides: it was never seen recursing, its accesses are more than IR816_ALIGN_GAIN times the cost of its calls and
//they save at least one IR816_ALIGN_SHARE'th of the profiled cycles.
#define IR816_ALIGN_GAIN		3
#define IR816_ALIGN_MAX_DEPTH	3
#define IR816_ALIGN_SHARE		10000
#define IR816_ALIGN_CYCLES		6
#define IR816_ALIGN_WORD_CYCLES	10
#define IR816_ALIGN_BIG_CYCLES	12


//The registers arguments may be passed in (A, X and Y), and what a word in a register is called in a function's
//register label (IR816AppendRegisterSuffix).
#define IR816_ARGUMENT_REGISTERS	3
#define IR816_REGISTER_LETTERS		"axy"


//One argument of a call or parameter of a function (the hidden result pointer first) as the register arguments place
//it: its words and the register of its first word (0 A, 1 X, 2 Y; its other words take the next ones), or -1 when it
//is pushed.
struct SIR816Argument
{
	int		iWords;
	int		iRegister;
};


typedef CArrayTemplate<SIR816Argument>	CArrayIR816Arguments;


//A function's frame (D + offset), as Layout lays it out, lowest first: the temporaries from IR816_FIRST_TEMPORARY;
//where an aligned frame past the page keeps S; the slots; the register words, at the frame's
//top.  Above it the link (the caller's D and the return address), then the pushed arguments, the hidden result
//pointer first.  AlignFrame gets it from GenerateFunction (GetFrame), with how the function uses the direct page.
struct SIR816Frame
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

	void	Init(int32 iRegisterWordsIn, bool bVariadicIn);
	int32	Add(int32 iBytes);
	int32	Link(void) const { return iFrameBytes + 1; }
	int32	Arguments(void) const { return Link() + IR816_LINK_BYTES; }
	int32	Top(void) const { return Arguments() + iArgumentBytes - 1; }
	int32	RegisterWord(int32 iWord) const { return iRegisterBase + 2 * iWord; }
	int32	ArgumentWord(int32 iWord) const { return Arguments() + 2 * iWord; }
};


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
//calls to runtime routines (R816_Mul16 ...), which GenerateRuntime generates.  The 16 and 32 bit multiply and unsigned
//divide and remainder take their operands in the caller's scratch bytes (IR816_SCRATCH_ARGUMENT_A and _B) and run on
//the caller's D, using its other scratch bytes; nothing is pushed for them.  Float operations call
//Libc65's soft float functions (_SoftFloatAdd ..., softfloat.cpp compiled for the W65C816).  A, X and Y may change
//in a call; D and the widths are kept.
//
//Register arguments (W65C816.txt): a function whose symbol is marked (CIRModule::SetRegisterCall: C++ linkage, not
//variadic) takes its arguments, the hidden result pointer first, in A, X and Y by a fixed rule from their sizes
//alone: each takes the next registers when all its words fit, else it is pushed as before.  Its code is at its label
//and a suffix naming each argument word's place (_Add__Fii.ax; IR816AppendRegisterSuffix), so a caller and a
//definition that don't agree don't link.  The prologue pushes the registers into the top of the frame.  The plain
//label is a stub for callers that push everything (function pointers, vtables, thunks, assembly): it loads the
//registers from the stack and goes on to the body.
class CIR816Backend : public CIRBackend
{
protected:
	CArrayIR816Arguments	masArguments;		//The function's hidden result pointer (if any) and parameters, as passed.
	SIR816Frame				msFrame;			//Its frame (FunctionArguments, Layout and GenerateBody).
	CArrayTemplate<int32>	maiOffsets;			//Per temporary: its place in the frame (D + offset).
	CArrayIRCopies			masCopies;			//A jump's phi copies (EdgeCopies)...
	CArrayIRCopySteps		masCopySteps;		//...in the order they are written (ParallelCopy).
	CArrayTemplate<int32>	maiSlots;			//Per temporary made by slot: the slot's place, else -1.
	CArrayTemplate<int32>	maiParameters;		//Per parameter: its place (D + offset).
	CArrayTemplate<size>	mauiPendingArguments;	//Instruction indices of the args before a call.
	CArrayTemplate<SIRReference>	masFoldBase;	//Per temporary: the pointer a folded pointer adds a constant to, else none.
	CArrayTemplate<int32>	maiFoldOffset;		//Per temporary: that constant.
	CArrayTemplate<SIRReference>	masFoldIndex;	//Per temporary: the i16 a folded pointer adds in Y instead (its offset is 0), else none.
	CArrayTemplate<int32>	maiUses;			//Per temporary: how many times it is used.
	CArrayTemplate<int32>	maiCountUses;		//Per temporary: how many of those are as the count of a shift by a count known when it runs.
	CArrayTemplate<int32>	maiUseBlock;		//Per temporary: the block of its one use by an instruction or a jump, else -1.
	int32					miBlock;			//The block being written.
	CIRDominators			mcDominators;		//The function's, worked out once (GenerateFunction)...
	CIRLoops				mcLoops;			//...and its loops: each block's depth (PackTemporaries, AllocateX, GenerateBody).
	CIRBackendPasses		mcBackendPasses;	//SinkSteps.
	CIRDominators*			mpcDominators;		//While the body is written: which jumps go round loops (JumpTo).
	CArrayTemplate<uint8>	mabInX;				//Per temporary: kept in X instead of its frame place (AllocateX).
	SIRInstruction*			mapsFused[IR816_MAX_FUSED];	//Not written: the next instruction reads their words into A as it goes (CanFuse).
	int						miFused;

public:
	void			Init(CIRModule* pcModule, int* piLabels);

	static void		IndexMnemonics(void);
	static void		SetRegisterArguments(bool bRegisterArguments);
	static bool		IsRegisterArguments(void);
	static CMallocator*	SetScratch(CMallocator* pcScratch);
	static CMallocator*	GetScratch(void);
	static void		SetSoftFloatInFrame(bool bInFrame);
	void			GetFrame(SIR816Frame* psFrame);
	static bool		AlignFrame(CChars* pszCode, const char* szLabel, const SIR816Frame* psFrame, bool bRecursive, CAsm65816Profile* pcProfile);
	void			Kill(void);
	bool			GenerateFunction(CIRFunction* pcFunction, const char* szLabel, CChars* pszDest, CCPPErrors* pcErrors);
	void			GenerateBody(const char* szLabel, size uiStart);
	void			DropFrame(size uiStart);
	void			FindSingleUses(void);
	void			ExpandMultiplies(void);
	SIRReference	InsertBefore(CIRBlock* pcBlock, size* puiIndex, uint16 uiOpcode, SIRType sType, SIRReference sFirst, SIRReference sSecond);

	static bool		GenerateRuntime(CArrayChars* paszRuntime, int* piLabels, CChars* pszDest, CCPPErrors* pcErrors);
	static void		AppendDivideTable(CChars* pszDest, const char* szName);
	static void		AppendBitsTable(CChars* pszDest, const char* szName);
	static void		Peephole(CChars* pszDest, size uiStart);
	static void		RemoveDeadStores(CChars* pszDest, size uiStart);
	static bool		RemoveFlowDeadStores(CChars* pszDest, size uiStart);
	static bool		PeepholeFlowRegisters(CChars* pszDest, size uiStart);
	static void		PeepholeBranches(CChars* pszDest, size uiStart);
	static void		PeepholeMasks(CChars* pszDest, size uiStart);
	static void		PeepholeStores(CChars* pszDest, size uiStart);
	static void		PeepholeTransfers(CChars* pszDest, size uiStart);
	static void		PeepholeRegisters(CChars* pszDest, size uiStart);
	static void		RemoveUnreadStores(CChars* pszDest, size uiStart);

protected:
	void			Line(const char* szText);
	void			Line(const char* szText, int64 lliValue);

	int				ValueBytes(SIRType sType);
	int				Words(SIRType sType);
	bool			IsHiddenResult(SIRType sType);
	void			Layout(void);
	void			AddRegions(CIR816LiveRanges* pcRanges);
	int32			PackTemporaries(void);
	int32			SharedLoopDepth(CIRBlock* pcFrom, CIRBlock* pcTo);
	int32			PlaceTemporaries(CIR816LiveRanges* pcRanges, CArrayTemplate<int32>* paiOrder, CArrayTemplate<bool>* pabParameter);
	void			GroupTemporaries(CIR816LiveRanges* pcRanges, CIR816Groups* pcGroups, CArrayTemplate<int32>* paiOrder, CArrayTemplate<bool>* pabParameter);
	bool			IsGroupFree(CIR816LiveRanges* pcRanges, CIR816Groups* pcGroups, int32 iGroup, int32 iOffset, CArrayTemplate<bool>* pabParameter);
	void			PlaceTemporary(CIR816LiveRanges* pcRanges, int32 iTemporary);
	int32			PlaceBytes(int32 iTemporary);
	void			AllocateX(CIR816LiveRanges* pcRanges, CArrayTemplate<bool>* pabParameter);
	bool			InX(SIRReference sValue);
	int32			OffsetOf(SIRReference sTemporary);

	void			Frame(const char* szMnemonic, int32 iOffset);
	void			Operand(const char* szMnemonic, SIRReference sValue, int iWord);
	void			Store(SIRReference sResult, int iWord);
	void			Mask(SIRType sType);
	bool			ConstantWord(SIRReference sValue, int iWord, uint16* puiWord);
	void			CopyWords(int32 iDestination, SIRReference sValue, int iWords);
	void			ZeroWords(int32 iDestination, int iFirst, int iWords);
	void			FillSign(int32 iDestination, int iFirst, int iWords);
	void			PushValue(SIRReference sValue, SIRType sType);
	void			PushWord(SIRReference sValue, int iWord);
	int32			ArgumentPlace(SIR816Argument* psArgument, int32* piPushed);
	void			PushFrameAddress(int32 iOffset);
	void			PointerPlace(SIRReference sPointer, int32* piPlace, char** pszSymbol, int32* piOffset);
	void			FoldPointers(void);
	SIRReference	FoldedBase(SIRReference sValue);
	SIRReference	FoldedIndex(SIRReference sValue);
	bool			IsFolded(SIRInstruction* psInstruction);
	void			FoldIndices(CArrayTemplate<int32>* paiUses);
	bool			LoadY(SIRReference sPointer, int32 iOffset);

	bool			Instruction(CIRBlock* pcBlock, size uiIndex);
	void			Arithmetic(SIRInstruction* psInstruction);
	bool			StepInPlace(SIRInstruction* psInstruction);
	void			Logic(SIRInstruction* psInstruction);
	void			Shift(SIRInstruction* psInstruction);
	bool			BitShift(SIRInstruction* psInstruction);
	bool			ShiftTwoWords(SIRInstruction* psInstruction);
	bool			ShiftCall(SIRInstruction* psInstruction);
	int32			FrameOperand(SIRReference sValue, SIRType sType, int32 iScratch);
	void			FrameOperandCall(const char* szName, SIRInstruction* psInstruction, int32 iFirst, int32 iSecond);
	void			ShiftInA(uint16 uiOpcode, int32 iResult, int iWords, SIRReference sAmount, int32 iLoop, int32 iEnd);
	void			ShiftStepInA(uint16 uiOpcode, int iWords, int32 iOther);
	void			ShiftInMemory(uint16 uiOpcode, int32 iResult, int iWords, SIRReference sAmount, int32 iLoop, int32 iEnd);
	void			ShiftConstant(uint16 uiOpcode, int32 iResult, int iWords, int iAmount);
	void			ShiftWordInA(uint16 uiOpcode, int iAmount);
	void			SignExtendInA(int iSignBit);
	bool			RuntimeCall(SIRInstruction* psInstruction);
	bool			WideMultiply(SIRInstruction* psInstruction);
	bool			IsHighHalfZero(SIRReference sValue, int iDepth);
	SIRInstruction*	FindDefinition(SIRReference sValue);
	void			RuntimeName(CChars* pszName, SIRInstruction* psInstruction);
	bool			IsScratchCall(SIRInstruction* psInstruction);
	void			Compare(SIRInstruction* psInstruction);
	bool			IsFusedCompare(CIRBlock* pcBlock, size uiIndex);
	void			FusedBranch(SIRInstruction* psInstruction, bool bWhen, int32 iLabel);
	void			Branch(const char* szMnemonic, int32 iLabel);
	bool			HasEdgeCopies(CIRBlock* pcFrom, CIRBlock* pcTo);
	void			BranchWhen(CIRBlock* pcBlock, bool bWhen, int32 iLabel);
	void			CompareFlags(SIRReference sLeft, SIRReference sRight, SIRType sType, bool bSigned);
	void			SignedLessFlags(SIRReference* psArguments, bool bSwapped);
	bool			TableDivide(SIRInstruction* psInstruction, bool bSigned);
	bool			ByteMultiply(SIRInstruction* psInstruction);
	void			Extend(SIRInstruction* psInstruction);
	void			ExtendInX(SIRInstruction* psInstruction);
	void			Address(SIRInstruction* psInstruction, int32 iOffset);
	void			Load(SIRInstruction* psInstruction);
	bool			CanFuse(CIRBlock* pcBlock, size uiIndex);
	bool			CanFuseReturn(CIRBlock* pcBlock, SIRInstruction* psInstruction);
	bool			FusedOverlaps(SIRReference sResult, SIRReference sInput, bool bAnyOverlap);
	bool			FusedInputsKept(SIRInstruction* psInstruction, SIRReference sResult);
	SIRInstruction*	Fused(SIRReference sValue);
	void			WordInA(SIRInstruction* psInstruction, int iWord);
	void			StoreMemory(SIRInstruction* psInstruction);
	void			Call(CIRBlock* pcBlock, SIRInstruction* psInstruction);
	int32			PushArguments(CIRBlock* pcBlock, SIRInstruction* psInstruction, CArrayIR816Arguments* pasArguments);
	void			LoadRegisterArguments(CIRBlock* pcBlock, SIRInstruction* psInstruction, CArrayIR816Arguments* pasArguments);
	void			LoadRegisters(SIRReference* asValues, int* aiWords, int32* aiHidden, int iRegisters);
	bool			LoadsRegisterDirectly(SIRReference sValue, int iWord);
	void			LoadRegisterWord(int iRegister, SIRReference sValue, int iWord, int32 iHiddenResult);
	bool			IsRegisterCall(SIRReference sCallee);
	void			FunctionArguments(void);
	void			RegisterStub(CChars* pszBodyLabel);
	bool			Forwarder(CChars* pszJump);
	void			GenerateForwarder(const char* szLabel, CChars* pszJump);
	bool			OnlyCounts(SIRReference sValue);
	void			RegisterStubLoad(int* aiWordOf, int iHigh, int iLow, int iPushed);
	void			RegisterStubCall(CChars* pszBodyLabel, int* aiWordOf, int iWords, int iStacked);
	void			StackLine(const char* szInstruction, int iOffset);
	void			CallerCleanUp(int32 iBytes, SIRInstruction* psInstruction);
	void			CallFunction(const char* szName, int32 iBytes, SIRInstruction* psInstruction);
	int32			SoftFloatCall(const char* szName, SIRReference* asValues, SIRType* asTypes, int iValues, int iExtended, bool bSigned, bool bHidden, int32 iHiddenOffset);
	void			FloatArithmetic(SIRInstruction* psInstruction);
	void			FloatCompare(SIRInstruction* psInstruction);
	void			FloatConversion(SIRInstruction* psInstruction);
	void			PushInt32(SIRReference sValue, SIRType sType, bool bSigned);

	void			Prologue(const char* szLabel);
	void			Epilogue(void);
	void			Return(CIRBlock* pcBlock);
	void			EdgeCopies(CIRBlock* pcFrom, CIRBlock* pcTo);
	void			AddCopy(SIRPhi* psPhi, SIRPhiArgument* psArgument);
	bool			CopyOverlaps(SIRReference sSource, SIRReference sDestination);
	bool			CopySamePlace(SIRReference sValue1, SIRReference sValue2);
	void			JumpTo(CIRBlock* pcFrom, CIRBlock* pcTo, CIRBlock* pcNext);
	void			Jump(CIRBlock* pcBlock, CIRBlock* pcNext);
};


#endif // __I_R_816_BACKEND_H__

