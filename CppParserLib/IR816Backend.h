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
#include "CPPErrors.h"
#include "Assembler65Lib/Asm816Profile.h"
#include "IRModule.h"
#include "IRSourceMarkers.h"


class CIR816LiveRanges;


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
//each weighted by IR816_ALIGN_LOOP_WEIGHT for each loop it is in (up to IR816_ALIGN_MAX_DEPTH), are more than
//IR816_ALIGN_GAIN times the cost.  With a profile (CAsm816Profile, CCPP65Builder::SetFrameProfile) the measure
//decides: it was never seen recursing, its accesses are more than IR816_ALIGN_GAIN times the cost of its calls and
//they save at least one IR816_ALIGN_SHARE'th of the profiled cycles.
#define IR816_ALIGN_GAIN		3
#define IR816_ALIGN_LOOP_WEIGHT	8
#define IR816_ALIGN_MAX_DEPTH	3
#define IR816_ALIGN_SHARE		10000
#define IR816_ALIGN_CYCLES		6
#define IR816_ALIGN_WORD_CYCLES	10
#define IR816_ALIGN_BIG_CYCLES	12


//What AlignFrame needs of a function's frame, from GenerateFunction (GetFrame).
struct SIR816Frame
{
	int32	iFrameBytes;
	int32	iArgumentBytes;
	int32	iSavedS;			//Where an aligned frame past the page keeps S, or -1.
	int32	iDirectPageWeight;	//The direct page accesses, weighted by the loops they are in.
	bool	bLoops;
	bool	bVariadic;
};


//Writes W65C816 assembly (Assembler816Syntax.txt) for IR functions, without optimising (W65C816.txt step 4).
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
class CIRDominators;
class CIR816Backend
{
protected:
	CIRModule*				mpcModule;
	int*					mpiLabels;
	CArrayChars				maszRuntime;		//The runtime routines used.
	CIRFunction*			mpcFunction;
	CChars*					mpszOut;
	uint32					muiSource;			//The source token + 1 of the last marker written (Source).
	bool					mbSourceMarkers;	//Source writes markers (SetSourceMarkers), for a listing.
	CIRSourceMarkers		mcSourceMarkers;	//The markers while the text passes run.
	CCPPErrors*				mpcErrors;
	bool					mbResult;
	CArrayTemplate<int32>	maiOffsets;			//Per temporary: its place in the frame (D + offset).
	CArrayTemplate<int32>	maiIncoming;		//Per phi: where its value is put on the way in, else -1.
	CArrayTemplate<int32>	maiSlots;			//Per temporary made by slot: the slot's place, else -1.
	CArrayTemplate<int32>	maiBlockLabels;		//Per block index.
	CArrayTemplate<int32>	maiParameters;		//Per parameter: its place (D + offset).
	CArrayTemplate<size>	maiPendingArguments;	//Instruction indices of the args before a call.
	int32					miFrameBytes;
	int32					miHiddenResult;		//The hidden result pointer's place, -1 when the result isn't 64 bits.
	int32					miArgumentBytes;	//The arguments' bytes (the hidden result pointer's too) above the frame.
	int32					miSavedS;			//Where an aligned frame past the page keeps S (Layout), or -1.
	int32					miDirectPageWeight;	//The direct page accesses, weighted by the loops they are in (AlignFrame).
	bool					mbLoops;
	const char*				mszSectionOptions;	//After each function's section name: ", comdat" for the runtime.
	CArrayTemplate<SIRReference>	masFoldBase;	//Per temporary: the pointer a folded pointer adds a constant to, else none.
	CArrayTemplate<int32>	maiFoldOffset;		//Per temporary: that constant.
	CArrayTemplate<SIRReference>	masFoldIndex;	//Per temporary: the i16 a folded pointer adds in Y instead (its offset is 0), else none.
	CArrayTemplate<int32>	maiUses;			//Per temporary: how many times it is used.
	CArrayTemplate<int32>	maiUseBlock;		//Per temporary: the block of its one use by an instruction or a jump, else -1.
	int32					miBlock;			//The block being written.
	CIRDominators*			mpcDominators;		//While the body is written: which jumps go round loops (JumpTo).
	CArrayTemplate<uint8>	mabInX;				//Per temporary: kept in X instead of its frame place (AllocateX).
	SIRInstruction*			mapsFused[IR816_MAX_FUSED];	//Not written: the next instruction reads their words into A as it goes (CanFuse).
	int						miFused;

public:
	void			Init(CIRModule* pcModule, int* piLabels);
	void			GetFrame(SIR816Frame* psFrame);
	static bool		AlignFrame(CChars* pszCode, const char* szLabel, SIR816Frame* psFrame, bool bRecursive, CAsm816Profile* pcProfile);
	void			Kill(void);
	bool			GenerateModule(CChars* pszDest, CCPPErrors* pcErrors);
	bool			GenerateFunction(CIRFunction* pcFunction, const char* szLabel, CChars* pszDest, CCPPErrors* pcErrors);
	void			SetSourceMarkers(bool bSourceMarkers);
	void			GenerateBody(const char* szLabel, size uiStart);
	void			DropFrame(size uiStart);
	void			FindSingleUses(void);
	void			ExpandMultiplies(void);
	SIRReference	InsertBefore(CIRBlock* pcBlock, size* puiIndex, uint16 uiOpcode, SIRType sType, SIRReference sFirst, SIRReference sSecond);
	void			GenerateGlobal(CIRGlobal* pcGlobal, CChars* pszDest);
	CArrayChars*	GetRuntime(void);

