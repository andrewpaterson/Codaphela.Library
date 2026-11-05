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
#ifndef __FLAT_65C816_DEBUG_TARGET_H__
#define __FLAT_65C816_DEBUG_TARGET_H__
#include "MicroprocessorLib/Fast65C816.h"
#include "DebugTarget.h"
#include "W65C816DebugRegisters.h"


//A CFast65C816 with nothing but memory: a flat block at 00:0000, uiMemorySize bytes.  Reads past it are $FF and
//writes are ignored.  Run's budget is instructions.
class CFlat65C816DebugTarget : public CDebugTarget
{
protected:
	CFast65C816		mcMPU;
	uint8*			mpuiMemory;
	uint32			muiMemorySize;

public:
	void			Init(uint32 uiMemorySize);
	void			Kill(void);

	CFast65C816*	GetMPU(void);
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
	EDebugInstruction	GetInstruction(uint32 uiAddress, size* puiCallLength) override;

protected:
	static uint8	ReadByte(void* pvContext, uint32 uiAddress);
	static void		WriteByte(void* pvContext, uint32 uiAddress, uint8 uiValue);
};


#endif // __FLAT_65C816_DEBUG_TARGET_H__
