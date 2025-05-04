#include "InstructionFactory.h"
#include "W65C816State.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void W65C816State::Init(void)
{
    mcProgramCounter.Init();
    muiStackPointer = 0x01FF;
    muiAccumulator = 0;
    muiXIndex = 0;
    muiYIndex = 0;
    muiDataBank = 0;
    muiDirectPage = 0;

    mbZeroFlag = false;
    mbNegativeFlag = false;
    mbDecimalFlag = false;
    mbInterruptDisableFlag = false;
    mbAccumulatorWidthFlag = false;
    mbIndexWidthFlag = false;
    mbCarryFlag = false;
    mbEmulationFlag = true;
    mbOverflowFlag = false;
    mbBreakFlag = false;

    mbStopped = false;
    mbBusEnable = true;
    mbNextInstruction = false;

    muiData = 0;
    miCycle = 0;
    muiOpCodeIndex = GetResetOpcode()->GetCode();
    muiInternal16BitData = 0;
    muiDirectOffset = 0;
    mcNewProgramCounter.Init();
    mcAddress.Init();
    mbNmi = false;

    CreateAbortValues();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void W65C816State::Init(W65C816State state)
{
    mbZeroFlag = state.mbZeroFlag;
    mbNegativeFlag = state.mbNegativeFlag;
    mbDecimalFlag = state.mbDecimalFlag;
    mbInterruptDisableFlag = state.mbInterruptDisableFlag;
    mbAccumulatorWidthFlag = state.mbAccumulatorWidthFlag;
    mbIndexWidthFlag = state.mbIndexWidthFlag;
    mbCarryFlag = state.mbCarryFlag;
    mbEmulationFlag = state.mbEmulationFlag;
    mbOverflowFlag = state.mbOverflowFlag;
    mbBreakFlag = state.mbBreakFlag;
    muiAccumulator = state.mbAccumulatorWidthFlag;
    muiXIndex = state.muiXIndex;
    muiYIndex = state.muiYIndex;
    muiDataBank = state.muiDataBank;
    muiDirectPage = state.muiDirectPage;
    mcProgramCounter.Init(&state.mcProgramCounter);
    muiStackPointer = state.muiStackPointer;
    miCycle = state.miCycle;
    muiOpCodeIndex = state.muiOpCodeIndex;
    mbStopped = state.mbStopped;
    muiAbortProcessRegister = state.muiAbortProcessRegister;
    muiAbortAccumulator = state.muiAbortAccumulator;
    muiAbortXIndex = state.muiAbortXIndex;
    muiAbortYIndex = state.muiAbortYIndex;
    muiAbortDataBank = state.muiAbortDataBank;
    muiAbortDirectPage = state.muiAbortDirectPage;
    mcAbortProgramCounter.Init(&state.mcAbortProgramCounter);
    muiAbortStackPointer = state.muiAbortStackPointer;
    mcAddress.Init(&state.mcAddress);
    muiInternal16BitData = state.muiInternal16BitData;
    muiDirectOffset = state.muiDirectOffset;
    mcNewProgramCounter.Init(&state.mcNewProgramCounter);
    mbBusEnable = state.mbBusEnable;
    mbIrq = state.mbIrq;
    mbAbort = state.mbAbort;
    mbNextInstruction = state.mbNextInstruction;
    muiData = state.muiData;
    mbNmi = state.mbNmi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* W65C816State::GetOpCodeTable(void)
{
	return CInstructionFactory::GetInstance()->GetInstructions();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* W65C816State::GetResetOpcode(void)
{
	return CInstructionFactory::GetInstance()->GetReset();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* W65C816State::GetIrqOpcode(void)
{
	return CInstructionFactory::GetInstance()->GetIRQ();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* W65C816State::GetNmiOpcode(void)
{
	return CInstructionFactory::GetInstance()->GetNMI();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* W65C816State::GetAbortOpcode(void)
{
	return CInstructionFactory::GetInstance()->GetAbort();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* W65C816State::GetFetchNextOpcode(void)
{
	return CInstructionFactory::GetInstance()->GetFetchNext();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void W65C816State::CreateAbortValues(void)
{
    muiAbortProcessRegister = GetProcessorRegisterValue();
    muiAbortAccumulator = muiAccumulator;
    muiAbortXIndex = muiXIndex;
    muiAbortYIndex = muiYIndex;
    muiAbortDataBank = muiDataBank;
    muiAbortDirectPage = muiDirectPage;
    mcAbortProgramCounter.Init(&mcProgramCounter);
    muiAbortStackPointer = muiStackPointer;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void W65C816State::CreatePartialAbortValues(void)
{
    muiAbortProcessRegister = GetProcessorRegisterValue();
    muiAbortDataBank = muiDataBank;
    mcAbortProgramCounter.Init(mcProgramCounter.GetBank(), mcAbortProgramCounter.GetOffset());
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void W65C816State::RestoreAbortValues(void)
{
    muiAbortProcessRegister = GetProcessorRegisterValue();
    muiAccumulator = muiAbortAccumulator;
    muiXIndex = muiAbortXIndex;
    muiYIndex = muiAbortYIndex;
    muiDataBank = muiAbortDataBank;
    muiDirectPage = muiAbortDirectPage;
    mcProgramCounter.Init(&mcAbortProgramCounter);
    muiStackPointer = muiAbortStackPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void W65C816State::ResetPulled(void)
{
    mbAbort = false;
    mbNmi = false;
    muiOpCodeIndex = GetResetOpcode()->GetCode();
    mbStopped = false;
    miCycle = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 W65C816State::GetProcessorRegisterValue(void)
{
    int value = 0;
    if (IsCarrySet())
    {
        value |= STATUS_CARRY;
    }
    if (IsZeroFlagSet())
    {
        value |= STATUS_ZERO;
    }
    if (IsInterruptDisable())
    {
        value |= STATUS_INTERRUPT_DISABLE;
    }
    if (IsDecimal())
    {
        value |= STATUS_DECIMAL;
    }
    if (mbEmulationFlag && mbBreakFlag)
    {
        value |= STATUS_BREAK;
    }
    if (!mbEmulationFlag && mbIndexWidthFlag)
    {
        value |= STATUS_INDEX_WIDTH;
    }
    if (!mbEmulationFlag && mbAccumulatorWidthFlag)
    {
        value |= STATUS_ACCUMULATOR_WIDTH;
    }
    if (IsOverflowFlag())
    {
        value |= STATUS_OVERFLOW;
    }
    if (IsNegativeSet())
    {
        value |= STATUS_NEGATIVE;
    }

    return value;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void W65C816State::ResetPulled(void)
{
    mbAbort = false;
    mbNmi = false;
    muiOpCodeIndex = GetResetOpcode()->GetCode();
    mbStopped = false;
    miCycle = 0;
}

