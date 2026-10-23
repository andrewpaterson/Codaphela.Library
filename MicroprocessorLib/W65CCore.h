/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __W65C_CORE_H__
#define __W65C_CORE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "W65Address.h"
#include "TraceValue.h"
#include "W65BCDResult.h"
#include "CPUFlags.h"
#include "Timeline.h"
#include "W65CPins.h"


//The machinery and registers the W65C02S and W65C816S share.  The W65C02S runs as a
//W65C816S that is always in emulation mode: 8 bit registers, the stack in page 1, the
//direct (zero) page, data bank and program bank all zero.  Each processor provides its
//instruction set, its own instructions and its own pins.
class CW65Instruction;
class CW65DataOperation;
class CW65BusCycle;
class CW65CCore
{
protected:
	bool		    mbEmulationFlag;

	// Status register
	bool		    mbZeroFlag;
	bool		    mbNegativeFlag;
	bool		    mbDecimalFlag;
	bool		    mbInterruptDisableFlag;
	bool		    mbAccumulatorWidthFlag;
	bool		    mbIndexWidthFlag;
	bool		    mbCarryFlag;
	bool		    mbOverflowFlag;
	bool		    mbBreakFlag;

	// Registers    
	uint16		    muiAccumulator;
	uint16		    muiXIndex;
	uint16		    muiYIndex;
	uint8		    muiDataBank;
	uint16		    muiDirectPage;
	CW65Address	    mcProgramCounter;
	uint16		    muiStackPointer;

	int16		    miCycle;
	int16		    miNextCycle;
	uint16		    muiOpcodeIndex;
	CW65BusCycle*	mpcBusCycle;  //The bus cycle at miCycle of muiOpcodeIndex, see UpdateBusCycle.
	bool		    mbStopped;
	bool		    mbWaiting;
	uint8		    muiEmulationStatusBits;  //Bits of P that read as 1 in emulation mode.

	uint8		    muiAbortProcessRegister;
	uint16		    muiAbortAccumulator;
	uint16		    muiAbortXIndex;
	uint16		    muiAbortYIndex;
	uint8		    muiAbortDataBank;
	uint16		    muiAbortDirectPage;
	CW65Address	    mcAbortProgramCounter;
	uint16		    muiAbortStackPointer;

	//These are not the values on the pins, they are internal data.
	CW65Address	    mcAddress;
	uint16		    muiData;
	uint8		    muiIOData;
	uint16		    muiDirectOffset;
	CW65Address	    mcNewProgramCounter;
	bool		    mbBusEnable;
	bool		    mbIrq;
	bool		    mbAbort;
	bool		    mbNextInstruction;
	bool		    mbNmi;
	bool		    mbPreviousClockLow;
	bool		    mbPreviousClockHigh;
	uint16		    muiResetCount;
    CW65CPins*      mpcPins;

public:
	void			Init(CW65CPins* pcPins);
	void			Kill(void);

    CW65CPins*      GetPins(void);

    void            ASL(void);
    void            ASL_A(void);
    void            PLP(void);
    void            PHP(void);
    void            BRK(void);
    void            ORA(void);
    void            TSB(void);
    void            TRB(void);
    void            BPL(void);
    void            BMI(void);
    void            CLC(void);
    void            INC(void);
    void            INC_A(void);
    void            AND(void);
    void            BIT(void);
    void            BIT_A(void);
    void            ROL(void);
    void            ROL_A(void);
    void            SEC(void);
    void            DEC(void);
    void            DEC_A(void);
    void            EOR(void);
    void            LSR(void);
    void            LSR_A(void);
    void            PHA(void);
    void            BVC(void);
    void            CLI(void);
    void            PHY(void);
    void            ADC(void);
    void            STZ(void);
    void            PLA(void);
    void            ROR(void);
    void            ROR_A(void);
    void            BVS(void);
    void            SEI(void);
    void            PLY(void);
    void            BRA(void);
    void            STA(void);
    void            STY(void);
    void            STX(void);
    void            DEY(void);
    void            TXA(void);
    void            BCC(void);
    void            LDY(void);
    void            LDA(void);
    void            LDX(void);
    void            BCS(void);
    void            CLV(void);
    void            TSX(void);
    void            TYA(void);
    void            TXS(void);
    void            CPY(void);
    void            CMP(void);
    void            TAY(void);
    void            TAX(void);
    void            PHX(void);
    void            STP(void);
    void            INY(void);
    void            DEX(void);
    void            BNE(void);
    void            CLD(void);
    void            CPX(void);
    void            SBC(void);
    void            INX(void);
    void            NOP(void);
    void            BEQ(void);
    void            SED(void);
    void            PLX(void);

