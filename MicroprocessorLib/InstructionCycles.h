#ifndef __INSTRUCTION_CYCLES_H__
#define __INSTRUCTION_CYCLES_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Malloc.h"
#include "ProgramCounter.h"
#include "StackPointer.h"
#include "BusCycleArray.h"
#include "AddressingMode.h"


typedef bool tNotLock;
extern tNotLock gtRMW;


class CInstructionCycles
{
protected:
    CBusCycleArray      mapcCycles;
    EAddressingMode     meAddressingMode;

public:
    void Init(EAddressingMode eAddressingMode, CBusCycleArray* papcCycles);

private:
    void Validate(void);

protected:
    void ValidateDoneOperation(void);

public:
    static CProgramCounter* PC(void);

    static CStackPointer* S(void);

    static CAddressOffsetArray* Address(CAddressOffset* pcOffset, ...);

    static CWriteDataHigh Write_DataHigh()
    {
        return Write_DataHigh(true);
    }

    static CWriteDataHigh Write_DataHigh(bool notMemoryLock)
    {
        return new WriteDataHigh(notMemoryLock);
    }

    static CProgramBank PBR()
    {
        return new ProgramBank();
    }

    static CAbsoluteAddress AA()
    {
        return new AbsoluteAddress();
    }

    static CXIndex X()
    {
        return new XIndex();
    }

    static CYIndex Y()
    {
        return new YIndex();
    }

    static CConstantOffset o(int offset)
    {
        return new ConstantOffset(offset);
    }

    static CDataBank DBR()
    {
        return new DataBank();
    }

    static CAbsoluteAddressHigh AAH()
    {
        return new AbsoluteAddressHigh();
    }

    static CInterruptAddress VA(InterruptVector interruptVector)
    {
        return new InterruptAddress(interruptVector);
    }

    static CDirectOffset DirectOffset()
    {
        return new DirectOffset();
    }

    static CDirectPage DP()
    {
        return new DirectPage();
    }

    static CNewProgramCounter New_PC()
    {
        return new NewProgramCounter();
    }

    static CNewProgramBank New_PBR()
    {
        return new NewProgramBank();
    }

    static CAddressBank AAB()
    {
        return new AddressBank();
    }

    static CInternalFirst OpCode()
    {
        return new InternalFirst();
    }

    static CIncrementProgramCounter PC_inc()
    {
        return new IncrementProgramCounter();
    }

    static CDecrementStackPointer SP_dec()
    {
        return new DecrementStackPointer();
    }

    static CIncrementStackPointer SP_inc()
    {
        return new IncrementStackPointer();
    }

    static CWriteProgramBank Write_PBR()
    {
        return new WriteProgramBank();
    }

    static CReadNewProgramCounterHigh Read_NewPCH()
    {
        return new ReadNewProgramCounterHigh(true);
    }

    static CInternalOperation IO()
    {
        return new InternalOperation(true);
    }

    static CSetProgramCounter PC_e(AddressOffset... addressOffsets)
    {
        return new SetProgramCounter(addressOffsets);
    }

    static CWriteProgramCounterLow Write_PCL()
    {
        return new WriteProgramCounterLow();
    }

    static CWriteProgramCounterHigh Write_PCH()
    {
        return new WriteProgramCounterHigh();
    }

    static CReadAbsoluteAddressHigh Read_AAH()
    {
        return new ReadAbsoluteAddressHigh(true, true);
    }

    static CReadAbsoluteAddressHigh Read_AAH(bool notMemoryLock)
    {
        return new ReadAbsoluteAddressHigh(notMemoryLock, true);
    }

    static CReadNewProgramBank Read_NewPBR()
    {
        return new ReadNewProgramBank(true);
    }

    static CReadNewProgramCounterLow Read_NewPCL()
    {
        return new ReadNewProgramCounterLow(true);
    }

    static CReadAbsoluteAddressLow Read_AAL()
    {
        return new ReadAbsoluteAddressLow(true, true);
    }

