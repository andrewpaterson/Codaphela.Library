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
#include "IRModule.h"


//The frame's first bytes (direct page offsets) are scratch: a pointer loads and stores go through, a call's result,
//a shift's count and a value being worked on.
#define IR816_SCRATCH_POINTER	1
#define IR816_SCRATCH_RESULT	5
#define IR816_SCRATCH_COUNT		13
#define IR816_SCRATCH_VALUE		15
#define IR816_FIRST_TEMPORARY	23

//The bytes between the frame and the arguments: the caller's D (phd) and the return address (jsl).
#define IR816_LINK_BYTES		5


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
//calls to runtime routines written in IR (R816_Mul16 ...), which GenerateRuntime generates.  Float operations call
//Libc65's soft float functions (_SoftFloatAdd ..., softfloat.cpp compiled for the W65C816).  A, X and Y may change
//in a call; D and the widths are kept.
class CIR816Backend
{
protected:
	CIRModule*				mpcModule;
	int*					mpiLabels;
	CArrayChars				maszRuntime;		//The runtime routines used.
	CIRFunction*			mpcFunction;
	CChars*					mpszOut;
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
	const char*				mszSectionOptions;	//After each function's section name: ", comdat" for the runtime.

public:
	void			Init(CIRModule* pcModule, int* piLabels);
	void			Kill(void);
	bool			GenerateModule(CChars* pszDest, CCPPErrors* pcErrors);
	bool			GenerateFunction(CIRFunction* pcFunction, const char* szLabel, CChars* pszDest, CCPPErrors* pcErrors);
	void			GenerateGlobal(CIRGlobal* pcGlobal, CChars* pszDest);
	CArrayChars*	GetRuntime(void);

	static bool		GenerateRuntime(CArrayChars* paszRuntime, int* piLabels, CChars* pszDest, CCPPErrors* pcErrors);

protected:
	void			Binding(const char* szName, uint32 uiAttributes, CChars* pszDest);
	bool			Error(const char* szMessage, const char* szDetail = NULL);
	void			Line(const char* szText);
	void			Line(const char* szText, int64 lliValue);
	int32			NewLabel(void);
	void			Label(int32 iLabel);
	void			UseRuntime(const char* szName);

	int				ValueBytes(SIRType sType);
	int				Words(SIRType sType);
	bool			IsHiddenResult(SIRType sType);
	void			Layout(void);
	int32			Allocate(int32* piNext, int32 iBytes);
	int32			OffsetOf(SIRReference sTemporary);

	void			Frame(const char* szMnemonic, int32 iOffset);
	void			Operand(const char* szMnemonic, SIRReference sValue, int iWord);
	void			Store(SIRReference sResult, int iWord);
	void			Mask(SIRType sType);
	void			CopyWords(int32 iDestination, SIRReference sValue, int iWords);
	void			ZeroWords(int32 iDestination, int iFirst, int iWords);
	void			FillSign(int32 iDestination, int iFirst, int iWords);
	void			PushValue(SIRReference sValue, SIRType sType);
	void			PushFrameAddress(int32 iOffset);
	void			PointerPlace(SIRReference sPointer, int32* piPlace, char** pszSymbol);

	bool			Instruction(CIRBlock* pcBlock, size uiIndex);
	void			Arithmetic(SIRInstruction* psInstruction, const char* szCarry, const char* szMnemonic);
	void			Logic(SIRInstruction* psInstruction, const char* szMnemonic, uint64 ulliNot);
	void			Shift(SIRInstruction* psInstruction);
	bool			RuntimeCall(SIRInstruction* psInstruction);
	void			Compare(SIRInstruction* psInstruction);
	void			CompareFlags(SIRReference sLeft, SIRReference sRight, SIRType sType, bool bSigned);
	void			Extend(SIRInstruction* psInstruction);
	void			Address(SIRInstruction* psInstruction, int32 iOffset);
	void			Load(SIRInstruction* psInstruction);
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
	void			JumpTo(CIRBlock* pcTo, CIRBlock* pcNext);
	void			Jump(CIRBlock* pcBlock, CIRBlock* pcNext);
};


#endif // __I_R_816_BACKEND_H__