    void            ABORT(void);
    void            IRQ(void);
    void            NMI(void);
    void            RES(void);

    void            InputTransition(CTimeline* pcTimeline);// override;

    void            GetAddressValueHex(CChars* psz, bool bAppend0x = true);
    void            GetAccumulatorValueHex(CChars* psz, bool bAppend0x = true);
    void            GetXValueHex(CChars* psz, bool bAppend0x = true);
    void            GetYValueHex(CChars* psz, bool bAppend0x = true);
    void            GetDataBankValueHex(CChars* psz, bool bAppend0x = true);
    void            GetStackValueHex(CChars* psz, bool bAppend0x = true);
    void            GetDirectPageValueHex(CChars* psz, bool bAppend0x = true);
    void            GetProgramCounterValueHex(CChars* psz, bool bAppend0x = true);
    void            GetDataValueHex(CChars* psz, bool bAppend0x = true);
    void            GetOpcodeValueHex(CChars* psz, int cycle, CW65Instruction* pcInstruction, bool bAppend0x = true);
    void            GetOpcodeValueHex(CChars* psz, bool bAppend0x = true);
    void            GetOpcodeMnemonicString(CChars* psz);
    void            GetCycleOperationString(CChars* psz);
    void            GetStatusString(CChars* psz);
    virtual char*   GetType(void) = 0;
    void            GetCycleString(CChars* psz);
    int16           GetCyclePlusOne(void);
    CW65Instruction*   GetInstruction(void);

	bool			IsMemory8Bit(void);
	bool			IsMemory16Bit(void);
	bool			IsIndex8Bit(void);

    void            Branch(bool condition);
    void            To8BitHexString(CChars* psz, uint8 ui8);
    void            To16BitHexString(CChars* psz, uint16 ui16);
    void            ToAddressHexString(CChars* psz, CW65Address* pcAddress, bool bAppend0x = true);
    void            DisableBuses(void);
    void            Append0x(CChars* psz, bool bAppend0x);
	void			ToAddressOperationString(CChars* psz);
	void			ToDataOperationString(CChars* psz);
	void			ToInstructionDescription(CChars* psz);

    void            ExecutPhi2Falling(CTimeline* pcTimeline);
    void            ExecutPhi2Rising(CTimeline* pcTimeline);
	CW65Address*		GetAddress(void);


	//Provided by each processor.
	virtual CW65Instruction*	GetInstruction(uint16 uiOpcode) = 0;
	virtual CW65Instruction*	GetResetOpcode(void) = 0;
	virtual CW65Instruction*	GetIrqOpcode(void) = 0;
	virtual CW65Instruction*	GetNmiOpcode(void) = 0;
	virtual CW65Instruction*	GetAbortOpcode(void) = 0;
	virtual CW65Instruction*	GetFetchNextOpcode(void) = 0;
	virtual void			WriteSignalsPhi2Falling(CTimeline* pcTimeline, CW65DataOperation* pcDataOperation, CW65BusCycle* pcBusCycle) = 0;
	virtual void			WriteSignalsPhi2Rising(CTimeline* pcTimeline);
	virtual bool			IsReadCycle(CW65DataOperation* pcDataOperation);


	void			CreateAbortValues(void);
	void			CreatePartialAbortValues(void);
	void			RestoreAbortValues(void);
	void			ResetPulled(void);
	void			IncrementReset(void);
	bool			IsReseting(STraceValue clockValue);
	void			ClearReset(void);