	static bool		GenerateRuntime(CArrayChars* paszRuntime, int* piLabels, CChars* pszDest, CCPPErrors* pcErrors);
	static void		PeepholeBranches(CChars* pszDest, size uiStart);

protected:
	void			Binding(const char* szName, uint32 uiAttributes, CChars* pszDest);
	bool			Error(const char* szMessage, const char* szDetail = NULL);
	void			Line(const char* szText);
	void			Source(uint32 uiSource);
	void			Line(const char* szText, int64 lliValue);
	int32			NewLabel(void);
	void			Label(int32 iLabel);
	void			UseRuntime(const char* szName);

	int				ValueBytes(SIRType sType);
	int				Words(SIRType sType);
	bool			IsHiddenResult(SIRType sType);
	void			Layout(void);
	void			Peephole(CChars* pszDest, size uiStart);
	void			RemoveDeadStores(CChars* pszDest, size uiStart);
	int32			PackTemporaries(void);
	void			AllocateX(CIR816LiveRanges* pcRanges, CArrayTemplate<bool>* pabParameter);
	bool			InX(SIRReference sValue);
	int32			Allocate(int32* piNext, int32 iBytes);
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
	void			ShiftInA(uint16 uiOpcode, int32 iResult, int iWords, SIRReference sAmount, int32 iLoop, int32 iEnd);
	void			ShiftInMemory(uint16 uiOpcode, int32 iResult, int iWords, SIRReference sAmount, int32 iLoop, int32 iEnd);
	void			ShiftConstant(uint16 uiOpcode, int32 iResult, int iWords, int iAmount);
	void			ShiftWordInA(uint16 uiOpcode, int iAmount);
	bool			RuntimeCall(SIRInstruction* psInstruction);
	void			Compare(SIRInstruction* psInstruction);
	bool			IsFusedCompare(CIRBlock* pcBlock, size uiIndex);
	void			FusedBranch(SIRInstruction* psInstruction, bool bWhen, int32 iLabel);
	void			Branch(const char* szMnemonic, int32 iLabel);
	bool			HasEdgeCopies(CIRBlock* pcFrom, CIRBlock* pcTo);
	void			BranchWhen(CIRBlock* pcBlock, bool bWhen, int32 iLabel);
	void			CompareFlags(SIRReference sLeft, SIRReference sRight, SIRType sType, bool bSigned);
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
	void			CallerCleanUp(int32 iBytes, SIRInstruction* psInstruction);
	void			CallFunction(const char* szName, int32 iBytes, SIRInstruction* psInstruction);
	void			FloatArithmetic(SIRInstruction* psInstruction);
	void			FloatCompare(SIRInstruction* psInstruction);
	void			FloatConversion(SIRInstruction* psInstruction);
	void			PushInt32(SIRReference sValue, SIRType sType, bool bSigned);

	void			Prologue(const char* szLabel);
	void			Epilogue(void);
	void			Return(CIRBlock* pcBlock);
	void			EdgeCopies(CIRBlock* pcFrom, CIRBlock* pcTo);
	void			PhiCopies(CIRBlock* pcBlock);
	bool			IsCoalescedPhiBlock(CIRBlock* pcBlock);
	void			JumpTo(CIRBlock* pcFrom, CIRBlock* pcTo, CIRBlock* pcNext);
	void			Jump(CIRBlock* pcBlock, CIRBlock* pcNext);
};


#endif // __I_R_816_BACKEND_H__

