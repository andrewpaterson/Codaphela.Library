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
#ifndef __I_R65_SELECTOR_H__
#define __I_R65_SELECTOR_H__
#include "IRBackend.h"


class CIR65Backend;


//An IR instruction as the 65C64's own: Instruction writes one, by its kind (IR65Instructions.cpp has the
//arithmetic, loads and stores; IR65Operands.cpp the operands and what loads and stores them; IR65Compares.cpp,
//IR65Multiplies.cpp and IR65Floats.cpp what their names say).  It is told where every temporary lives, what is
//folded and what is fused by the back end's other parts.
class CIR65Selector
{
protected:
	CIR65Backend*	mpcBackend;

public:
	void	Init(CIR65Backend* pcBackend);
	void	Kill(void);

	bool				DividePartner(SIRInstruction* psFirst, SIRInstruction* psPartner);
	void				AppendPlace(CChars* psz, SIR65Place* psPlace);
	void				AppendImmediate(CChars* psz, SIRReference sConstant, EIR65Extend eExtend);
	bool				Operand(SIRReference sValue, CChars* pszOperand, bool bArithmetic, EIR65Extend eExtend);
	bool				SmallImmediate(SIRReference sValue, int iMin, int iMax);
	const char*			WidthRegister(const char* szRegister);
	const char*			WorkRegister(SIRInstruction* psInstruction);
	bool				IsRegisterFree(int iRegister, CIRBlock* pcBlock, size uiIndex);
	const char*			FreeAddressRegister(CIRBlock* pcBlock, size uiIndex, const char* szPrefer);
	bool				CopyExtendOperand(SIRReference sAddress, CChars* pszOperand, const char* szWidened, CIRBlock* pcBlock, size uiIndex, const char* szPrefer);
	void				Extend(const char* szRegister, SIRType sType, EIR65Extend eExtend);
	void				Truncate(const char* szRegister, SIRType sType);
	void				LoadConstant(const char* szRegister, SIRType sType, uint64 ulliBits, EIR65Extend eExtend);
	void				LoadPlace(SIR65Place* psPlace, SIRType sType, const char* szRegister, EIR65Extend eExtend);
	void				StorePlace(SIR65Place* psPlace, SIRType sType, const char* szRegister);
	void				LoadValue(SIRReference sValue, const char* szRegister, EIR65Extend eExtend);
	void				StoreResult(SIRReference sResult, const char* szRegister = "A", bool bExtended = false);
	void				MemoryOperand(SIRReference sAddress, CChars* pszOperand, const char* szWidened);
	void				FoldedOperand(SIR65Place* psFolded, CChars* pszOperand, const char* szWidened);
	void				Push(SIRReference sValue, SIRType sType);
	void				Multiply(uint8 uiMnemonic);
	SIRInstruction*		PairedDivide(CIRBlock* pcBlock, size uiIndex);
	bool				PairsWithDivide(SIRInstruction* psInstruction, SIRInstruction* psPartner);
	bool				Instruction(CIRBlock* pcBlock, size uiIndex);
	bool				Commutes(uint16 uiOpcode);
	bool				ReadModifyWrite(SIRInstruction* psInstruction, SIRInstruction* psStore);
	bool				Integer(SIRInstruction* psInstruction);
	bool				IntegerWide(SIRInstruction* psInstruction);
	bool				MultiplyInstruction(SIRInstruction* psInstruction);
	bool				ShiftSigned(SIRInstruction* psInstruction);
	bool				Divide(SIRInstruction* psInstruction, SIRInstruction* psNext);
	bool				Conversion(SIRInstruction* psInstruction);
	bool				FloatOperation(SIRInstruction* psInstruction);
	bool				FloatConversion(SIRInstruction* psInstruction);
	bool				PureMemoryOperand(SIRReference sAddress, CChars* pszOperand, char* pcRegister);
	bool				Load(CIRBlock* pcBlock, size uiIndex, SIRInstruction* psExtend);
	bool				Store(CIRBlock* pcBlock, size uiIndex);
	bool				Compare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel);
	void				CompareOperands(SIRReference sFirst, SIRReference sSecond);
	bool				IntegerCompare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel);
	bool				NarrowMemoryOperand(SIRReference sValue, CChars* pszOperand);
	bool				FloatCompare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel);
	bool				TakesInA(SIRInstruction* psNext, SIRReference sValue);
	bool				JumpTakesInA(CIRBlock* pcBlock, SIRInstruction* psFused, SIRReference sValue);
	bool				Immediate(SIRReference sValue, SIRType sType);
	void				LoadOperands(SIRInstruction* psInstruction, EIR65Extend eExtend, bool bImmediate);
	bool				TakesInB(SIRInstruction* psNext, SIRReference sValue);
};


#endif // __I_R65_SELECTOR_H__