	uint8			GetProcessorRegisterValue(void);
	uint8			GetPushedProcessorRegisterValue(bool bBreak);
	void			SetZeroFlag(bool bZeroFlag);
	void			SetNegativeFlag(bool bSignFlag);
	void			SetDecimalFlag(bool bDecimalFlag);
	void			SetInterruptDisableFlag(bool bInterruptDisableFlag);
	void			SetAccumulatorWidthFlag(bool bAccumulatorWidthFlag);
	void			SetIndexWidthFlag(bool bIndexWidthFlag);
	void			SetCarryFlag(bool bCarryFlag);
	void			SetEmulationFlag(bool bEmulationFlag);
	bool			IsZeroFlagSet(void);
	bool			IsNegativeSet(void);
	bool			IsDecimal(void);
	bool			IsInterruptDisable(void);
	bool			IsCarrySet(void);
	bool			IsEmulation(void);
	bool			IsBreak(void);
	bool			IsOverflowFlag(void);
	void			SetBreakFlag(bool bBreakFlag);
	void			SetOverflowFlag(bool bOverflowFlag);
	void			Cycle(CW65CCore* pcCPU);

	void			ExecuteInitialSideOperation(CW65CCore* pcCPU);
	void			ExecuteTrailingSideOperation(CW65CCore* pcCPU);

	CW65BusCycle*		GetBusCycle(void);
	void			UpdateBusCycle(void);
	void			NextInstruction(void);
	void			DoneInstruction(void);
	void			NextCycle(void);
	int16			GetCycle(void);
	void			SetOpcode(uint16 uiOpcodeIndex);
	CW65DataOperation* GetDataOperation(void);
	bool			IsStopped(void);
	bool			IsRead(void);
	void			SetX(uint16 uiXIndex);
	void			SetY(uint16 uiYIndex);
	void			SetA(uint16 uiAccumulator);
	void			SetC(uint16 uiAccumulator);
	void			SetMemoryData(uint16 uiData, bool bUpdateFlags);
	void			SetIndexData(uint16 uiData, bool bUpdateFlags);
	void			SetSignAndZeroFromMemory(uint16 uiValue);
	void			SetSignAndZeroFromIndex(uint16 uiValue);
	void			SetMemoryData(uint16 uiData);
	uint16			GetA(void);
	uint16			GetC(void);
	uint16			GetX(void);
	uint16			GetY(void);
	uint8			GetDataBank(void);
	void			SetDataBank(uint8 uiDataBank);
	void			SetDirectPage(uint16 uiDirectPage);
	CW65Address*		GetProgramCounter(void);
	void			SetProcessorRegisterValue(uint8 uiValue);
	void			SetZeroFlagFrom8BitValue(uint16 uiValue);
	void			SetZeroFlagFrom16BitValue(uint16 uiValue);
	void			SetNegativeFlagFrom8BitValue(uint16 uiValue);
	void			SetNegativeFlagFrom16BitValue(uint16 uiValue);
	void			SetSignAndZeroFlagFrom8BitValue(uint16 uiValue);
	void			SetSignAndZeroFlagFrom16BitValue(uint16 uiValue);
	bool			Is8bitValueNegative(uint16 uiValue);
	bool			Is16bitValueNegative(uint16 uiValue);
	bool			Is8bitValueZero(uint16 uiValue);
	bool			Is16bitValueZero(uint16 uiValue);
	bool			IsMemoryNegative(uint16 uiOperand);
	uint16			TrimMemory(uint16 uiValue);
	uint16			TrimIndex(uint16 uiValue);
	bool			IsIndex16Bit(void);
	uint16			GetCarry(void);
	bool			IsSignSet(void);
	bool			IsOverflowSet(void);
	void			SetProgramCounter(CW65Address* pcAddress);
	void			SetProgramAddressBank(uint8 uibank);
	void			SetAddressLow(uint8 uiAddressLow);
	void			SetAddressHigh(uint8 uiAddressHigh);
	void			SetAddressBank(uint8 uiAddressBank);
	uint8			GetDataLow(void);
	uint8			GetDataHigh(void);
	uint8			GetIOData(void);
	uint16			GetMemoryData(void);
	uint16			GetIndexData(void);
	uint16			GetData16Bit(void);
	uint16			GetDirectPage(void);
	uint16			GetDirectOffset(void);
	uint16			GetStackPointer(void);
	uint16			GetStackPointer16Bit(void);

