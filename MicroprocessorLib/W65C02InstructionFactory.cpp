#include "BaseLib/PointerFunctions.h"
#include "W65BRKVector.h"
#include "W65NMIVector.h"
#include "W65IRQVector.h"
#include "W65ResetVector.h"
#include "W65C02InstructionFactory.h"


CW65C02InstructionFactory	gcW65C02InstructionFactory;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65C02InstructionFactory* CW65C02InstructionFactory::GetInstance(void)
{
	return &gcW65C02InstructionFactory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C02InstructionFactory::Init(void)
{
	CW65ResetVector*	pcResetVector;
	CW65IRQVector*		pcIRQVector;
	CW65NMIVector*		pcNMIVector;

	memset(this, 0, sizeof(CW65C02InstructionFactory));
	CreateInstructions();
	muiInstructions = 256;

	pcResetVector = NewMalloc<CW65ResetVector>();
	pcResetVector->Init();
	pcIRQVector = NewMalloc<CW65IRQVector>();
	pcIRQVector->Init();
	pcNMIVector = NewMalloc<CW65NMIVector>();
	pcNMIVector->Init();

	mpcReset = Instruct(CreateStackResetCycles(pcResetVector, &CW65CCore::RES, false), "RES", "Reset the CPU");
	mpcIRQ = Instruct(CreateStackHardwareInterruptCycles(pcIRQVector, &CW65CCore::IRQ, false), "IRQ", "Interrupt request");
	mpcNMI = Instruct(CreateStackHardwareInterruptCycles(pcNMIVector, &CW65CCore::NMI, false), "NMI", "Non-maskable interrupt");
	mpcFetchNext = Instruct(CreateFetchOpcodeCycles(), "OPC", "Fetch Opcode from address in program counter");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C02InstructionFactory::Kill(void)
{
	uint16			i;
	CW65Instruction*	pcInstruction;

	for (i = 0; i < muiInstructions; i++)
	{
		pcInstruction = mapcInstructions[i];
		SafeKill(pcInstruction);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65Instruction* CW65C02InstructionFactory::GetInstruction(uint16 uiOpcode)
{
	if (uiOpcode < W65C02_INSTRUCTIONS_SIZE)
	{
		return mapcInstructions[uiOpcode];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65Instruction* CW65C02InstructionFactory::GetReset(void)
{
	return mpcReset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65Instruction* CW65C02InstructionFactory::GetIRQ(void)
{
	return mpcIRQ;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65Instruction* CW65C02InstructionFactory::GetNMI(void)
{
	return mpcNMI;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65Instruction* CW65C02InstructionFactory::GetFetchNext(void)
{
	return mpcFetchNext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65Instruction* CW65C02InstructionFactory::Instruct(uint16 uiCode, CW65InstructionCycles* pcInstructionCycles, char* szName, char* szDescription)
{
	CW65Instruction*	pcInstruction;

	pcInstruction = NewMalloc<CW65Instruction>();
	pcInstruction->Init(uiCode, pcInstructionCycles, szName, szDescription);
	return pcInstruction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65Instruction* CW65C02InstructionFactory::Instruct(CW65InstructionCycles* pcInstructionCycles, char* szName, char* szDescription)
{
	//The pseudo instructions (reset, interrupts and opcode fetch) follow the 256 opcodes.
	CW65Instruction*	pcInstruction;

	pcInstruction = Instruct(muiInstructions, pcInstructionCycles, szName, szDescription);
	mapcInstructions[muiInstructions] = pcInstruction;
	muiInstructions++;
	return pcInstruction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C02InstructionFactory::CreateInstructions(void)
{
	CW65BRKVector*		pcBRKVector;

	pcBRKVector = NewMalloc<CW65BRKVector>();
	pcBRKVector->Init();

	mapcInstructions[0x00] = Instruct(W65C02_BRK_Interrupt, CreateW65C02StackSoftwareInterruptCycles(pcBRKVector, &CW65CCore::BRK), "BRK", "Force software break interrupt");
	mapcInstructions[0x01] = Instruct(W65C02_ORA_ZeroPageIndexedIndirectWithX, CreateW65C02ZeroPageIndexedIndirectWithXCycles(&CW65CCore::ORA, false), "ORA", "'OR' memory with A; result in A and update NZ");
	mapcInstructions[0x02] = Instruct(W65C02_NOP_02, CreateW65C02NOPCycles(2), "NOP", "No Operation");
	mapcInstructions[0x03] = Instruct(W65C02_NOP_03, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x04] = Instruct(W65C02_TSB_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65CCore::TSB), "TSB", "Test and Set Bit");
	mapcInstructions[0x05] = Instruct(W65C02_ORA_ZeroPage, CreateW65C02ZeroPageCycles(&CW65CCore::ORA, false), "ORA", "'OR' memory with A; result in A and update NZ");
	mapcInstructions[0x06] = Instruct(W65C02_ASL_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65CCore::ASL), "ASL", "Shift memory left 1 bit; result in memory and update NZC");
	mapcInstructions[0x07] = Instruct(W65C02_RMB0_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65C02::RMB), "RMB0", "Reset Memory Bit");
	mapcInstructions[0x08] = Instruct(W65C02_PHP_Stack, CreateW65C02StackPushCycles(&CW65CCore::PHP), "PHP", "Push Processor Status on Stack");
	mapcInstructions[0x09] = Instruct(W65C02_ORA_Immediate, CreateW65C02ImmediateCycles(&CW65CCore::ORA, false), "ORA", "'OR' memory with A; result in A and update NZ");
	mapcInstructions[0x0A] = Instruct(W65C02_ASL_Accumulator, CreateW65C02ImpliedCycles(&CW65CCore::ASL_A), "ASL", "Shift memory left 1 bit; result in memory and update NZC");
	mapcInstructions[0x0B] = Instruct(W65C02_NOP_0B, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x0C] = Instruct(W65C02_TSB_Absolute, CreateW65C02AbsoluteRMWCycles(&CW65CCore::TSB), "TSB", "Test and Set Bit");
	mapcInstructions[0x0D] = Instruct(W65C02_ORA_Absolute, CreateW65C02AbsoluteCycles(&CW65CCore::ORA, false), "ORA", "'OR' memory with A; result in A and update NZ");
	mapcInstructions[0x0E] = Instruct(W65C02_ASL_Absolute, CreateW65C02AbsoluteRMWCycles(&CW65CCore::ASL), "ASL", "Shift memory left 1 bit; result in memory and update NZC");
	mapcInstructions[0x0F] = Instruct(W65C02_BBR0_ZeroPageProgramCounterRelative, CreateW65C02ZeroPageRelativeCycles(), "BBR0", "Branch on Bit Reset");
	mapcInstructions[0x10] = Instruct(W65C02_BPL_ProgramCounterRelative, CreateW65C02RelativeCycles(&CW65CCore::BPL), "BPL", "Branch if Result Plus (N=0)");
	mapcInstructions[0x11] = Instruct(W65C02_ORA_ZeroPageIndirectIndexedWithY, CreateW65C02ZeroPageIndirectIndexedWithYCycles(&CW65CCore::ORA, false), "ORA", "'OR' memory with A; result in A and update NZ");
	mapcInstructions[0x12] = Instruct(W65C02_ORA_ZeroPageIndirect, CreateW65C02ZeroPageIndirectCycles(&CW65CCore::ORA, false), "ORA", "'OR' memory with A; result in A and update NZ");
	mapcInstructions[0x13] = Instruct(W65C02_NOP_13, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x14] = Instruct(W65C02_TRB_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65CCore::TRB), "TRB", "Test and Reset Bit");
	mapcInstructions[0x15] = Instruct(W65C02_ORA_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedCycles(false, &CW65CCore::ORA, false), "ORA", "'OR' memory with A; result in A and update NZ");
	mapcInstructions[0x16] = Instruct(W65C02_ASL_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedWithXRMWCycles(&CW65CCore::ASL), "ASL", "Shift memory left 1 bit; result in memory and update NZC");
	mapcInstructions[0x17] = Instruct(W65C02_RMB1_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65C02::RMB), "RMB1", "Reset Memory Bit");
	mapcInstructions[0x18] = Instruct(W65C02_CLC_Implied, CreateW65C02ImpliedCycles(&CW65CCore::CLC), "CLC", "Clear Carry Flag");
	mapcInstructions[0x19] = Instruct(W65C02_ORA_AbsoluteIndexedWithY, CreateW65C02AbsoluteIndexedWithYCycles(&CW65CCore::ORA, false), "ORA", "'OR' memory with A; result in A and update NZ");
	mapcInstructions[0x1A] = Instruct(W65C02_INC_Accumulator, CreateW65C02ImpliedCycles(&CW65CCore::INC_A), "INC", "Increment accumulator; update NZ");
	mapcInstructions[0x1B] = Instruct(W65C02_NOP_1B, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x1C] = Instruct(W65C02_TRB_Absolute, CreateW65C02AbsoluteRMWCycles(&CW65CCore::TRB), "TRB", "Test and Reset Bit");
	mapcInstructions[0x1D] = Instruct(W65C02_ORA_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXCycles(&CW65CCore::ORA, false), "ORA", "'OR' memory with A; result in A and update NZ");
	mapcInstructions[0x1E] = Instruct(W65C02_ASL_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXRMWCycles(&CW65CCore::ASL, false), "ASL", "Shift memory left 1 bit; result in memory and update NZC");
	mapcInstructions[0x1F] = Instruct(W65C02_BBR1_ZeroPageProgramCounterRelative, CreateW65C02ZeroPageRelativeCycles(), "BBR1", "Branch on Bit Reset");
	mapcInstructions[0x20] = Instruct(W65C02_JSR_Absolute, CreateW65C02AbsoluteJSRCycles(), "JSR", "Jump to new location save return address on Stack");
	mapcInstructions[0x21] = Instruct(W65C02_AND_ZeroPageIndexedIndirectWithX, CreateW65C02ZeroPageIndexedIndirectWithXCycles(&CW65CCore::AND, false), "AND", "Bitwise AND memory with A; result in A and update NZ");
	mapcInstructions[0x22] = Instruct(W65C02_NOP_22, CreateW65C02NOPCycles(2), "NOP", "No Operation");
	mapcInstructions[0x23] = Instruct(W65C02_NOP_23, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x24] = Instruct(W65C02_BIT_ZeroPage, CreateW65C02ZeroPageCycles(&CW65CCore::BIT, false), "BIT", "Bit Test");
	mapcInstructions[0x25] = Instruct(W65C02_AND_ZeroPage, CreateW65C02ZeroPageCycles(&CW65CCore::AND, false), "AND", "Bitwise AND memory with A; result in A and update NZ");
	mapcInstructions[0x26] = Instruct(W65C02_ROL_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65CCore::ROL), "ROL", "Rotate Memory One Bit Left");
	mapcInstructions[0x27] = Instruct(W65C02_RMB2_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65C02::RMB), "RMB2", "Reset Memory Bit");
	mapcInstructions[0x28] = Instruct(W65C02_PLP_Stack, CreateW65C02StackPullCycles(&CW65CCore::PLP), "PLP", "Pull Processor Status from Stack");
	mapcInstructions[0x29] = Instruct(W65C02_AND_Immediate, CreateW65C02ImmediateCycles(&CW65CCore::AND, false), "AND", "Bitwise AND memory with A; result in A and update NZ");
	mapcInstructions[0x2A] = Instruct(W65C02_ROL_Accumulator, CreateW65C02ImpliedCycles(&CW65CCore::ROL_A), "ROL", "Rotate Memory One Bit Left");
	mapcInstructions[0x2B] = Instruct(W65C02_NOP_2B, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x2C] = Instruct(W65C02_BIT_Absolute, CreateW65C02AbsoluteCycles(&CW65CCore::BIT, false), "BIT", "Bit Test");
	mapcInstructions[0x2D] = Instruct(W65C02_AND_Absolute, CreateW65C02AbsoluteCycles(&CW65CCore::AND, false), "AND", "Bitwise AND memory with A; result in A and update NZ");
	mapcInstructions[0x2E] = Instruct(W65C02_ROL_Absolute, CreateW65C02AbsoluteRMWCycles(&CW65CCore::ROL), "ROL", "Rotate Memory One Bit Left");
	mapcInstructions[0x2F] = Instruct(W65C02_BBR2_ZeroPageProgramCounterRelative, CreateW65C02ZeroPageRelativeCycles(), "BBR2", "Branch on Bit Reset");
	mapcInstructions[0x30] = Instruct(W65C02_BMI_ProgramCounterRelative, CreateW65C02RelativeCycles(&CW65CCore::BMI), "BMI", "Branch if Result Minus (N=1)");
	mapcInstructions[0x31] = Instruct(W65C02_AND_ZeroPageIndirectIndexedWithY, CreateW65C02ZeroPageIndirectIndexedWithYCycles(&CW65CCore::AND, false), "AND", "Bitwise AND memory with A; result in A and update NZ");
	mapcInstructions[0x32] = Instruct(W65C02_AND_ZeroPageIndirect, CreateW65C02ZeroPageIndirectCycles(&CW65CCore::AND, false), "AND", "Bitwise AND memory with A; result in A and update NZ");
	mapcInstructions[0x33] = Instruct(W65C02_NOP_33, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x34] = Instruct(W65C02_BIT_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedCycles(false, &CW65CCore::BIT, false), "BIT", "Bit Test");
	mapcInstructions[0x35] = Instruct(W65C02_AND_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedCycles(false, &CW65CCore::AND, false), "AND", "Bitwise AND memory with A; result in A and update NZ");
	mapcInstructions[0x36] = Instruct(W65C02_ROL_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedWithXRMWCycles(&CW65CCore::ROL), "ROL", "Rotate Memory One Bit Left");
	mapcInstructions[0x37] = Instruct(W65C02_RMB3_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65C02::RMB), "RMB3", "Reset Memory Bit");
	mapcInstructions[0x38] = Instruct(W65C02_SEC_Implied, CreateW65C02ImpliedCycles(&CW65CCore::SEC), "SEC", "Set Carry Flag");
	mapcInstructions[0x39] = Instruct(W65C02_AND_AbsoluteIndexedWithY, CreateW65C02AbsoluteIndexedWithYCycles(&CW65CCore::AND, false), "AND", "Bitwise AND memory with A; result in A and update NZ");
	mapcInstructions[0x3A] = Instruct(W65C02_DEC_Accumulator, CreateW65C02ImpliedCycles(&CW65CCore::DEC_A), "DEC", "Decrement accumulator; update NZ");
	mapcInstructions[0x3B] = Instruct(W65C02_NOP_3B, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x3C] = Instruct(W65C02_BIT_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXCycles(&CW65CCore::BIT, false), "BIT", "Bit Test");
	mapcInstructions[0x3D] = Instruct(W65C02_AND_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXCycles(&CW65CCore::AND, false), "AND", "Bitwise AND memory with A; result in A and update NZ");
	mapcInstructions[0x3E] = Instruct(W65C02_ROL_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXRMWCycles(&CW65CCore::ROL, false), "ROL", "Rotate Memory One Bit Left");
	mapcInstructions[0x3F] = Instruct(W65C02_BBR3_ZeroPageProgramCounterRelative, CreateW65C02ZeroPageRelativeCycles(), "BBR3", "Branch on Bit Reset");
	mapcInstructions[0x40] = Instruct(W65C02_RTI_Stack, CreateW65C02StackRTICycles(), "RTI", "Return from Interrupt");
	mapcInstructions[0x41] = Instruct(W65C02_EOR_ZeroPageIndexedIndirectWithX, CreateW65C02ZeroPageIndexedIndirectWithXCycles(&CW65CCore::EOR, false), "EOR", "'Exclusive OR' Memory with Accumulator");
	mapcInstructions[0x42] = Instruct(W65C02_NOP_42, CreateW65C02NOPCycles(2), "NOP", "No Operation");
	mapcInstructions[0x43] = Instruct(W65C02_NOP_43, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x44] = Instruct(W65C02_NOP_44, CreateW65C02NOPZeroPageCycles(), "NOP", "No Operation");
	mapcInstructions[0x45] = Instruct(W65C02_EOR_ZeroPage, CreateW65C02ZeroPageCycles(&CW65CCore::EOR, false), "EOR", "'Exclusive OR' Memory with Accumulator");
	mapcInstructions[0x46] = Instruct(W65C02_LSR_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65CCore::LSR), "LSR", "Shift memory right one bit; update NZC");
	mapcInstructions[0x47] = Instruct(W65C02_RMB4_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65C02::RMB), "RMB4", "Reset Memory Bit");
	mapcInstructions[0x48] = Instruct(W65C02_PHA_Stack, CreateW65C02StackPushCycles(&CW65CCore::PHA), "PHA", "Push Accumulator onto Stack");
	mapcInstructions[0x49] = Instruct(W65C02_EOR_Immediate, CreateW65C02ImmediateCycles(&CW65CCore::EOR, false), "EOR", "'Exclusive OR' Memory with Accumulator");
	mapcInstructions[0x4A] = Instruct(W65C02_LSR_Accumulator, CreateW65C02ImpliedCycles(&CW65CCore::LSR_A), "LSR", "Shift memory right one bit; update NZC");
	mapcInstructions[0x4B] = Instruct(W65C02_NOP_4B, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x4C] = Instruct(W65C02_JMP_Absolute, CreateW65C02AbsoluteJMPCycles(), "JMP", "Jump to New Location");
	mapcInstructions[0x4D] = Instruct(W65C02_EOR_Absolute, CreateW65C02AbsoluteCycles(&CW65CCore::EOR, false), "EOR", "'Exclusive OR' Memory with Accumulator");
	mapcInstructions[0x4E] = Instruct(W65C02_LSR_Absolute, CreateW65C02AbsoluteRMWCycles(&CW65CCore::LSR), "LSR", "Shift memory right one bit; update NZC");
	mapcInstructions[0x4F] = Instruct(W65C02_BBR4_ZeroPageProgramCounterRelative, CreateW65C02ZeroPageRelativeCycles(), "BBR4", "Branch on Bit Reset");
	mapcInstructions[0x50] = Instruct(W65C02_BVC_ProgramCounterRelative, CreateW65C02RelativeCycles(&CW65CCore::BVC), "BVC", "Branch on Overflow Clear (V=0)");
	mapcInstructions[0x51] = Instruct(W65C02_EOR_ZeroPageIndirectIndexedWithY, CreateW65C02ZeroPageIndirectIndexedWithYCycles(&CW65CCore::EOR, false), "EOR", "'Exclusive OR' Memory with Accumulator");
	mapcInstructions[0x52] = Instruct(W65C02_EOR_ZeroPageIndirect, CreateW65C02ZeroPageIndirectCycles(&CW65CCore::EOR, false), "EOR", "'Exclusive OR' Memory with Accumulator");
	mapcInstructions[0x53] = Instruct(W65C02_NOP_53, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x54] = Instruct(W65C02_NOP_54, CreateW65C02NOPZeroPageIndexedCycles(), "NOP", "No Operation");
	mapcInstructions[0x55] = Instruct(W65C02_EOR_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedCycles(false, &CW65CCore::EOR, false), "EOR", "'Exclusive OR' Memory with Accumulator");
	mapcInstructions[0x56] = Instruct(W65C02_LSR_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedWithXRMWCycles(&CW65CCore::LSR), "LSR", "Shift memory right one bit; update NZC");
	mapcInstructions[0x57] = Instruct(W65C02_RMB5_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65C02::RMB), "RMB5", "Reset Memory Bit");
	mapcInstructions[0x58] = Instruct(W65C02_CLI_Implied, CreateW65C02ImpliedCycles(&CW65CCore::CLI), "CLI", "Clear Interrupt Disable Bit");
	mapcInstructions[0x59] = Instruct(W65C02_EOR_AbsoluteIndexedWithY, CreateW65C02AbsoluteIndexedWithYCycles(&CW65CCore::EOR, false), "EOR", "'Exclusive OR' Memory with Accumulator");
	mapcInstructions[0x5A] = Instruct(W65C02_PHY_Stack, CreateW65C02StackPushCycles(&CW65CCore::PHY), "PHY", "Push Index Y on Stack");
	mapcInstructions[0x5B] = Instruct(W65C02_NOP_5B, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x5C] = Instruct(W65C02_NOP_5C, CreateW65C02NOPAbsoluteCycles(), "NOP", "No Operation");
	mapcInstructions[0x5D] = Instruct(W65C02_EOR_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXCycles(&CW65CCore::EOR, false), "EOR", "'Exclusive OR' Memory with Accumulator");
	mapcInstructions[0x5E] = Instruct(W65C02_LSR_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXRMWCycles(&CW65CCore::LSR, false), "LSR", "Shift memory right one bit; update NZC");
	mapcInstructions[0x5F] = Instruct(W65C02_BBR5_ZeroPageProgramCounterRelative, CreateW65C02ZeroPageRelativeCycles(), "BBR5", "Branch on Bit Reset");
	mapcInstructions[0x60] = Instruct(W65C02_RTS_Stack, CreateW65C02StackRTSCycles(), "RTS", "Return from Subroutine");
	mapcInstructions[0x61] = Instruct(W65C02_ADC_ZeroPageIndexedIndirectWithX, CreateW65C02ZeroPageIndexedIndirectWithXCycles(&CW65CCore::ADC, true), "ADC", "Add memory and carry to A; result in A and update NZC");
	mapcInstructions[0x62] = Instruct(W65C02_NOP_62, CreateW65C02NOPCycles(2), "NOP", "No Operation");
	mapcInstructions[0x63] = Instruct(W65C02_NOP_63, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x64] = Instruct(W65C02_STZ_ZeroPage, CreateW65C02ZeroPageWriteCycles(&CW65CCore::STZ), "STZ", "Store Zero in Memory");
	mapcInstructions[0x65] = Instruct(W65C02_ADC_ZeroPage, CreateW65C02ZeroPageCycles(&CW65CCore::ADC, true), "ADC", "Add memory and carry to A; result in A and update NZC");
	mapcInstructions[0x66] = Instruct(W65C02_ROR_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65CCore::ROR), "ROR", "Rotate memory right one bit; update NZC");
	mapcInstructions[0x67] = Instruct(W65C02_RMB6_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65C02::RMB), "RMB6", "Reset Memory Bit");
	mapcInstructions[0x68] = Instruct(W65C02_PLA_Stack, CreateW65C02StackPullCycles(&CW65CCore::PLA), "PLA", "Pull Accumulator from Stack");
	mapcInstructions[0x69] = Instruct(W65C02_ADC_Immediate, CreateW65C02ImmediateCycles(&CW65CCore::ADC, true, 0x7F), "ADC", "Add memory and carry to A; result in A and update NZC");
	mapcInstructions[0x6A] = Instruct(W65C02_ROR_Accumulator, CreateW65C02ImpliedCycles(&CW65CCore::ROR_A), "ROR", "Rotate memory right one bit; update NZC");
	mapcInstructions[0x6B] = Instruct(W65C02_NOP_6B, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x6C] = Instruct(W65C02_JMP_AbsoluteIndirect, CreateW65C02AbsoluteIndirectJMPCycles(), "JMP", "Jump to New Location");
	mapcInstructions[0x6D] = Instruct(W65C02_ADC_Absolute, CreateW65C02AbsoluteCycles(&CW65CCore::ADC, true), "ADC", "Add memory and carry to A; result in A and update NZC");
	mapcInstructions[0x6E] = Instruct(W65C02_ROR_Absolute, CreateW65C02AbsoluteRMWCycles(&CW65CCore::ROR), "ROR", "Rotate memory right one bit; update NZC");
	mapcInstructions[0x6F] = Instruct(W65C02_BBR6_ZeroPageProgramCounterRelative, CreateW65C02ZeroPageRelativeCycles(), "BBR6", "Branch on Bit Reset");
	mapcInstructions[0x70] = Instruct(W65C02_BVS_ProgramCounterRelative, CreateW65C02RelativeCycles(&CW65CCore::BVS), "BVS", "Branch on Overflow Set (V=1)");
	mapcInstructions[0x71] = Instruct(W65C02_ADC_ZeroPageIndirectIndexedWithY, CreateW65C02ZeroPageIndirectIndexedWithYCycles(&CW65CCore::ADC, true), "ADC", "Add memory and carry to A; result in A and update NZC");
	mapcInstructions[0x72] = Instruct(W65C02_ADC_ZeroPageIndirect, CreateW65C02ZeroPageIndirectCycles(&CW65CCore::ADC, true), "ADC", "Add memory and carry to A; result in A and update NZC");
	mapcInstructions[0x73] = Instruct(W65C02_NOP_73, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x74] = Instruct(W65C02_STZ_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedWriteCycles(false, &CW65CCore::STZ), "STZ", "Store Zero in Memory");
	mapcInstructions[0x75] = Instruct(W65C02_ADC_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedCycles(false, &CW65CCore::ADC, true), "ADC", "Add memory and carry to A; result in A and update NZC");
	mapcInstructions[0x76] = Instruct(W65C02_ROR_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedWithXRMWCycles(&CW65CCore::ROR), "ROR", "Rotate memory right one bit; update NZC");
	mapcInstructions[0x77] = Instruct(W65C02_RMB7_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65C02::RMB), "RMB7", "Reset Memory Bit");
	mapcInstructions[0x78] = Instruct(W65C02_SEI_Implied, CreateW65C02ImpliedCycles(&CW65CCore::SEI), "SEI", "Set Interrupt Disable Status");
	mapcInstructions[0x79] = Instruct(W65C02_ADC_AbsoluteIndexedWithY, CreateW65C02AbsoluteIndexedWithYCycles(&CW65CCore::ADC, true), "ADC", "Add memory and carry to A; result in A and update NZC");
	mapcInstructions[0x7A] = Instruct(W65C02_PLY_Stack, CreateW65C02StackPullCycles(&CW65CCore::PLY), "PLY", "Pull Index Y from Stack");
	mapcInstructions[0x7B] = Instruct(W65C02_NOP_7B, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x7C] = Instruct(W65C02_JMP_AbsoluteIndexedIndirectWithX, CreateW65C02AbsoluteIndexedIndirectWithXJMPCycles(), "JMP", "Jump to New Location");
	mapcInstructions[0x7D] = Instruct(W65C02_ADC_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXCycles(&CW65CCore::ADC, true), "ADC", "Add memory and carry to A; result in A and update NZC");
	mapcInstructions[0x7E] = Instruct(W65C02_ROR_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXRMWCycles(&CW65CCore::ROR, false), "ROR", "Rotate memory right one bit; update NZC");
	mapcInstructions[0x7F] = Instruct(W65C02_BBR7_ZeroPageProgramCounterRelative, CreateW65C02ZeroPageRelativeCycles(), "BBR7", "Branch on Bit Reset");
	mapcInstructions[0x80] = Instruct(W65C02_BRA_ProgramCounterRelative, CreateW65C02RelativeCycles(&CW65CCore::BRA), "BRA", "Branch Always");
	mapcInstructions[0x81] = Instruct(W65C02_STA_ZeroPageIndexedIndirectWithX, CreateW65C02ZeroPageIndexedIndirectWithXWriteCycles(&CW65CCore::STA), "STA", "Store Accumulator in Memory");
	mapcInstructions[0x82] = Instruct(W65C02_NOP_82, CreateW65C02NOPCycles(2), "NOP", "No Operation");
	mapcInstructions[0x83] = Instruct(W65C02_NOP_83, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x84] = Instruct(W65C02_STY_ZeroPage, CreateW65C02ZeroPageWriteCycles(&CW65CCore::STY), "STY", "Store Index Y in Memory");
	mapcInstructions[0x85] = Instruct(W65C02_STA_ZeroPage, CreateW65C02ZeroPageWriteCycles(&CW65CCore::STA), "STA", "Store Accumulator in Memory");
	mapcInstructions[0x86] = Instruct(W65C02_STX_ZeroPage, CreateW65C02ZeroPageWriteCycles(&CW65CCore::STX), "STX", "Store Index X in Memory");
	mapcInstructions[0x87] = Instruct(W65C02_SMB0_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65C02::SMB), "SMB0", "Set Memory Bit");
	mapcInstructions[0x88] = Instruct(W65C02_DEY_Implied, CreateW65C02ImpliedCycles(&CW65CCore::DEY), "DEY", "Decrement Index Y by One");
	mapcInstructions[0x89] = Instruct(W65C02_BIT_Immediate, CreateW65C02ImmediateCycles(&CW65CCore::BIT_A, false), "BIT", "Bit Test");
	mapcInstructions[0x8A] = Instruct(W65C02_TXA_Implied, CreateW65C02ImpliedCycles(&CW65CCore::TXA), "TXA", "Transfer Index X to Accumulator");
	mapcInstructions[0x8B] = Instruct(W65C02_NOP_8B, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x8C] = Instruct(W65C02_STY_Absolute, CreateW65C02AbsoluteWriteCycles(&CW65CCore::STY), "STY", "Store Index Y in Memory");
	mapcInstructions[0x8D] = Instruct(W65C02_STA_Absolute, CreateW65C02AbsoluteWriteCycles(&CW65CCore::STA), "STA", "Store Accumulator in Memory");
	mapcInstructions[0x8E] = Instruct(W65C02_STX_Absolute, CreateW65C02AbsoluteWriteCycles(&CW65CCore::STX), "STX", "Store Index X in Memory");
	mapcInstructions[0x8F] = Instruct(W65C02_BBS0_ZeroPageProgramCounterRelative, CreateW65C02ZeroPageRelativeCycles(), "BBS0", "Branch on Bit Set");
	mapcInstructions[0x90] = Instruct(W65C02_BCC_ProgramCounterRelative, CreateW65C02RelativeCycles(&CW65CCore::BCC), "BCC", "Branch on Carry Clear (C=0)");
	mapcInstructions[0x91] = Instruct(W65C02_STA_ZeroPageIndirectIndexedWithY, CreateW65C02ZeroPageIndirectIndexedWithYWriteCycles(&CW65CCore::STA), "STA", "Store Accumulator in Memory");
	mapcInstructions[0x92] = Instruct(W65C02_STA_ZeroPageIndirect, CreateW65C02ZeroPageIndirectWriteCycles(&CW65CCore::STA), "STA", "Store Accumulator in Memory");
	mapcInstructions[0x93] = Instruct(W65C02_NOP_93, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x94] = Instruct(W65C02_STY_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedWriteCycles(false, &CW65CCore::STY), "STY", "Store Index Y in Memory");
	mapcInstructions[0x95] = Instruct(W65C02_STA_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedWriteCycles(false, &CW65CCore::STA), "STA", "Store Accumulator in Memory");
	mapcInstructions[0x96] = Instruct(W65C02_STX_ZeroPageIndexedWithY, CreateW65C02ZeroPageIndexedWriteCycles(true, &CW65CCore::STX), "STX", "Store Index X in Memory");
	mapcInstructions[0x97] = Instruct(W65C02_SMB1_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65C02::SMB), "SMB1", "Set Memory Bit");
	mapcInstructions[0x98] = Instruct(W65C02_TYA_Implied, CreateW65C02ImpliedCycles(&CW65CCore::TYA), "TYA", "Transfer Index Y to Accumulator");
	mapcInstructions[0x99] = Instruct(W65C02_STA_AbsoluteIndexedWithY, CreateW65C02AbsoluteIndexedWithYWriteCycles(&CW65CCore::STA), "STA", "Store Accumulator in Memory");
	mapcInstructions[0x9A] = Instruct(W65C02_TXS_Implied, CreateW65C02ImpliedCycles(&CW65CCore::TXS), "TXS", "Transfer Index X to Stack Pointer Register");
	mapcInstructions[0x9B] = Instruct(W65C02_NOP_9B, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0x9C] = Instruct(W65C02_STZ_Absolute, CreateW65C02AbsoluteWriteCycles(&CW65CCore::STZ), "STZ", "Store Zero in Memory");
	mapcInstructions[0x9D] = Instruct(W65C02_STA_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXWriteCycles(&CW65CCore::STA), "STA", "Store Accumulator in Memory");
	mapcInstructions[0x9E] = Instruct(W65C02_STZ_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXWriteCycles(&CW65CCore::STZ), "STZ", "Store Zero in Memory");
	mapcInstructions[0x9F] = Instruct(W65C02_BBS1_ZeroPageProgramCounterRelative, CreateW65C02ZeroPageRelativeCycles(), "BBS1", "Branch on Bit Set");
	mapcInstructions[0xA0] = Instruct(W65C02_LDY_Immediate, CreateW65C02ImmediateCycles(&CW65CCore::LDY, false), "LDY", "Load Index Y with Memory");
	mapcInstructions[0xA1] = Instruct(W65C02_LDA_ZeroPageIndexedIndirectWithX, CreateW65C02ZeroPageIndexedIndirectWithXCycles(&CW65CCore::LDA, false), "LDA", "Load Accumulator with Memory");
	mapcInstructions[0xA2] = Instruct(W65C02_LDX_Immediate, CreateW65C02ImmediateCycles(&CW65CCore::LDX, false), "LDX", "Load Index X with Memory");
	mapcInstructions[0xA3] = Instruct(W65C02_NOP_A3, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0xA4] = Instruct(W65C02_LDY_ZeroPage, CreateW65C02ZeroPageCycles(&CW65CCore::LDY, false), "LDY", "Load Index Y with Memory");
	mapcInstructions[0xA5] = Instruct(W65C02_LDA_ZeroPage, CreateW65C02ZeroPageCycles(&CW65CCore::LDA, false), "LDA", "Load Accumulator with Memory");
	mapcInstructions[0xA6] = Instruct(W65C02_LDX_ZeroPage, CreateW65C02ZeroPageCycles(&CW65CCore::LDX, false), "LDX", "Load Index X with Memory");
	mapcInstructions[0xA7] = Instruct(W65C02_SMB2_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65C02::SMB), "SMB2", "Set Memory Bit");
	mapcInstructions[0xA8] = Instruct(W65C02_TAY_Implied, CreateW65C02ImpliedCycles(&CW65CCore::TAY), "TAY", "Transfer Accumulator in Index Y");
	mapcInstructions[0xA9] = Instruct(W65C02_LDA_Immediate, CreateW65C02ImmediateCycles(&CW65CCore::LDA, false), "LDA", "Load Accumulator with Memory");
	mapcInstructions[0xAA] = Instruct(W65C02_TAX_Implied, CreateW65C02ImpliedCycles(&CW65CCore::TAX), "TAX", "Transfer Accumulator in Index X");
	mapcInstructions[0xAB] = Instruct(W65C02_NOP_AB, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0xAC] = Instruct(W65C02_LDY_Absolute, CreateW65C02AbsoluteCycles(&CW65CCore::LDY, false), "LDY", "Load Index Y with Memory");
	mapcInstructions[0xAD] = Instruct(W65C02_LDA_Absolute, CreateW65C02AbsoluteCycles(&CW65CCore::LDA, false), "LDA", "Load Accumulator with Memory");
	mapcInstructions[0xAE] = Instruct(W65C02_LDX_Absolute, CreateW65C02AbsoluteCycles(&CW65CCore::LDX, false), "LDX", "Load Index X with Memory");
	mapcInstructions[0xAF] = Instruct(W65C02_BBS2_ZeroPageProgramCounterRelative, CreateW65C02ZeroPageRelativeCycles(), "BBS2", "Branch on Bit Set");
	mapcInstructions[0xB0] = Instruct(W65C02_BCS_ProgramCounterRelative, CreateW65C02RelativeCycles(&CW65CCore::BCS), "BCS", "Branch on Carry Set (C=1)");
	mapcInstructions[0xB1] = Instruct(W65C02_LDA_ZeroPageIndirectIndexedWithY, CreateW65C02ZeroPageIndirectIndexedWithYCycles(&CW65CCore::LDA, false), "LDA", "Load Accumulator with Memory");
	mapcInstructions[0xB2] = Instruct(W65C02_LDA_ZeroPageIndirect, CreateW65C02ZeroPageIndirectCycles(&CW65CCore::LDA, false), "LDA", "Load Accumulator with Memory");
	mapcInstructions[0xB3] = Instruct(W65C02_NOP_B3, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0xB4] = Instruct(W65C02_LDY_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedCycles(false, &CW65CCore::LDY, false), "LDY", "Load Index Y with Memory");
	mapcInstructions[0xB5] = Instruct(W65C02_LDA_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedCycles(false, &CW65CCore::LDA, false), "LDA", "Load Accumulator with Memory");
	mapcInstructions[0xB6] = Instruct(W65C02_LDX_ZeroPageIndexedWithY, CreateW65C02ZeroPageIndexedCycles(true, &CW65CCore::LDX, false), "LDX", "Load Index X with Memory");
	mapcInstructions[0xB7] = Instruct(W65C02_SMB3_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65C02::SMB), "SMB3", "Set Memory Bit");
	mapcInstructions[0xB8] = Instruct(W65C02_CLV_Implied, CreateW65C02ImpliedCycles(&CW65CCore::CLV), "CLV", "Clear Overflow Flag");
	mapcInstructions[0xB9] = Instruct(W65C02_LDA_AbsoluteIndexedWithY, CreateW65C02AbsoluteIndexedWithYCycles(&CW65CCore::LDA, false), "LDA", "Load Accumulator with Memory");
	mapcInstructions[0xBA] = Instruct(W65C02_TSX_Implied, CreateW65C02ImpliedCycles(&CW65CCore::TSX), "TSX", "Transfer Stack Pointer Register to Index X");
	mapcInstructions[0xBB] = Instruct(W65C02_NOP_BB, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0xBC] = Instruct(W65C02_LDY_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXCycles(&CW65CCore::LDY, false), "LDY", "Load Index Y with Memory");
	mapcInstructions[0xBD] = Instruct(W65C02_LDA_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXCycles(&CW65CCore::LDA, false), "LDA", "Load Accumulator with Memory");
	mapcInstructions[0xBE] = Instruct(W65C02_LDX_AbsoluteIndexedWithY, CreateW65C02AbsoluteIndexedWithYCycles(&CW65CCore::LDX, false), "LDX", "Load Index X with Memory");
	mapcInstructions[0xBF] = Instruct(W65C02_BBS3_ZeroPageProgramCounterRelative, CreateW65C02ZeroPageRelativeCycles(), "BBS3", "Branch on Bit Set");
	mapcInstructions[0xC0] = Instruct(W65C02_CPY_Immediate, CreateW65C02ImmediateCycles(&CW65CCore::CPY, false), "CPY", "Compare Memory and Index Y");
	mapcInstructions[0xC1] = Instruct(W65C02_CMP_ZeroPageIndexedIndirectWithX, CreateW65C02ZeroPageIndexedIndirectWithXCycles(&CW65CCore::CMP, false), "CMP", "Compare Memory and Accumulator");
	mapcInstructions[0xC2] = Instruct(W65C02_NOP_C2, CreateW65C02NOPCycles(2), "NOP", "No Operation");
	mapcInstructions[0xC3] = Instruct(W65C02_NOP_C3, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0xC4] = Instruct(W65C02_CPY_ZeroPage, CreateW65C02ZeroPageCycles(&CW65CCore::CPY, false), "CPY", "Compare Memory and Index Y");
	mapcInstructions[0xC5] = Instruct(W65C02_CMP_ZeroPage, CreateW65C02ZeroPageCycles(&CW65CCore::CMP, false), "CMP", "Compare Memory and Accumulator");
	mapcInstructions[0xC6] = Instruct(W65C02_DEC_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65CCore::DEC), "DEC", "Decrement accumulator; update NZ");
	mapcInstructions[0xC7] = Instruct(W65C02_SMB4_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65C02::SMB), "SMB4", "Set Memory Bit");
	mapcInstructions[0xC8] = Instruct(W65C02_INY_Implied, CreateW65C02ImpliedCycles(&CW65CCore::INY), "INY", "Increment Index Y by One");
	mapcInstructions[0xC9] = Instruct(W65C02_CMP_Immediate, CreateW65C02ImmediateCycles(&CW65CCore::CMP, false), "CMP", "Compare Memory and Accumulator");
	mapcInstructions[0xCA] = Instruct(W65C02_DEX_Implied, CreateW65C02ImpliedCycles(&CW65CCore::DEX), "DEX", "Decrement Index X by One");
	mapcInstructions[0xCB] = Instruct(W65C02_WAI_Implied, CreateW65C02WaitForInterruptCycles(), "WAI", "Wait for Interrupt");
	mapcInstructions[0xCC] = Instruct(W65C02_CPY_Absolute, CreateW65C02AbsoluteCycles(&CW65CCore::CPY, false), "CPY", "Compare Memory and Index Y");
	mapcInstructions[0xCD] = Instruct(W65C02_CMP_Absolute, CreateW65C02AbsoluteCycles(&CW65CCore::CMP, false), "CMP", "Compare Memory and Accumulator");
	mapcInstructions[0xCE] = Instruct(W65C02_DEC_Absolute, CreateW65C02AbsoluteRMWCycles(&CW65CCore::DEC), "DEC", "Decrement accumulator; update NZ");
	mapcInstructions[0xCF] = Instruct(W65C02_BBS4_ZeroPageProgramCounterRelative, CreateW65C02ZeroPageRelativeCycles(), "BBS4", "Branch on Bit Set");
	mapcInstructions[0xD0] = Instruct(W65C02_BNE_ProgramCounterRelative, CreateW65C02RelativeCycles(&CW65CCore::BNE), "BNE", "Branch if Not Equal (Z=0)");
	mapcInstructions[0xD1] = Instruct(W65C02_CMP_ZeroPageIndirectIndexedWithY, CreateW65C02ZeroPageIndirectIndexedWithYCycles(&CW65CCore::CMP, false), "CMP", "Compare Memory and Accumulator");
	mapcInstructions[0xD2] = Instruct(W65C02_CMP_ZeroPageIndirect, CreateW65C02ZeroPageIndirectCycles(&CW65CCore::CMP, false), "CMP", "Compare Memory and Accumulator");
	mapcInstructions[0xD3] = Instruct(W65C02_NOP_D3, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0xD4] = Instruct(W65C02_NOP_D4, CreateW65C02NOPZeroPageIndexedCycles(), "NOP", "No Operation");
	mapcInstructions[0xD5] = Instruct(W65C02_CMP_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedCycles(false, &CW65CCore::CMP, false), "CMP", "Compare Memory and Accumulator");
	mapcInstructions[0xD6] = Instruct(W65C02_DEC_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedWithXRMWCycles(&CW65CCore::DEC), "DEC", "Decrement accumulator; update NZ");
	mapcInstructions[0xD7] = Instruct(W65C02_SMB5_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65C02::SMB), "SMB5", "Set Memory Bit");
	mapcInstructions[0xD8] = Instruct(W65C02_CLD_Implied, CreateW65C02ImpliedCycles(&CW65CCore::CLD), "CLD", "Clear Decimal Mode");
	mapcInstructions[0xD9] = Instruct(W65C02_CMP_AbsoluteIndexedWithY, CreateW65C02AbsoluteIndexedWithYCycles(&CW65CCore::CMP, false), "CMP", "Compare Memory and Accumulator");
	mapcInstructions[0xDA] = Instruct(W65C02_PHX_Stack, CreateW65C02StackPushCycles(&CW65CCore::PHX), "PHX", "Push Index X on Stack");
	mapcInstructions[0xDB] = Instruct(W65C02_STP_Implied, CreateW65C02StopTheClockCycles(&CW65CCore::STP), "STP", "Stop the Clock");
	mapcInstructions[0xDC] = Instruct(W65C02_NOP_DC, CreateW65C02NOPAbsoluteCycles(), "NOP", "No Operation");
	mapcInstructions[0xDD] = Instruct(W65C02_CMP_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXCycles(&CW65CCore::CMP, false), "CMP", "Compare Memory and Accumulator");
	mapcInstructions[0xDE] = Instruct(W65C02_DEC_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXRMWCycles(&CW65CCore::DEC, true), "DEC", "Decrement accumulator; update NZ");
	mapcInstructions[0xDF] = Instruct(W65C02_BBS5_ZeroPageProgramCounterRelative, CreateW65C02ZeroPageRelativeCycles(), "BBS5", "Branch on Bit Set");
	mapcInstructions[0xE0] = Instruct(W65C02_CPX_Immediate, CreateW65C02ImmediateCycles(&CW65CCore::CPX, false), "CPX", "Compare Memory and Index X");
	mapcInstructions[0xE1] = Instruct(W65C02_SBC_ZeroPageIndexedIndirectWithX, CreateW65C02ZeroPageIndexedIndirectWithXCycles(&CW65CCore::SBC, true), "SBC", "Subtract memory and carry from A; result in A and update NZC");
	mapcInstructions[0xE2] = Instruct(W65C02_NOP_E2, CreateW65C02NOPCycles(2), "NOP", "No Operation");
	mapcInstructions[0xE3] = Instruct(W65C02_NOP_E3, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0xE4] = Instruct(W65C02_CPX_ZeroPage, CreateW65C02ZeroPageCycles(&CW65CCore::CPX, false), "CPX", "Compare Memory and Index X");
	mapcInstructions[0xE5] = Instruct(W65C02_SBC_ZeroPage, CreateW65C02ZeroPageCycles(&CW65CCore::SBC, true), "SBC", "Subtract memory and carry from A; result in A and update NZC");
	mapcInstructions[0xE6] = Instruct(W65C02_INC_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65CCore::INC), "INC", "Increment accumulator; update NZ");
	mapcInstructions[0xE7] = Instruct(W65C02_SMB6_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65C02::SMB), "SMB6", "Set Memory Bit");
	mapcInstructions[0xE8] = Instruct(W65C02_INX_Implied, CreateW65C02ImpliedCycles(&CW65CCore::INX), "INX", "Increment Index X by One");
	mapcInstructions[0xE9] = Instruct(W65C02_SBC_Immediate, CreateW65C02ImmediateCycles(&CW65CCore::SBC, true, 0x00), "SBC", "Subtract memory and carry from A; result in A and update NZC");
	mapcInstructions[0xEA] = Instruct(W65C02_NOP_Implied, CreateW65C02ImpliedCycles(&CW65CCore::NOP), "NOP", "No Operation");
	mapcInstructions[0xEB] = Instruct(W65C02_NOP_EB, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0xEC] = Instruct(W65C02_CPX_Absolute, CreateW65C02AbsoluteCycles(&CW65CCore::CPX, false), "CPX", "Compare Memory and Index X");
	mapcInstructions[0xED] = Instruct(W65C02_SBC_Absolute, CreateW65C02AbsoluteCycles(&CW65CCore::SBC, true), "SBC", "Subtract memory and carry from A; result in A and update NZC");
	mapcInstructions[0xEE] = Instruct(W65C02_INC_Absolute, CreateW65C02AbsoluteRMWCycles(&CW65CCore::INC), "INC", "Increment accumulator; update NZ");
	mapcInstructions[0xEF] = Instruct(W65C02_BBS6_ZeroPageProgramCounterRelative, CreateW65C02ZeroPageRelativeCycles(), "BBS6", "Branch on Bit Set");
	mapcInstructions[0xF0] = Instruct(W65C02_BEQ_ProgramCounterRelative, CreateW65C02RelativeCycles(&CW65CCore::BEQ), "BEQ", "Branch if Equal (Z=1)");
	mapcInstructions[0xF1] = Instruct(W65C02_SBC_ZeroPageIndirectIndexedWithY, CreateW65C02ZeroPageIndirectIndexedWithYCycles(&CW65CCore::SBC, true), "SBC", "Subtract memory and carry from A; result in A and update NZC");
	mapcInstructions[0xF2] = Instruct(W65C02_SBC_ZeroPageIndirect, CreateW65C02ZeroPageIndirectCycles(&CW65CCore::SBC, true), "SBC", "Subtract memory and carry from A; result in A and update NZC");
	mapcInstructions[0xF3] = Instruct(W65C02_NOP_F3, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0xF4] = Instruct(W65C02_NOP_F4, CreateW65C02NOPZeroPageIndexedCycles(), "NOP", "No Operation");
	mapcInstructions[0xF5] = Instruct(W65C02_SBC_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedCycles(false, &CW65CCore::SBC, true), "SBC", "Subtract memory and carry from A; result in A and update NZC");
	mapcInstructions[0xF6] = Instruct(W65C02_INC_ZeroPageIndexedWithX, CreateW65C02ZeroPageIndexedWithXRMWCycles(&CW65CCore::INC), "INC", "Increment accumulator; update NZ");
	mapcInstructions[0xF7] = Instruct(W65C02_SMB7_ZeroPage, CreateW65C02ZeroPageRMWCycles(&CW65C02::SMB), "SMB7", "Set Memory Bit");
	mapcInstructions[0xF8] = Instruct(W65C02_SED_Implied, CreateW65C02ImpliedCycles(&CW65CCore::SED), "SED", "Set Decimal Mode");
	mapcInstructions[0xF9] = Instruct(W65C02_SBC_AbsoluteIndexedWithY, CreateW65C02AbsoluteIndexedWithYCycles(&CW65CCore::SBC, true), "SBC", "Subtract memory and carry from A; result in A and update NZC");
	mapcInstructions[0xFA] = Instruct(W65C02_PLX_Stack, CreateW65C02StackPullCycles(&CW65CCore::PLX), "PLX", "Pull Index X from Stack");
	mapcInstructions[0xFB] = Instruct(W65C02_NOP_FB, CreateW65C02NOPCycles(1), "NOP", "No Operation");
	mapcInstructions[0xFC] = Instruct(W65C02_NOP_FC, CreateW65C02NOPAbsoluteCycles(), "NOP", "No Operation");
	mapcInstructions[0xFD] = Instruct(W65C02_SBC_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXCycles(&CW65CCore::SBC, true), "SBC", "Subtract memory and carry from A; result in A and update NZC");
	mapcInstructions[0xFE] = Instruct(W65C02_INC_AbsoluteIndexedWithX, CreateW65C02AbsoluteIndexedWithXRMWCycles(&CW65CCore::INC, true), "INC", "Increment accumulator; update NZ");
	mapcInstructions[0xFF] = Instruct(W65C02_BBS7_ZeroPageProgramCounterRelative, CreateW65C02ZeroPageRelativeCycles(), "BBS7", "Branch on Bit Set");
}

