#ifndef __INSTRUCTION_FACTORY_H__
#define __INSTRUCTION_FACTORY_H__
#include "InstructionCodes.h"
#include "Instruction.h"
#include "InstructionCycleFactory.h"
#include "W65C816.h"


class CInstructionFactory
{
protected:
	CInstruction*	mapcInstructions[256];
	CInstruction*	mpcReset;
	CInstruction*	mpcIRQ;
	CInstruction*	mpcNMI;
	CInstruction*	mpcAbort;
	CInstruction*	mpcFetchNext;

public:
	static	CInstructionFactory*	GetInstance(void);

			void					Init(void);

			CInstruction*			GetInstruction(uint16 uiOpcode);
			CInstruction*			GetReset(void);
			CInstruction*			GetIRQ(void);
			CInstruction*			GetNMI(void);
			CInstruction*			GetAbort(void);
			CInstruction*			GetFetchNext(void);

protected:
void CreateInstructions(void)
  {
    mapcInstructions[0] = CreateBrk(BRK_Interrupt, CreateStackSoftwareInterruptCycles(new BRKVector(), &CW65C816::BRK));
    mapcInstructions[1] = CreateORA(ORA_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::ORA));
    mapcInstructions[2] = CreateCOP(COP_Interrupt, CreateStackSoftwareInterruptCycles(new COPVector(), &CW65C816::COP));
    mapcInstructions[3] = CreateORA(ORA_StackRelative, CreateStackRelativeCycles(&CW65C816::ORA));
    mapcInstructions[4] = CreateTSB(TSB_DirectPage, CreateDirectRMWCycles(&CW65C816::TSB));
    mapcInstructions[5] = CreateORA(ORA_DirectPage, CreateDirectCycles(&CW65C816::ORA, WFR_M));
    mapcInstructions[6] = CreateASL(ASL_DirectPage, CreateDirectRMWCycles(&CW65C816::ASL));
    mapcInstructions[7] = CreateORA(ORA_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::ORA));
    mapcInstructions[8] = CreatePHP(PHP_StackImplied, CreateStackImpliedPHPCycles(&CW65C816::PHP));
    mapcInstructions[9] = CreateORA(ORA_Immediate, CreateImmediateCycles(&CW65C816::ORA, WFR_M));
    mapcInstructions[0] = CreateSLA(ASL_Accumulator, CreateAccumulatorCycles(&CW65C816::ASL_A));
    mapcInstructions[1] = CreatePHD(PHD_StackImplied, CreateStackPHDCycles(&CW65C816::PHD));
    mapcInstructions[2] = CreateTSB(TSB_Absolute, CreateAbsoluteRMWCycles(&CW65C816::TSB));
    mapcInstructions[3] = CreateORA(ORA_Absolute, CreateAbsoluteCycles(&CW65C816::ORA, WFR_M));
    mapcInstructions[4] = CreateASL(ASL_Absolute, CreateAbsoluteRMWCycles(&CW65C816::ASL));
    mapcInstructions[5] = CreateORA(ORA_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::ORA));
    mapcInstructions[6] = CreateBPL(BPL_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BPL));
    mapcInstructions[7] = CreateORA(ORA_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::ORA));
    mapcInstructions[8] = CreateORA(ORA_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::ORA));
    mapcInstructions[9] = CreateORA(ORA_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::ORA));
    mapcInstructions[0] = CreateTRB(TRB_DirectPage, CreateDirectRMWCycles(&CW65C816::TRB));
    mapcInstructions[1] = CreateORA(ORA_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::ORA, WFR_M));
    mapcInstructions[2] = CreateASL(ASL_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::ASL));
    mapcInstructions[3] = CreateORA(ORA_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::ORA));
    mapcInstructions[4] = CreateCLC(CLC_Implied, CreateImpliedCycles(&CW65C816::CLC));
    mapcInstructions[5] = CreateORA(ORA_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::ORA, WFR_M));
    mapcInstructions[6] = CreateINA(INC_Accumulator, CreateAccumulatorCycles(&CW65C816::INC_A));
    mapcInstructions[7] = CreateTCS(TCS_Implied, CreateImpliedCycles(&CW65C816::TCS));
    mapcInstructions[8] = CreateTRB(TRB_Absolute, CreateAbsoluteRMWCycles(&CW65C816::TRB));
    mapcInstructions[9] = CreateORA(ORA_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::ORA, WFR_M));
    mapcInstructions[0] = CreateASL(ASL_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::ASL));
    mapcInstructions[1] = CreateORA(ORA_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::ORA));
    mapcInstructions[2] = CreateJSR(JSR_Absolute, CreateAbsoluteJSRCycles());
    mapcInstructions[3] = CreateAND(AND_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::AND));
    mapcInstructions[4] = CreateJSL(JSL_AbsoluteLong, CreateAbsoluteLongJSLCycles());
    mapcInstructions[5] = CreateAND(AND_StackRelative, CreateStackRelativeCycles(&CW65C816::AND));
    mapcInstructions[6] = CreateBIT(BIT_DirectPage, CreateDirectCycles(&CW65C816::BIT, WFR_M));
    mapcInstructions[7] = CreateAND(AND_DirectPage, CreateDirectCycles(&CW65C816::AND, WFR_M));
    mapcInstructions[8] = CreateROL(ROL_DirectPage, CreateDirectRMWCycles(&CW65C816::ROL));
    mapcInstructions[9] = CreateAND(AND_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::AND));
    mapcInstructions[0] = CreatePLP(PLP_StackImplied, CreateStackPLPCycles(&CW65C816::PLP));
    mapcInstructions[1] = CreateAND(AND_Immediate, CreateImmediateCycles(&CW65C816::AND, WFR_M));
    mapcInstructions[2] = CreateRLA(ROL_Accumulator, CreateAccumulatorCycles(&CW65C816::ROL_A));
    mapcInstructions[3] = CreatePLD(PLD_StackImplied, CreateStackPLDCycles(&CW65C816::PLD));
    mapcInstructions[4] = CreateBIT(BIT_Absolute, CreateAbsoluteCycles(&CW65C816::BIT, WFR_M));
    mapcInstructions[5] = CreateAND(AND_Absolute, CreateAbsoluteCycles(&CW65C816::AND, WFR_M));
    mapcInstructions[6] = CreateROL(ROL_Absolute, CreateAbsoluteRMWCycles(&CW65C816::ROL));
    mapcInstructions[7] = CreateAND(AND_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::AND));
    mapcInstructions[8] = CreateBMI(BMI_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BMI));
    mapcInstructions[9] = CreateAND(AND_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::AND));
    mapcInstructions[0] = CreateAND(AND_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::AND));
    mapcInstructions[1] = CreateAND(AND_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::AND));
    mapcInstructions[2] = CreateBIT(BIT_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::BIT, WFR_M));
    mapcInstructions[3] = CreateAND(AND_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::AND, WFR_M));
    mapcInstructions[4] = CreateROL(ROL_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::ROL));
    mapcInstructions[5] = CreateAND(AND_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::AND));
    mapcInstructions[6] = CreateSEC(SEC_Implied, CreateImpliedCycles(&CW65C816::SEC));
    mapcInstructions[7] = CreateAND(AND_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::AND, WFR_M));
    mapcInstructions[8] = CreateDEA(DEC_Accumulator, CreateAccumulatorCycles(&CW65C816::DEC_A));
    mapcInstructions[9] = CreateTSC(TSC_Implied, CreateImpliedCycles(&CW65C816::TSC));
    mapcInstructions[0] = CreateBIT(BIT_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::BIT, WFR_M));
    mapcInstructions[1] = CreateAND(AND_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::AND, WFR_M));
    mapcInstructions[2] = CreateROL(ROL_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::ROL));
    mapcInstructions[3] = CreateAND(AND_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::AND));
    mapcInstructions[4] = CreateRTI(RTI_StackImplied, CreateStackRTICycles());
    mapcInstructions[5] = CreateEOR(EOR_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::EOR));
    mapcInstructions[6] = CreateWDM(WDM_Implied, CreateWDMImpliedCycles(&CW65C816::WDM));
    mapcInstructions[7] = CreateEOR(EOR_StackRelative, CreateStackRelativeCycles(&CW65C816::EOR));
    mapcInstructions[8] = CreateMVP(MVP_BlockMove, CreateBlockMoveCycles(&CW65C816::MVP));
    mapcInstructions[9] = CreateEOR(EOR_DirectPage, CreateDirectCycles(&CW65C816::EOR, WFR_M));
    mapcInstructions[0] = CreateLSR(LSR_DirectPage, CreateDirectRMWCycles(&CW65C816::LSR));
    mapcInstructions[1] = CreateEOR(EOR_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::EOR));
    mapcInstructions[2] = CreatePHA(PHA_StackImplied, CreateStackPushCycles(&CW65C816::PHA, WFR_M));
    mapcInstructions[3] = CreateEOR(EOR_Immediate, CreateImmediateCycles(&CW65C816::EOR, WFR_M));
    mapcInstructions[4] = CreateSRA(LSR_Accumulator, CreateAccumulatorCycles(&CW65C816::LSR_A));
    mapcInstructions[5] = CreatePHK(PHK_StackImplied, CreateStackPHKCycles(&CW65C816::PHK));
    mapcInstructions[6] = CreateJMP(JMP_Absolute, CreateAbsoluteJMPCycles());
    mapcInstructions[7] = CreateEOR(EOR_Absolute, CreateAbsoluteCycles(&CW65C816::EOR, WFR_M));
    mapcInstructions[8] = CreateLSR(LSR_Absolute, CreateAbsoluteRMWCycles(&CW65C816::LSR));
    mapcInstructions[9] = CreateEOR(EOR_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::EOR));
    mapcInstructions[0] = CreateBVC(BVC_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BVC));
    mapcInstructions[1] = CreateEOR(EOR_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::EOR));
    mapcInstructions[2] = CreateEOR(EOR_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::EOR));
    mapcInstructions[3] = CreateEOR(EOR_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::EOR));
    mapcInstructions[4] = CreateMVN(MVN_BlockMove, CreateBlockMoveCycles(&CW65C816::MVN));
    mapcInstructions[5] = CreateEOR(EOR_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::EOR, WFR_M));
    mapcInstructions[6] = CreateLSR(LSR_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::LSR));
    mapcInstructions[7] = CreateEOR(EOR_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::EOR));
    mapcInstructions[8] = CreateCLI(CLI_Implied, CreateImpliedCycles(&CW65C816::CLI));
    mapcInstructions[9] = CreateEOR(EOR_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::EOR, WFR_M));
    mapcInstructions[0] = CreatePHY(PHY_StackImplied, CreateStackPushCycles(&CW65C816::PHY, WFR_XY));
    mapcInstructions[1] = CreateTCD(TCD_Implied, CreateImpliedCycles(&CW65C816::TCD));
    mapcInstructions[2] = CreateJMP(JML_AbsoluteLong, CreateAbsoluteLongJMLCycles());
    mapcInstructions[3] = CreateEOR(EOR_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::EOR, WFR_M));
    mapcInstructions[4] = CreateLSR(LSR_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::LSR));
    mapcInstructions[5] = CreateEOR(EOR_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::EOR));
    mapcInstructions[6] = CreateRTS(RTS_StackImplied, CreateStackRTSCycles());
    mapcInstructions[7] = CreateADC(ADC_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::ADC));
    mapcInstructions[8] = CreatePER(PER_StackProgramCounterRelativeLong, CreateStackPERCycles(&CW65C816::PER));
    mapcInstructions[9] = CreateADC(ADC_StackRelative, CreateStackRelativeCycles(&CW65C816::ADC));
    mapcInstructions[0] = CreateSTZ(STZ_DirectPage, CreateDirectWriteCycles(&CW65C816::STZ, WFR_M));
    mapcInstructions[1] = CreateADC(ADC_DirectPage, CreateDirectCycles(&CW65C816::ADC, WFR_M));
    mapcInstructions[2] = CreateROR(ROR_DirectPage, CreateDirectRMWCycles(&CW65C816::ROR));
    mapcInstructions[3] = CreateADC(ADC_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::ADC));
    mapcInstructions[4] = CreatePLA(PLA_Stack, CreateStackPullCycles(&CW65C816::PLA, WFR_M));
    mapcInstructions[5] = CreateADC(ADC_Immediate, CreateImmediateCycles(&CW65C816::ADC, WFR_M));
    mapcInstructions[6] = CreateRRA(ROR_Accumulator, CreateAccumulatorCycles(&CW65C816::ROR_A));
    mapcInstructions[7] = CreateRTL(RTL_StackImplied, CreateStackRTLCycles());
    mapcInstructions[8] = CreateJMP(JMP_AbsoluteIndirect, CreateAbsoluteIndirectJMPCycles());
    mapcInstructions[9] = CreateADC(ADC_Absolute, CreateAbsoluteCycles(&CW65C816::ADC, WFR_M));
    mapcInstructions[0] = CreateROR(ROR_Absolute, CreateAbsoluteRMWCycles(&CW65C816::ROR));
    mapcInstructions[1] = CreateADC(ADC_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::ADC));
    mapcInstructions[2] = CreateBVS(BVS_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BVS));
    mapcInstructions[3] = CreateADC(ADC_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::ADC));
    mapcInstructions[4] = CreateADC(ADC_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::ADC));
    mapcInstructions[5] = CreateADC(ADC_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::ADC));
    mapcInstructions[6] = CreateSTZ(STZ_DirectPageIndexedWithX, CreateDirectIndexedWithXWriteCycles(&CW65C816::STZ, WFR_M));
    mapcInstructions[7] = CreateADC(ADC_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::ADC, WFR_M));
    mapcInstructions[8] = CreateROR(ROR_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::ROR));
    mapcInstructions[9] = CreateADC(ADC_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::ADC));
    mapcInstructions[0] = CreateSEI(SEI_Implied, CreateImpliedCycles(&CW65C816::SEI));
    mapcInstructions[1] = CreateADC(ADC_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::ADC, WFR_M));
    mapcInstructions[2] = CreatePLY(PLY_StackImplied, CreateStackPullCycles(&CW65C816::PLY, WFR_XY));
    mapcInstructions[3] = CreateTDC(TDC_Implied, CreateImpliedCycles(&CW65C816::TDC));
    mapcInstructions[4] = CreateJMP(JMP_AbsoluteIndexedIndirectWithX, CreateAbsoluteIndexedIndirectWithXJMPCycles());
    mapcInstructions[5] = CreateADC(ADC_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::ADC, WFR_M));
    mapcInstructions[6] = CreateROR(ROR_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::ROR));
    mapcInstructions[7] = CreateADC(ADC_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::ADC));
    mapcInstructions[8] = CreateBRA(BRA_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BRA));
    mapcInstructions[9] = CreateSTA(STA_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXWriteCycles(&CW65C816::STA));
    mapcInstructions[0] = CreateBRL(BRL_ProgramCounterRelativeLong, CreateRelativeLongCycles(&CW65C816::BRA));
    mapcInstructions[1] = CreateSTA(STA_StackRelative, CreateStackRelativeWriteCycles(&CW65C816::STA));
    mapcInstructions[2] = CreateSTY(STY_DirectPage, CreateDirectWriteCycles(&CW65C816::STY, WFR_XY));
    mapcInstructions[3] = CreateSTA(STA_DirectPage, CreateDirectWriteCycles(&CW65C816::STA, WFR_M));
    mapcInstructions[4] = CreateSTX(STX_DirectPage, CreateDirectWriteCycles(&CW65C816::STX, WFR_XY));
    mapcInstructions[5] = CreateSTA(STA_DirectPageIndirectLong, CreateDirectIndirectLongWriteCycles(&CW65C816::STA));
    mapcInstructions[6] = CreateDEY(DEY_Implied, CreateImpliedCycles(&CW65C816::DEY));
    mapcInstructions[7] = CreateBIT(BIT_Immediate, CreateImmediateCycles(&CW65C816::BIT_I, WFR_M));
    mapcInstructions[8] = CreateTXA(TXA_Implied, CreateImpliedCycles(&CW65C816::TXA));
    mapcInstructions[9] = CreatePHB(PHB_StackImplied, CreateStackPHBCycles(&CW65C816::PHB));
    mapcInstructions[0] = CreateSTY(STY_Absolute, CreateAbsoluteWriteCycles(&CW65C816::STY, WFR_XY));
    mapcInstructions[1] = CreateSTA(STA_Absolute, CreateAbsoluteWriteCycles(&CW65C816::STA, WFR_M));
    mapcInstructions[2] = CreateSTX(STX_Absolute, CreateAbsoluteWriteCycles(&CW65C816::STX, WFR_XY));
    mapcInstructions[3] = CreateSTA(STA_AbsoluteLong, CreateAbsoluteLongWriteCycles(&CW65C816::STA));
    mapcInstructions[4] = CreateBCC(BCC_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BCC));
    mapcInstructions[5] = CreateSTA(STA_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYWriteCycles(&CW65C816::STA));
    mapcInstructions[6] = CreateSTA(STA_DirectPageIndirect, CreateDirectIndirectWriteCycles(&CW65C816::STA));
    mapcInstructions[7] = CreateSTA(STA_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYWriteCycles(&CW65C816::STA));
    mapcInstructions[8] = CreateSTY(STY_DirectPageIndexedWithX, CreateDirectIndexedWithXWriteCycles(&CW65C816::STY, WFR_XY));
    mapcInstructions[9] = CreateSTA(STA_DirectPageIndexedWithX, CreateDirectIndexedWithXWriteCycles(&CW65C816::STA, WFR_M));
    mapcInstructions[0] = CreateSTX(STX_DirectPageIndexedWithY, CreateDirectIndexedWithYWriteCycles(&CW65C816::STX, WFR_XY));
    mapcInstructions[1] = CreateSTA(STA_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYWriteCycles(&CW65C816::STA));
    mapcInstructions[2] = CreateTYA(TYA_Implied, CreateImpliedCycles(&CW65C816::TYA));
    mapcInstructions[3] = CreateSTA(STA_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYWriteCycles(&CW65C816::STA));
    mapcInstructions[4] = CreateTXS(TXS_Implied, CreateImpliedCycles(&CW65C816::TXS));
    mapcInstructions[5] = CreateTXY(TXY_Implied, CreateImpliedCycles(&CW65C816::TXY));
    mapcInstructions[6] = CreateSTZ(STZ_Absolute, CreateAbsoluteWriteCycles(&CW65C816::STZ, WFR_M));
    mapcInstructions[7] = CreateSTA(STA_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXWriteCycles(&CW65C816::STA));
    mapcInstructions[8] = CreateSTZ(STZ_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXWriteCycles(&CW65C816::STZ));
    mapcInstructions[9] = CreateSTA(STA_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXWriteCycles(&CW65C816::STA));
    mapcInstructions[0] = CreateLDY(LDY_Immediate, CreateImmediateCycles(&CW65C816::LDY, WFR_XY));
    mapcInstructions[1] = CreateLDA(LDA_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::LDA));
    mapcInstructions[2] = CreateLDX(LDX_Immediate, CreateImmediateCycles(&CW65C816::LDX, WFR_XY));
    mapcInstructions[3] = CreateLDA(LDA_StackRelative, CreateStackRelativeCycles(&CW65C816::LDA));
    mapcInstructions[4] = CreateLDY(LDY_DirectPage, CreateDirectCycles(&CW65C816::LDY, WFR_XY));
    mapcInstructions[5] = CreateLDA(LDA_DirectPage, CreateDirectCycles(&CW65C816::LDA, WFR_M));
    mapcInstructions[6] = CreateLDX(LDX_DirectPage, CreateDirectCycles(&CW65C816::LDX, WFR_XY));
    mapcInstructions[7] = CreateLDA(LDA_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::LDA));
    mapcInstructions[8] = CreateTAY(TAY_Implied, CreateImpliedCycles(&CW65C816::TAY));
    mapcInstructions[9] = CreateLDA(LDA_Immediate, CreateImmediateCycles(&CW65C816::LDA, WFR_M));
    mapcInstructions[0] = CreateTAX(TAX_Implied, CreateImpliedCycles(&CW65C816::TAX));
    mapcInstructions[1] = CreatePLB(PLB_StackImplied, CreateStackPLBCycles(&CW65C816::PLB));
    mapcInstructions[2] = CreateLDY(LDY_Absolute, CreateAbsoluteCycles(&CW65C816::LDY, WFR_XY));
    mapcInstructions[3] = CreateLDA(LDA_Absolute, CreateAbsoluteCycles(&CW65C816::LDA, WFR_M));
    mapcInstructions[4] = CreateLDX(LDX_Absolute, CreateAbsoluteCycles(&CW65C816::LDX, WFR_XY));
    mapcInstructions[5] = CreateLDA(LDA_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::LDA));
    mapcInstructions[6] = CreateBCS(BCS_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BCS));
    mapcInstructions[7] = CreateLDA(LDA_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::LDA));
    mapcInstructions[8] = CreateLDA(LDA_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::LDA));
    mapcInstructions[9] = CreateLDA(LDA_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::LDA));
    mapcInstructions[0] = CreateLDY(LDY_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::LDY, WFR_XY));
    mapcInstructions[1] = CreateLDA(LDA_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::LDA, WFR_M));
    mapcInstructions[2] = CreateLDX(LDX_DirectPageIndexedWithY, CreateDirectIndexedWithYCycles(&CW65C816::LDX, WFR_XY));
    mapcInstructions[3] = CreateLDA(LDA_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::LDA));
    mapcInstructions[4] = CreateCLV(CLV_Implied, CreateImpliedCycles(&CW65C816::CLV));
    mapcInstructions[5] = CreateLDA(LDA_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::LDA, WFR_M));
    mapcInstructions[6] = CreateTSX(TSX_Implied, CreateImpliedCycles(&CW65C816::TSX));
    mapcInstructions[7] = CreateTYX(TYX_Implied, CreateImpliedCycles(&CW65C816::TYX));
    mapcInstructions[8] = CreateLDY(LDY_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::LDY, WFR_XY));
    mapcInstructions[9] = CreateLDA(LDA_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::LDA, WFR_M));
    mapcInstructions[0] = CreateLDX(LDX_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::LDX, WFR_XY));
    mapcInstructions[1] = CreateLDA(LDA_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::LDA));
    mapcInstructions[2] = CreateCPY(CPY_Immediate, CreateImmediateCycles(&CW65C816::CPY, WFR_XY));
    mapcInstructions[3] = CreateCMP(CMP_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::CMP));
    mapcInstructions[4] = CreateREP(REP_Immediate, CreateImmediateREPSEPCycles(&CW65C816::REP));
    mapcInstructions[5] = CreateCMP(CMP_StackRelative, CreateStackRelativeCycles(&CW65C816::CMP));
    mapcInstructions[6] = CreateCPY(CPY_DirectPage, CreateDirectCycles(&CW65C816::CPY, WFR_XY));
    mapcInstructions[7] = CreateCMP(CMP_DirectPage, CreateDirectCycles(&CW65C816::CMP, WFR_M));
    mapcInstructions[8] = CreateDEC(DEC_DirectPage, CreateDirectRMWCycles(&CW65C816::DEC));
    mapcInstructions[9] = CreateCMP(CMP_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::CMP));
    mapcInstructions[0] = CreateINY(INY_Implied, CreateImpliedCycles(&CW65C816::INY));
    mapcInstructions[1] = CreateCMP(CMP_Immediate, CreateImmediateCycles(&CW65C816::CMP, WFR_M));
    mapcInstructions[2] = CreateDEX(DEX_Implied, CreateImpliedCycles(&CW65C816::DEX));
    mapcInstructions[3] = CreateWAI(WAI_Implied, CreateWaitForInterruptCycles());
    mapcInstructions[4] = CreateCPY(CPY_Absolute, CreateAbsoluteCycles(&CW65C816::CPY, WFR_XY));
    mapcInstructions[5] = CreateCMP(CMP_Absolute, CreateAbsoluteCycles(&CW65C816::CMP, WFR_M));
    mapcInstructions[6] = CreateDEC(DEC_Absolute, CreateAbsoluteRMWCycles(&CW65C816::DEC));
    mapcInstructions[7] = CreateCMP(CMP_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::CMP));
    mapcInstructions[8] = CreateBNE(BNE_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BNE));
    mapcInstructions[9] = CreateCMP(CMP_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::CMP));
    mapcInstructions[0] = CreateCMP(CMP_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::CMP));
    mapcInstructions[1] = CreateCMP(CMP_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::CMP));
    mapcInstructions[2] = CreatePEI(PEI_StackDirectPageIndirect, CreateStackPEICycles());
    mapcInstructions[3] = CreateCMP(CMP_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::CMP, WFR_M));
    mapcInstructions[4] = CreateDEC(DEC_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::DEC));
    mapcInstructions[5] = CreateCMP(CMP_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::CMP));
    mapcInstructions[6] = CreateCLD(CLD_Implied, CreateImpliedCycles(&CW65C816::CLD));
    mapcInstructions[7] = CreateCMP(CMP_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::CMP, WFR_M));
    mapcInstructions[8] = CreatePHX(PHX_StackImplied, CreateStackPushCycles(&CW65C816::PHX, WFR_XY));
    mapcInstructions[9] = CreateSTP(STP_Implied, CreateStopTheClockCycles(&CW65C816::STP));
    mapcInstructions[0] = CreateJMP(JML_AbsoluteIndirectLong, CreateAbsoluteIndirectJMLCycles());
    mapcInstructions[1] = CreateCMP(CMP_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::CMP, WFR_M));
    mapcInstructions[2] = CreateDEC(DEC_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::DEC));
    mapcInstructions[3] = CreateCMP(CMP_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::CMP));
    mapcInstructions[4] = CreateCPX(CPX_Immediate, CreateImmediateCycles(&CW65C816::CPX, WFR_XY));
    mapcInstructions[5] = CreateSBC(SBC_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::SBC));
    mapcInstructions[6] = CreateSEP(SEP_Immediate, CreateImmediateREPSEPCycles(&CW65C816::SEP));
    mapcInstructions[7] = CreateSBC(SBC_StackRelative, CreateStackRelativeCycles(&CW65C816::SBC));
    mapcInstructions[8] = CreateCPX(CPX_DirectPage, CreateDirectCycles(&CW65C816::CPX, WFR_XY));
    mapcInstructions[9] = CreateSBC(SBC_DirectPage, CreateDirectCycles(&CW65C816::SBC, WFR_M));
    mapcInstructions[0] = CreateINC(INC_DirectPage, CreateDirectRMWCycles(&CW65C816::INC));
    mapcInstructions[1] = CreateSBC(SBC_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::SBC));
    mapcInstructions[2] = CreateINX(INX_Implied, CreateImpliedCycles(&CW65C816::INX));
    mapcInstructions[3] = CreateSBC(SBC_Immediate, CreateImmediateCycles(&CW65C816::SBC, WFR_M));
    mapcInstructions[4] = CreateNOP(NOP_Implied, CreateImpliedCycles(&CW65C816::NOP));
    mapcInstructions[5] = CreateXBA(XBA_Implied, CreateImpliedXBACycles(&CW65C816::XBA));
    mapcInstructions[6] = CreateCPX(CPX_Absolute, CreateAbsoluteCycles(&CW65C816::CPX, WFR_XY));
    mapcInstructions[7] = CreateSBC(SBC_Absolute, CreateAbsoluteCycles(&CW65C816::SBC, WFR_M));
    mapcInstructions[8] = CreateINC(INC_Absolute, CreateAbsoluteRMWCycles(&CW65C816::INC));
    mapcInstructions[9] = CreateSBC(SBC_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::SBC));
    mapcInstructions[0] = CreateBEQ(BEQ_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BEQ));
    mapcInstructions[1] = CreateSBC(SBC_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::SBC));
    mapcInstructions[2] = CreateSBC(SBC_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::SBC));
    mapcInstructions[3] = CreateSBC(SBC_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::SBC));
    mapcInstructions[4] = CreatePEA(PEA_StackImmediate, CreateStackPEACycles());
    mapcInstructions[5] = CreateSBC(SBC_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::SBC, WFR_M));
    mapcInstructions[6] = CreateINC(INC_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::INC));
    mapcInstructions[7] = CreateSBC(SBC_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::SBC));
    mapcInstructions[8] = CreateSED(SED_Implied, CreateImpliedCycles(&CW65C816::SED));
    mapcInstructions[9] = CreateSBC(SBC_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::SBC, WFR_M));
    mapcInstructions[0] = CreatePLX(PLX_StackImplied, CreateStackPullCycles(&CW65C816::PLX, WFR_XY));
    mapcInstructions[1] = CreateXCE(XCE_Implied, CreateImpliedCycles(&CW65C816::XCE));
    mapcInstructions[2] = CreateJSR(JSR_AbsoluteIndexedIndirectWithX, CreateAbsoluteIndexedIndirectWithXJSRCycles());
    mapcInstructions[3] = CreateSBC(SBC_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::SBC, WFR_M));
    mapcInstructions[4] = CreateINC(INC_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::INC));
    mapcInstructions[5] = CreateSBC(SBC_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::SBC));

    for (uint16 i = 0; i <= 255; i++)
    {
      CInstruction* opCode = mapcInstructions[i];
      if (opCode->GetCode() != i)
      {

          gcLogger.Error2(__METHOD__, " OpCode [", opCode->GetName(), "] has code [", ShortToString(opCode->GetCode()), "] but is at index [", ShortToString(i), "].", NULL);
      }
    }
  }


