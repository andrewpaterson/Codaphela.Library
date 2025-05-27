#ifndef __W65C816_State_H__
#define __W65C816_State_H__
#include "BaseLib/PrimitiveTypes.h"
#include "Address.h"
#include "BCDResult.h"
#include "DataOperation.h"
#include "Instruction.h"
#include "CPUFlags.h"
#include "BusCycle.h"
#include "IntUtil.h"


class CW65C816;
class CW65C816State : public CUnknown
{
public:
	bool		mbEmulationFlag;

	// Status register
	bool		mbZeroFlag;
	bool		mbNegativeFlag;
	bool		mbDecimalFlag;
	bool		mbInterruptDisableFlag;
	bool		mbAccumulatorWidthFlag;
	bool		mbIndexWidthFlag;
	bool		mbCarryFlag;
	bool		mbOverflowFlag;
	bool		mbBreakFlag;

	// Registers
	uint16		muiAccumulator;
	uint16		muiXIndex;
	uint16		muiYIndex;
	uint8		muiDataBank;
	uint16		muiDirectPage;
	CAddress	mcProgramCounter;
	uint16		muiStackPointer;

	int16		miCycle;
	int16		miNextCycle;
	uint16		muiOpCodeIndex;
	bool		mbStopped;

	uint8		muiAbortProcessRegister;
	uint16		muiAbortAccumulator;
	uint16		muiAbortXIndex;
	uint16		muiAbortYIndex;
	uint8		muiAbortDataBank;
	uint16		muiAbortDirectPage;
	CAddress	mcAbortProgramCounter;
	uint16		muiAbortStackPointer;

	//These are not the values on the pins, they are internal data.
	CAddress	mcAddress;
	uint16		muiInternal16BitData;
	uint16		muiDirectOffset;
	CAddress	mcNewProgramCounter;
	bool		mbBusEnable;
	bool		mbIrq;
	bool		mbAbort;
	bool		mbNextInstruction;
	uint8		muiData;
	bool		mbNmi;
	bool		mbPreviousClockLow;
	bool		mbPreviousClockHigh;

public:
	void			Init(void);
	void			Init(CW65C816State state);
		
	CInstruction*   GetInstruction(uint16 uiOpcode);
	CInstruction*   GetResetOpcode(void);
	CInstruction*   GetIrqOpcode(void);
	CInstruction*   GetNmiOpcode(void);
	CInstruction*   GetAbortOpcode(void);
	CInstruction*   GetFetchNextOpcode(void);

	void			CreateAbortValues(void);
	void			CreatePartialAbortValues(void);
	void			RestoreAbortValues(void);
	void			ResetPulled(void);
	uint8			GetProcessorRegisterValue(void);
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
	void			Cycle(CW65C816* pcCPU);

	void			ExecuteInitialSideOperation(CW65C816* pcCPU);
	void			ExecuteTrailingSideOperation(CW65C816* pcCPU);

	CBusCycle*		GetBusCycle(void);
	void			NextInstruction(void);
	void			DoneInstruction(void);
	void			NextCycle(void);
	int16			GetCycle(void);
	void			SetOpCode(uint16 uiOpCodeIndex);
	CDataOperation* GetDataOperation(void);
	bool			IsStopped(void);
	bool			IsRead(void);
	bool			IsMemory8Bit(void);
	bool			IsMemory16Bit(void);
	bool			IsIndex8Bit(void);
	void			SetX(uint16 uiXIndex);
	void			SetY(uint16 uiYIndex);
	void			SetA(uint16 uiAccumulator);
	void			SetC(uint16 uiAccumulator);
	void			SetInternal16BitData(uint16 uiData, bool bUpdateFlags);
	void			SetIndexData(uint16 uiData, bool bUpdateFlags);
	void			SetSignAndZeroFromMemory(uint16 uiValue);
	void			SetSignAndZeroFromIndex(uint16 uiValue);
	void			SetInternal16BitData(uint16 uiInternal16BitData);
	uint16			GetA(void);
	uint16			GetC(void);
	uint16			GetX(void);
	uint16			GetY(void);
	uint8			GetDataBank(void);
	void			SetDataBank(uint8 uiDataBank);
	void			SetDirectPage(uint16 uiDirectPage);
	CAddress*		GetProgramCounter(void);
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
	void			BlockMoveNext(void);
	void			BlockMovePrevious(void);
	uint16			TrimMemory(uint16 uiValue);
	uint16			TrimIndex(uint16 uiValue);
	bool			IsIndex16Bit(void);
	uint16			GetCarry(void);
	bool			IsSignSet(void);
	bool			IsOverflowSet(void);
	void			SetProgramCounter(CAddress* pcAddress);
	void			SetProgramAddressBank(uint8 uibank);
	void			SetAddressLow(uint8 uiAddressLow);
	void			SetAddressHigh(uint8 uiAddressHigh);
	void			SetAddressBank(uint8 uiAddressBank);
	uint8			GetDataLow(void);
	uint8			GetDataHigh(void);
	uint16			GetInternal16BitData(void);
	uint16			GetIndexData(void);
	uint16			GetData16Bit(void);
	uint16			GetDirectPage(void);
	uint16			GetDirectOffset(void);
	uint16			GetStackPointer(void);
	CAddress*		GetAddress(void);

