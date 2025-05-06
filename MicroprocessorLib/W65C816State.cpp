#include "InstructionFactory.h"
#include "W65C816State.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816State::Init(void)
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
void CW65C816State::Init(CW65C816State state)
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
CInstruction* CW65C816State::GetInstruction(uint16 uiOpcode)
{
	return CInstructionFactory::GetInstance()->GetInstruction(uiOpcode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CW65C816State::GetResetOpcode(void)
{
	return CInstructionFactory::GetInstance()->GetReset();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CW65C816State::GetIrqOpcode(void)
{
	return CInstructionFactory::GetInstance()->GetIRQ();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CW65C816State::GetNmiOpcode(void)
{
	return CInstructionFactory::GetInstance()->GetNMI();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CW65C816State::GetAbortOpcode(void)
{
	return CInstructionFactory::GetInstance()->GetAbort();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CW65C816State::GetFetchNextOpcode(void)
{
	return CInstructionFactory::GetInstance()->GetFetchNext();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816State::CreateAbortValues(void)
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
void CW65C816State::CreatePartialAbortValues(void)
{
    muiAbortProcessRegister = GetProcessorRegisterValue();
    muiAbortDataBank = muiDataBank;
    mcAbortProgramCounter.Init(mcProgramCounter.GetBank(), mcAbortProgramCounter.GetOffset());
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816State::RestoreAbortValues(void)
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
void CW65C816State::ResetPulled(void)
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
uint8 CW65C816State::GetProcessorRegisterValue(void)
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
void CW65C816State::ResetPulled(void)
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
void CW65C816State::ResetPulled(void)
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
void CW65C816State::SetZeroFlag(bool bZeroFlag)
{
    mbZeroFlag = bZeroFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816State::SetNegativeFlag(bool bSignFlag)
{
    mbNegativeFlag = bSignFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816State::SetDecimalFlag(bool bDecimalFlag)
{
    mbDecimalFlag = bDecimalFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816State::SetInterruptDisableFlag(bool bInterruptDisableFlag)
{
    mbInterruptDisableFlag = bInterruptDisableFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816State::SetAccumulatorWidthFlag(bool bAccumulatorWidthFlag)
{
    mbAccumulatorWidthFlag = bAccumulatorWidthFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816State::SetIndexWidthFlag(bool bIndexWidthFlag)
{
    mbIndexWidthFlag = bIndexWidthFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816State::SetCarryFlag(bool bCarryFlag)
{
    mbCarryFlag = bCarryFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816State::SetEmulationFlag(bool bEmulationFlag)
{
    mbEmulationFlag = bEmulationFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CW65C816State::IsZeroFlagSet(void)
{
    return mbZeroFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CW65C816State::IsNegativeSet(void)
{
    return mbNegativeFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CW65C816State::IsDecimal(void)
{
    return mbDecimalFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CW65C816State::IsInterruptDisable(void)
{
    return mbInterruptDisableFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CW65C816State::IsCarrySet(void)
{
    return mbCarryFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CW65C816State::IsEmulation(void)
{
    return mbEmulationFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CW65C816State::IsBreak(void)
{
    return mbBreakFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CW65C816State::IsOverflowFlag(void)
{
    return mbOverflowFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816State::SetBreakFlag(bool bBreakFlag)
{
    mbBreakFlag = bBreakFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816State::SetOverflowFlag(bool bOverflowFlag)
{
    mbOverflowFlag = bOverflowFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816State::Cycle(CW65C816* pcCPU)
{
    if (mbNextInstruction)
    {
        miCycle = -1;
        mbNextInstruction = false;
        NextInstruction();
    }

    NextCycle();
    while (!GetBusCycle()->MustExecute(pcCPU))
    {
        NextCycle();
    }
}

