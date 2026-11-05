/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela EmulatorDebugLib

Codaphela EmulatorDebugLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela EmulatorDebugLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela EmulatorDebugLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __FLAT_65C64_DEBUG_TARGET_H__
#define __FLAT_65C64_DEBUG_TARGET_H__
#include "MicroprocessorLib/Fast65C64.h"
#include "DebugTarget.h"


enum EFlat65C64Register
{
	F64R_A,
	F64R_B,
	F64R_X,
	F64R_Y,
	F64R_S,
	F64R_V,
	F64R_W,
	F64R_C,
	F64R_P,

	F64R_NumRegisters
};


//A CFast65C64 with nothing but memory: a flat block at 0000:0000, uiMemorySize bytes.  Reads past it are $FFFF
//and writes are ignored.  Run's budget is instructions.
class CFlat65C64DebugTarget : public CDebugTarget
{
protected:
	CFast65C64		mcMPU;
	uint8*			mpuiMemory;
	uint32			muiMemorySize;

public:
	void			Init(uint32 uiMemorySize);
	void			Kill(void);

	CFast65C64*		GetMPU(void);
	uint8*			GetMemory(void);

	const char*		GetProcessorName(void) override;
	size			NumAddressBits(void) override;
	size			NumRegisters(void) override;
	void			GetRegister(size uiRegister, SDebugRegister* psDest) override;
	uint64			GetRegisterValue(size uiRegister) override;
	bool			SetRegisterValue(size uiRegister, uint64 uiValue) override;

	uint32			GetProgramAddress(void) override;
	void			SetProgramAddress(uint32 uiAddress) override;
	uint32			GetStackPointer(void) override;

	CBreakpoints*	GetBreakpoints(void) override;
	bool			IsAtBreakpoint(void) override;
	bool			IsStopped(void) override;

	void			Run(uint64 uiBudget) override;
	bool			Step(void) override;
	void			Reset(void) override;

	size			PeekMemory(uint32 uiAddress, void* pvDest, size uiSize) override;
	size			PokeMemory(uint32 uiAddress, void* pvSource, size uiSize) override;

protected:
	static uint16	ReadWord(void* pvContext, uint32 uiAddress);
	static void		WriteByte(void* pvContext, uint32 uiAddress, uint8 uiValue);
};


#endif // __FLAT_65C64_DEBUG_TARGET_H__