#define INSTRUCTION()
#define ADDRESS(code, cycles, name, description) \
{ \
	CInstruction*	pcInstruction; \
	pcInstruction = NewMalloc<CInstruction>(); \
	pcInstruction->Init(code, cycles, name, description); \
	return pcInstruction; \
}


  CInstruction* CreateBEQ(int code, CInstructionCycles* pcInstructionCycles)
  {
      INSTRUCTION(code, pcInstructionCycles, "BEQ",
                           "Branch if Equal (Z=1)");
  }

  CInstruction* CreateXBA(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "XBA",
                           "Exchange B and A Accumulator");
  }

  CInstruction* CreateNOP(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "NOP",
                           "No Operation for two cycles.");
  }

  CInstruction* CreateINX(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "INX",
                           "Increment Index X by One");
  }

  CInstruction* CreateSEP(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "SEP",
                           "Set Processor Status Bit");
  }

  CInstruction* CreateBNE(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "BNE",
                           "Branch if Not Equal (Z=0)");
  }

  CInstruction* CreateWAI(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "WAI",
                           "Wait for Interrupt");
  }

  CInstruction* CreateDEX(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "DEX",
                           "Decrement Index X by One.");
  }

  CInstruction* CreateINY(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "INY",
                           "Increment Index Y by One");
  }

  CInstruction* CreatePEA(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "PEA",
                           "Push Absolute Address");
  }

  CInstruction* CreateINC(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "INC",
                           "Increment memory; result in memory and update NZ.");
  }

  CInstruction* CreateXCE(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "XCE",
                           "Exchange Carry and Emulation Bits");
  }

  CInstruction* CreatePLX(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "PLX",
                           "Pull Index X from Stack");
  }

  CInstruction* CreateSED(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles,
                           "SED", "Set Decimal Mode");
  }

  CInstruction* CreateSBC(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "SBC",
                           "Subtract memory and carry from A; result in A and update NZC.");
  }

  CInstruction* CreateCPX(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "CPX",
                           "Compare Memory and Index X");
  }

  CInstruction* CreateSTP(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "STP",
                           "Stop the Clock");
  }

  CInstruction* CreatePHX(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "PHX",
                           "Push Index X on Stack");
  }

  CInstruction* CreateCLD(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "CLD",
                           "Clear Decimal Mode");
  }

  CInstruction* CreatePEI(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "PEI",
                           "Push Indirect Address");
  }

  CInstruction* CreateDEC(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "DEC",
                           "Decrement memory; result in memory and update NZ.");
  }

  CInstruction* CreateREP(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "REP",
                           "Reset Status Bits");
  }

  CInstruction* CreateCMP(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "CMP",
                           "Compare Memory and Accumulator");
  }

  CInstruction* CreateCPY(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "CPY",
                           "Compare Memory and Index Y");
  }

  CInstruction* CreateTYX(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "TYX",
                           "Transfer Index Y to Index X");
  }

  CInstruction* CreateTSX(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "TSX",
                           "Transfer Stack Pointer Register to Index X");
  }

  CInstruction* CreateCLV(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "CLV",
                           "Clear Overflow Flag");
  }

  CInstruction* CreateBCS(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "BCS",
                           "Branch on Carry Set (C=1)");
  }

  CInstruction* CreatePLB(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "PLB",
                           "Pull Data Bank Register from Stack");
  }

  CInstruction* CreateTAX(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "TAX",
                           "Transfer Accumulator in Index X");
  }

  CInstruction* CreateTAY(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "TAY",
                           "Transfer Accumulator in Index Y");
  }

  CInstruction* CreateLDX(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "LDX", "Load Index X with Memory");
  }

  CInstruction* CreateBRL(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "BRL",
                           "Branch Always Long");
  }

  CInstruction* CreateBRA(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "BRA",
                           "Branch Always");
  }

  CInstruction* CreateTDC(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "TDC",
                           "Transfer Direct Register to C Accumulator");
  }

  CInstruction* CreatePLY(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "PLY",
                           "Pull Index Y from Stack");
  }

  CInstruction* CreateSEI(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "SEI",
                           "Set Interrupt Disable Status");
  }

  CInstruction* CreateBVS(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "BVS", "Branch on Overflow Set (V=1)");
  }

  CInstruction* CreateRTL(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "RTL",
                           "Return from Subroutine Long");
  }

  CInstruction* CreateRRA(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "ROR",
                           "Rotate accumulator right one bit; update NZC.");
  }

  CInstruction* CreatePLA(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "PLA",
                           "Pull Accumulator from Stack");
  }

  CInstruction* CreateROR(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "ROR",
                           "Rotate memory right one bit; update NZC.");
  }

  CInstruction* CreatePER(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "PER",
                           "Push Program Counter Relative Address");
  }

  CInstruction* CreateADC(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "ADC",
                           "Add memory and carry to A; result in A and update NZC.");
  }

  CInstruction* CreateRTS(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "RTS",
                           "Return from Subroutine");
  }

  CInstruction* CreateTCD(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "TCD",
                           "Transfer C Accumulator to Direct Register");
  }

  CInstruction* CreatePHY(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "PHY",
                           "Push Index Y on Stack");
  }

  CInstruction* CreateCLI(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "CLI",
                           "Clear Interrupt Disable Bit");
  }

  CInstruction* CreateMVN(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "MVN",
                           "Block move next...");
  }

  CInstruction* CreateBVC(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "BVC",
                           "Branch on Overflow Clear (V=0)");
  }

  CInstruction* CreateJMP(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "JMP",
                           "Jump to New Location");
  }

  CInstruction* CreatePHK(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "PHK",
                           "Push Program Bank Register on Stack");
  }

  CInstruction* CreateSRA(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "LSR",
                           "Shift accumulator right one bit; update NZC.");
  }

  CInstruction* CreatePHA(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "PHA",
                           "Push Accumulator onto Stack");
  }

  CInstruction* CreateLSR(int code_lsr, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code_lsr, pcInstructionCycles,
                           "LSR", "Shift memory right one bit; update NZC.");
  }

  CInstruction* CreateMVP(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "MVP",
                           "Block move previous...");
  }

  CInstruction* CreateWDM(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "WDM",
                           "Reserved for future use");
  }

  CInstruction* CreateEOR(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "EOR",
                           "'Exclusive OR' Memory with Accumulator");
  }

  CInstruction* CreateRTI(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "RTI",
                           "Return from Interrupt");
  }

  CInstruction* CreateTSC(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "TSC",
                           "Transfer Stack Pointer to C Accumulator");
  }

  CInstruction* CreateDEA(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "DEC",
                           "Decrement accumulator; update NZ.");
  }

  CInstruction* CreateSEC(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "SEC",
                           "Set Carry Flag");
  }

  CInstruction* CreateBMI(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "BMI",
                           "Branch if Result Minus (N=1)");
  }

  CInstruction* CreatePLP(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "PLP",
                           "Pull Processor Status from Stack");
  }

  CInstruction* CreateRLA(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "ROL",
                           "Rotate Accumulator One Bit Left.");
  }

  CInstruction* CreatePLD(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "PLD",
                           "Pull Direct Register from Stack");
  }

  CInstruction* CreateROL(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "ROL",
                           "Rotate Memory One Bit Left.");
  }

  CInstruction* CreateLDA(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "LDA",
                           "Load Accumulator with Memory");
  }

  CInstruction* CreateLDY(int code, CInstructionCycles* immediateCycles)
  {
    INSTRUCTION(code, immediateCycles, "LDY",
                           "Load Index Y with Memory");
  }

  CInstruction* CreatePHB(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "PHB",
                           "Push Data Bank Register on Stack");
  }

  CInstruction* CreateTXA(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "TXA",
                           "Transfer Index X to Accumulator");
  }

  CInstruction* CreateSTY(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "STY",
                           "Store Index Y in Memory");
  }

  CInstruction* CreateBCC(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "BCC",
                           "Branch on Carry Clear (C=0)");
  }

  CInstruction* CreateSTX(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "STX",
                           "Store Index X in Memory");
  }

  CInstruction* CreateTYA(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "TYA",
                           "Transfer Index Y to Accumulator");
  }

  CInstruction* CreateTXS(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "TXS",
                           "Transfer Index X to Stack Pointer Register");
  }

  CInstruction* CreateTXY(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "TXY",
                           "Transfer Index X to Index Y");
  }

  CInstruction* CreateSTZ(int stz_absolute, CInstructionCycles* absoluteWriteCycles)
  {
    INSTRUCTION(stz_absolute, absoluteWriteCycles,
                           "STZ", "Store Zero in Memory");
  }

  CInstruction* CreateSTA(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "STA",
                           "Store Accumulator in Memory");
  }

  CInstruction* CreateDEY(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "DEY", "Decrement Index Y by One");
  }

  CInstruction* CreateBIT(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "BIT",
                           "Bit Test");
  }

  CInstruction* CreateJSL(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "JSL",
                           "Jump long to new location save return address on Stack.");
  }

  CInstruction* CreateAND(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "AND",
                           "Bitwise AND memory with A; result in A and update NZ.");
  }

  CInstruction* CreateJSR(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "JSR",
                           "Jump to new location save return address on Stack.");
  }

  CInstruction* CreateTCS(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "TCS",
                           "Transfer C Accumulator to Stack Pointer");
  }

  CInstruction* CreateINA(int code, CInstructionCycles* busCycles)
  {
    INSTRUCTION(code, busCycles, "INC",
                           "Increment accumulator; update NZ.");
  }

  CInstruction* CreateCLC(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "CLC",
                           "Clear Carry Flag");
  }

  CInstruction* CreateTRB(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "TRB",
                           "Test and Reset Bit");
  }

  CInstruction* CreateBPL(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "BPL",
                           "Branch if Result Plus (N=0)");
  }

  CInstruction* CreatePHD(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "PHD",
                           "Push Direct Register on Stack");
  }

  CInstruction* CreateSLA(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "ASL",
                           "Shift accumulator left one bit; update NZC.");
  }

  CInstruction* CreateASL(int code, CInstructionCycles* directRMWCycles)
  {
    INSTRUCTION(code, directRMWCycles, "ASL", "Shift memory left 1 bit; result in memory and update NZC.");
  }

  CInstruction* CreatePHP(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "PHP", "Push Processor Status on Stack");
  }

  CInstruction* CreateTSB(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles,
                           "TSB", "Test and Set Bit");
  }

  CInstruction* CreateCOP(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "COP",
                           "Force co-processor software interrupt.");
  }

  CInstruction* CreateORA(int code, CInstructionCycles* directIndexedIndirectWithXCycles)
  {
    INSTRUCTION(code, directIndexedIndirectWithXCycles,
                           "ORA", "'OR' memory with A; result in A and update NZ.");
  }

  CInstruction* CreateBrk(int code, CInstructionCycles* pcInstructionCycles)
  {
    INSTRUCTION(code, pcInstructionCycles, "BRK",
                           "Force break software interrupt.");
  }

  CInstruction* CreateReset(List<CInstruction*> opCodes)
  {
    CInstruction* opCode = new CInstruction*(opCodes.size(), CreateStackResetCycles(new ResetVector(), &CW65C816::RES), "RES",
                                         "Reset the CPU.");
    opCodes.add(opCode);
    return opCode;
  }

  CInstruction* CreateIRQ(List<CInstruction*> opCodes)
  {
    CInstruction* opCode = new CInstruction*(opCodes.size(), CreateStackHardwareInterruptCycles(new IRQVector(), &CW65C816::IRQ), "IRQ",
                                         "Interrupt request.");
    opCodes.add(opCode);
    return opCode;
  }

  CInstruction* CreateNMI(List<CInstruction*> opCodes)
  {
    CInstruction* opCode = new CInstruction*(opCodes.size(), CreateStackHardwareInterruptCycles(new NMIVector(), &CW65C816::NMI), "NMI",
                                         "Non-maskable interrupt.");
    opCodes.add(opCode);
    return opCode;
  }

  CInstruction* CreateAbort(List<CInstruction*> opCodes)
  {
    CInstruction* opCode = new CInstruction*(opCodes.size(), CreateStackAbortInterruptCycles(new AbortVector(), &CW65C816::ABORT), "ABORT",
                                         "Stop the current instruction and return processor status to what it was prior to the current instruction.");
    opCodes.add(opCode);
    return opCode;
  }

  CInstruction* CreateFetchNext(List<CInstruction*> opCodes)
  {
    CInstruction* opCode = new CInstruction*(opCodes.size(), CreateFetchOpCodeCycles(), "NEXT",
                                         "Fetch Opcode from address in program counter.");
    opCodes.add(opCode);
    return opCode;
  }
}




};


#endif // __INSTRUCTION_FACTORY_H__