	CInstruction*	GetOpCode(void);
	CAddress*		GetNewProgramCounter(void);
	void			IncrementProgramAddress(void);
	void			DecrementProgramCounter(void);
	void			IncrementStackPointer(void);
	void			DecrementStackPointer(void);
	void			SetDirectOffset(uint8 uiData);
	void			SetInternal16BitDataLow(uint8 uiData);
	void			SetInternal16BitDataHigh(uint8 uiData);
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
	void			Execute8BitBCDSBC(void);
	void			Execute16BitBCDSBC(void);
	CBCDResult		BCDAdd8Bit(uint16 uiBCDFirst, uint16 uiBCDSecond, bool bCarry);
	CBCDResult		BCDAdd16Bit(uint16 uiBCDFirst, uint16 uiBCDSecond, bool bCarry);
	CBCDResult		BCDSubtract8Bit(uint16 uiBCDFirst, uint16 uiBCDSecond, bool bBorrow);
	CBCDResult		BCDSubtract16Bit(uint16 uiBCDFirst, uint16 uiBCDSecond, bool bBorrow);
	uint16			SetBit(uint16 uiValue, bool bBitValue, uint16 uiBitNumber);
	uint16			ClearBit(uint16 uiValue, uint16 uiBitNumber);
	uint16			SetBit(uint16 uiValue, uint16 uiBitNumber);
	bool			IsBusEnable(void);
	uint8			GetData(void);
	void			SetData(uint8 uiData);
	void			PER(void);
	void			PHD(void);
	void			PLP(void);
	void			SoftBreak(void);
	void			ORA(void);
	void			TSB(void);
	void			TRB(void);
	void			INC_A(void);
	void			INX(void);
	void			INY(void);
	void			DEC_A(void);
	void			DEY(void);
	void			DEX(void);
	uint16			RotateLeft(uint16 uiValue);
	uint16			RotateRight(uint16 uiValue);
	uint16			ShiftRight(uint16 uiValue);
	void			BIT(void);
	void			BIT_A(void);
	void			ROR(void);
	void			ROR_A(void);
	void			ROL(void);
	void			ROL_A(void);
	void			LSR(void);
	void			LSR_A(void);
	void			ASL(void);
	void			ASL_A(void);
	void			TSC(void);
	void			EOR(void);
	void			TCD(void);
	void			ADC(void);
	void			TXA(void);
	void			LDA(void);
	void			LDY(void);
	void			LDX(void);
	void			TSX(void);
	void			TYA(void);
	void			TXS(void);
	void			CPY(void);
	void			CMP(void);
	void			REP(void);
	void			TAY(void);
	void			TAX(void);
	void			DEC(void);
	void			INC(void);
	void			CPX(void);
	void			SBC(void);
	void			SEP(void);
	void			XBA(void);
	void			XCE(void);
	void			Reset(void);
	void			TCS(void);
	void			AND(void);
	void			DoneIfIndex8Bit(void);
	void			DoneIfIndex16Bit(void);
	void			DoneIfMemory8Bit(void);
	void			DoneIfMemory16Bit(void);
	void			WriteProcessorStatus(void);
	void			ReadProcessorStatus(void);
	uint16			GetData16BitOffset(void);
	uint16			GetData8BitOffset(void);
	void			WriteDataLow(void);
	void			WriteDataHigh(void);
	void			ReadOpCode(void);
	bool			NoteFourX(bool bNextWillRead);
	bool			NoteFourY(bool bNextWillRead);
	bool			NoteSix(void);
	uint16			GetAddressOffsetX(void);
	uint16			GetAddressOffsetY(void);
	void			WriteProgramBank(void);
	void			WriteProgramCounterHigh(void);
	void			WriteProgramCounterLow(void);
};


#endif // __W65C816_State_H__

