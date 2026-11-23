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
#ifndef __I_R_W65_C816_SELECTOR_H__
#define __I_R_W65_C816_SELECTOR_H__
#include "IRBackend.h"


class CIRW65C816Backend;


//An IR instruction as the W65C816's own: Instruction writes one, by its kind (IRW65C816Instructions.cpp has
//the operands, loads, stores and arithmetic; IRW65C816Compares.cpp the comparisons and the flags they leave;
//IRW65C816Shifts.cpp, IRW65C816Multiplies.cpp and IRW65C816Floats.cpp what their names say).  It is told where
//every temporary lives, what is folded and what is fused by the back end's other parts.
class CIRW65C816Selector
{
protected:
	CIRW65C816Backend*	mpcBackend;

public:
	void	Init(CIRW65C816Backend* pcBackend);
	void	Kill(void);

	uint8			ZeroWords(SIRReference sValue);
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
	void			PushFrameAddress(int32 iOffset);
	void			PointerPlace(SIRReference sPointer, int32* piPlace, char** pszSymbol, int32* piOffset);
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
	void			SignWordInA(void);
	bool			RuntimeCall(SIRInstruction* psInstruction);
	bool			RemainderLeft(SIRInstruction* psInstruction);
	bool			InlineMultiply16(SIRInstruction* psInstruction);
	void			InlineCrossProduct(int32 iHigh, int32 iLow);
	void			TableProduct(void);
	bool			WideMultiply(SIRInstruction* psInstruction);
	bool			IsHighHalfZero(SIRReference sValue, int iDepth);
	SIRInstruction*	FindDefinition(SIRReference sValue);
	bool			IsScratchCall(SIRInstruction* psInstruction);
	void			Compare(SIRInstruction* psInstruction);
	bool			IsFusedCompare(CIRBlock* pcBlock, size uiIndex);
	void			FusedBranch(SIRInstruction* psInstruction, bool bWhen, int32 iLabel, bool bFlagsKnown, bool bSubtract);
	bool			EqualityFollows(CIRBlock* pcBlock);
	bool			SubtractsSigned(CIRBlock* pcBlock);
	bool			FlagsKnown(CIRBlock* pcBlock);
	bool			LeavesEqualityInZ(SIRInstruction* psInstruction);
	void			CompareFlags(SIRReference sLeft, SIRReference sRight, SIRType sType, bool bSigned);
	void			SignedLessFlags(SIRReference* psArguments, int iWords, bool bSwapped);
	bool			TableDivide(SIRInstruction* psInstruction, bool bSigned);
	bool			ByteMultiply(SIRInstruction* psInstruction);
	void			Extend(SIRInstruction* psInstruction);
	void			ExtendInX(SIRInstruction* psInstruction);
	void			Address(SIRInstruction* psInstruction, int32 iOffset);
	void			Load(SIRInstruction* psInstruction);
	void			WordInA(SIRInstruction* psInstruction, int iWord);
	void			StoreMemory(SIRInstruction* psInstruction);
	void			DirectStore(SIRInstruction* psInstruction, int32 iOffset);
	bool			OnlyCounts(SIRReference sValue);
	int32			SoftFloatCall(const char* szName, SIRReference* asValues, SIRType* asTypes, int iValues, int iExtended, bool bSigned, bool bHidden, int32 iHiddenOffset);
	void			FloatArithmetic(SIRInstruction* psInstruction);
	void			FloatCompare(SIRInstruction* psInstruction);
	void			FloatConversion(SIRInstruction* psInstruction);
	void			PushInt32(SIRReference sValue, SIRType sType, bool bSigned);
};


#endif // __I_R_W65_C816_SELECTOR_H__