    static CReadAbsoluteAddressLow Read_AAVL()
    {
        return new ReadAbsoluteAddressLow(true, false);
    }

    static CReadAbsoluteAddressHigh Read_AAVH()
    {
        return new ReadAbsoluteAddressHigh(true, false);
    }

    static CAbsoluteAddressLowPlusXLow AAL_XL()
    {
        return new AbsoluteAddressLowPlusXLow();
    }

    static CAbsoluteAddressLowPlusYLow AAL_YL()
    {
        return new AbsoluteAddressLowPlusYLow();
    }

    static CDirectOffset D0()
    {
        return DirectOffset();
    }

    static CReadDirectOffset Read_D0()
    {
        return new ReadDirectOffset(true);
    }

    static CReadAbsoluteAddressBank Read_AAB()
    {
        return new ReadAbsoluteAddressBank(true);
    }

    static CReadDataBank Read_DBR()
    {
        return new ReadDataBank(true);
    }

    static CWriteProcessorStatus Write_PS()
    {
        return new WriteProcessorStatus();
    }

    static CReadProcessorStatus Read_PS()
    {
        return new ReadProcessorStatus();
    }

    static CReadAbsoluteAddressLow Read_AAL(bool notMemoryLock)
    {
        return new ReadAbsoluteAddressLow(notMemoryLock, true);
    }

    static CInternalOperation IO(bool notMemoryLock)
    {
        return new InternalOperation(notMemoryLock);
    }

    static CReadDataHigh Read_DataHigh()
    {
        return Read_DataHigh(true);
    }

    static CReadDataHigh Read_DataHigh(bool notMemoryLock)
    {
        return new ReadDataHigh(notMemoryLock);
    }

    static CReadDataLow Read_DataLow(bool notMemoryLock)
    {
        return new ReadDataLow(notMemoryLock);
    }

    static CReadDataLow Read_DataLow()
    {
        return Read_DataLow(true);
    }

    static CWriteDataLow Write_DataLow()
    {
        return Write_DataLow(true);
    }

    static CReadDirectOffset Read_D0(bool notMemoryLock)
    {
        return new ReadDirectOffset(notMemoryLock);
    }

    static CWriteDataLow Write_DataLow(bool notMemoryLock)
    {
        return new WriteDataLow(notMemoryLock);
    }

    static CDoneInstruction DONE()
    {
        return new DoneInstruction();
    }

    static CSetProgramBank PBR_e(int bank)
    {
        return new SetProgramBank(bank);
    }

    public:
static WriteAbsoluteAddressLow Write_AAL()
    {
        return new WriteAbsoluteAddressLow();
    }

    public:
static WriteAbsoluteAddressHigh Write_AAH()
    {
        return new WriteAbsoluteAddressHigh();
    }

    static CExecuteIf16Bit E16Bit(Executor<W65C816> consumer, WidthFromRegister width)
    {

        return new ExecuteIf16Bit(consumer, width);
    }

    static CExecuteIf8Bit E8Bit(Executor<W65C816> consumer, WidthFromRegister width)
    {
        return new ExecuteIf8Bit(consumer, width);
    }

    static CExecute E(Executor<W65C816> consumer)
    {
        return new Execute(consumer);
    }

    static COperation DONE16Bit(WidthFromRegister width)
    {
        return new DoneInstructionIf16Bit(width);
    }

    static COperation DONE8Bit(WidthFromRegister width)
    {
        return new DoneInstructionIf8Bit(width);
    }

public:
AddressingMode GetAddressingMode()
    {
        return addressingMode;
    }

CBusCycle* GetBusCycle(int16 miIndex)
    {
        if ((miIndex >= 0) && (miIndex < mapcCycles.size()))
        {
            return mapcCycles.get(miIndex);
        }
        return NULL;
    }

int Size()
    {
        return mapcCycles.NumElements();
    }
};


#endif // __INSTRUCTION_CYCLES_H__