	CW65Instruction*	GetOpcode(void);
	CW65Address*		GetNewProgramCounter(void);
	void			IncrementProgramAddress(void);
	void			DecrementProgramCounter(void);
	void			IncrementStackPointer(void);
	void			DecrementStackPointer(void);
	void			SetDirectOffset(uint8 uiData);
	void			SetDataLow(uint8 uiData);
	void			SetDataHigh(uint8 uiData);
	void			SetIOData(uint8 uiData);
	void			SetStackPointer(uint16 uiData);
	void			SetNewProgramCounterLow(uint8 uiData);
	void			SetNewProgramCounterHigh(uint8 uiData);
	void			SetNewProgramCounterBank(uint8 uiData);
	void			SetEmulationMode(bool bEmulation);
	void			ProcessorStatusChanged(void);
	void			Execute8BitADC(void);
	void			Execute16BitADC(void);
	void			Execute8BitBCDADC(void);
	void			Execute16BitBCDADC(void);
	void			Execute8BitSBC(void);
	void			Execute16BitSBC(void);
	virtual void	Execute8BitBCDSBC(void);
	void			Execute16BitBCDSBC(void);
	CW65BCDResult		BCDAdd8Bit(uint16 uiBCDFirst, uint16 uiBCDSecond, bool bCarry);
	CW65BCDResult		BCDAdd16Bit(uint16 uiBCDFirst, uint16 uiBCDSecond, bool bCarry);
	CW65BCDResult		BCDSubtract8Bit(uint16 uiBCDFirst, uint16 uiBCDSecond, bool bBorrow);
	CW65BCDResult		BCDSubtract16Bit(uint16 uiBCDFirst, uint16 uiBCDSecond, bool bBorrow);
	uint16			SetBit(uint16 uiValue, bool bBitValue, uint16 uiBitNumber);
	uint16			ClearBit(uint16 uiValue, uint16 uiBitNumber);
	uint16			SetBit(uint16 uiValue, uint16 uiBitNumber);
	bool			IsBusEnable(void);
	uint16			RotateLeft(uint16 uiValue);
	uint16			RotateRight(uint16 uiValue);
	uint16			ShiftRight(uint16 uiValue);
	void			SoftBreak(void);
	void			RTIStatus(void);
	void			ExecuteDecimal(int iNibbles, bool bSubtract);
	void			RTIStatusIfEmulation(void);
	void			Wait(void);
	bool			IsWaiting(void);
	void			Reset(void);
	void			DoneIfIndex8Bit(void);
	void			DoneIfIndex16Bit(void);
	void			DoneIfMemory8Bit(void);
	void			DoneIfMemory16Bit(void);
	void			WriteProcessorStatus(bool bBreak);
	void			ReadProcessorStatus(void);
	uint16			GetData16BitOffset(void);
	uint16			GetData8BitOffset(void);
	void			WriteDataLow(void);
	void			WriteDataHigh(void);
	void			ReadOpcode(void);
	bool			NoteFour(bool bIndexY, bool bNextWillRead);
	bool			NoteSix(void);
	uint16			GetAddressOffsetLowX(void);
	uint16			GetAddressOffsetLowY(void);
	void			WriteProgramBank(void);
	void			WriteProgramCounterHigh(void);
	void			WriteProgramCounterLow(void);

	//Only for tests.  They set registers directly, bypassing reset and instructions.
	void			TestSetC(uint16 uiAccumulator);
	void			TestSetX(uint16 uiXIndex);
	void			TestSetY(uint16 uiYIndex);
	void			TestSetStackPointer(uint16 uiStackPointer);
	void			TestSetProgramCounter(uint8 uiProgramBank, uint16 uiProgramCounter);
	void			TestSetProcessorStatus(uint8 uiProcessorStatus, bool bEmulation);
	void			TestStartInstruction(void);
};



#endif //  __W65C_CORE_H__

