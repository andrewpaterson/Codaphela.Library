#include "BRKVector.h"
#include "COPVector.h"
#include "NMIVector.h"
#include "IRQVector.h"
#include "AbortVector.h"
#include "ResetVector.h"
#include "InstructionFactory.h"


CInstructionFactory	gcInstructionFactory;
bool				gbInstructionFactoryInitialised = false;


#define INSTRUCTION(uiCode, cycles, name, description) \
{ \
	CInstruction*	pcInstruction; \
	pcInstruction = NewMalloc<CInstruction>(); \
	pcInstruction->Init(uiCode, cycles, name, description); \
	return pcInstruction; \
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstructionFactory* CInstructionFactory::GetInstance(void)
{
	if (gbInstructionFactoryInitialised)
	{
		return &gcInstructionFactory;
	}
	else
	{
		gcInstructionFactory.Init();
		gbInstructionFactoryInitialised = true;
		return &gcInstructionFactory;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstructionFactory::Init(void)
{
    memset(this, 0, sizeof(CInstructionFactory));

	CreateInstructions();
    ValidateOpCodes();

	CreateReset();
	CreateIRQ();
	CreateNMI();
	CreateAbort();
    CreateFetchNext();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstructionFactory::Kill(void)
{
    uint16          i;
    CInstruction*   pcInstruction;

    for (i = 0; i <= 255; i++)
    {
        pcInstruction = mapcInstructions[i];
        SafeKill(pcInstruction);
    }

    SafeKill(mpcReset);
    SafeKill(mpcIRQ);
    SafeKill(mpcNMI);
    SafeKill(mpcAbort);
    SafeKill(mpcFetchNext);

    gbInstructionFactoryInitialised = false;
}
 

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::GetInstruction(uint16 uiOpcode)
{
	return mapcInstructions[uiOpcode];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::GetReset(void)
{
	return mpcReset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::GetIRQ(void)
{
	return mpcIRQ;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::GetNMI(void)
{
	return mpcNMI;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::GetAbort(void)
{
	return mpcAbort;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::GetFetchNext(void)
{
	return mpcFetchNext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstructionFactory::CreateInstructions(void)
{
    CBRKVector* pcBRKVector;
    CCOPVector* pcCOPVector;

    pcBRKVector = NewMalloc<CBRKVector>();
    pcBRKVector->Init();
    pcCOPVector = NewMalloc<CCOPVector>();
    pcCOPVector->Init();

    mapcInstructions[0] = CreateBRK(BRK_Interrupt, CreateStackSoftwareInterruptCycles(pcBRKVector, &CW65C816::BRK));
    mapcInstructions[1] = CreateORA(ORA_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::ORA));
    mapcInstructions[2] = CreateCOP(COP_Interrupt, CreateStackSoftwareInterruptCycles(pcCOPVector, &CW65C816::COP));
    mapcInstructions[3] = CreateORA(ORA_StackRelative, CreateStackRelativeCycles(&CW65C816::ORA));
    mapcInstructions[4] = CreateTSB(TSB_DirectPage, CreateDirectRMWCycles(&CW65C816::TSB));
    mapcInstructions[5] = CreateORA(ORA_DirectPage, CreateDirectCycles(&CW65C816::ORA, WFR_M));
    mapcInstructions[6] = CreateASL(ASL_DirectPage, CreateDirectRMWCycles(&CW65C816::ASL));
    mapcInstructions[7] = CreateORA(ORA_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::ORA));
    mapcInstructions[8] = CreatePHP(PHP_StackImplied, CreateStackImpliedPHPCycles(&CW65C816::PHP));
    mapcInstructions[9] = CreateORA(ORA_Immediate, CreateImmediateCycles(&CW65C816::ORA, WFR_M));
    mapcInstructions[10] = CreateSLA(ASL_Accumulator, CreateAccumulatorCycles(&CW65C816::ASL_A));
    mapcInstructions[11] = CreatePHD(PHD_StackImplied, CreateStackPHDCycles(&CW65C816::PHD));
    mapcInstructions[12] = CreateTSB(TSB_Absolute, CreateAbsoluteRMWCycles(&CW65C816::TSB));
    mapcInstructions[13] = CreateORA(ORA_Absolute, CreateAbsoluteCycles(&CW65C816::ORA, WFR_M));
    mapcInstructions[14] = CreateASL(ASL_Absolute, CreateAbsoluteRMWCycles(&CW65C816::ASL));
    mapcInstructions[15] = CreateORA(ORA_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::ORA));
    mapcInstructions[16] = CreateBPL(BPL_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BPL));
    mapcInstructions[17] = CreateORA(ORA_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::ORA));
    mapcInstructions[18] = CreateORA(ORA_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::ORA));
    mapcInstructions[19] = CreateORA(ORA_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::ORA));
    mapcInstructions[20] = CreateTRB(TRB_DirectPage, CreateDirectRMWCycles(&CW65C816::TRB));
    mapcInstructions[21] = CreateORA(ORA_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::ORA, WFR_M));
    mapcInstructions[22] = CreateASL(ASL_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::ASL));
    mapcInstructions[23] = CreateORA(ORA_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::ORA));
    mapcInstructions[24] = CreateCLC(CLC_Implied, CreateImpliedCycles(&CW65C816::CLC));
    mapcInstructions[25] = CreateORA(ORA_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::ORA, WFR_M));
    mapcInstructions[26] = CreateINC_A(INC_Accumulator, CreateAccumulatorCycles(&CW65C816::INC_A));
    mapcInstructions[27] = CreateTCS(TCS_Implied, CreateImpliedCycles(&CW65C816::TCS));
    mapcInstructions[28] = CreateTRB(TRB_Absolute, CreateAbsoluteRMWCycles(&CW65C816::TRB));
    mapcInstructions[29] = CreateORA(ORA_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::ORA, WFR_M));
    mapcInstructions[30] = CreateASL(ASL_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::ASL));
    mapcInstructions[31] = CreateORA(ORA_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::ORA));
    mapcInstructions[32] = CreateJSR(JSR_Absolute, CreateAbsoluteJSRCycles());
    mapcInstructions[33] = CreateAND(AND_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::AND));
    mapcInstructions[34] = CreateJSL(JSL_AbsoluteLong, CreateAbsoluteLongJSLCycles());
    mapcInstructions[35] = CreateAND(AND_StackRelative, CreateStackRelativeCycles(&CW65C816::AND));
    mapcInstructions[36] = CreateBIT(BIT_DirectPage, CreateDirectCycles(&CW65C816::BIT, WFR_M));
    mapcInstructions[37] = CreateAND(AND_DirectPage, CreateDirectCycles(&CW65C816::AND, WFR_M));
    mapcInstructions[38] = CreateROL(ROL_DirectPage, CreateDirectRMWCycles(&CW65C816::ROL));
    mapcInstructions[39] = CreateAND(AND_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::AND));
    mapcInstructions[40] = CreatePLP(PLP_StackImplied, CreateStackPLPCycles(&CW65C816::PLP));
    mapcInstructions[41] = CreateAND(AND_Immediate, CreateImmediateCycles(&CW65C816::AND, WFR_M));
    mapcInstructions[42] = CreateRLA(ROL_Accumulator, CreateAccumulatorCycles(&CW65C816::ROL_A));
    mapcInstructions[43] = CreatePLD(PLD_StackImplied, CreateStackPLDCycles(&CW65C816::PLD));
    mapcInstructions[44] = CreateBIT(BIT_Absolute, CreateAbsoluteCycles(&CW65C816::BIT, WFR_M));
    mapcInstructions[45] = CreateAND(AND_Absolute, CreateAbsoluteCycles(&CW65C816::AND, WFR_M));
    mapcInstructions[46] = CreateROL(ROL_Absolute, CreateAbsoluteRMWCycles(&CW65C816::ROL));
    mapcInstructions[47] = CreateAND(AND_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::AND));
    mapcInstructions[48] = CreateBMI(BMI_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BMI));
    mapcInstructions[49] = CreateAND(AND_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::AND));
    mapcInstructions[50] = CreateAND(AND_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::AND));
    mapcInstructions[51] = CreateAND(AND_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::AND));
    mapcInstructions[52] = CreateBIT(BIT_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::BIT, WFR_M));
    mapcInstructions[53] = CreateAND(AND_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::AND, WFR_M));
    mapcInstructions[54] = CreateROL(ROL_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::ROL));
    mapcInstructions[55] = CreateAND(AND_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::AND));
    mapcInstructions[56] = CreateSEC(SEC_Implied, CreateImpliedCycles(&CW65C816::SEC));
    mapcInstructions[57] = CreateAND(AND_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::AND, WFR_M));
    mapcInstructions[58] = CreateDEC_A(DEC_Accumulator, CreateAccumulatorCycles(&CW65C816::DEC_A));
    mapcInstructions[59] = CreateTSC(TSC_Implied, CreateImpliedCycles(&CW65C816::TSC));
    mapcInstructions[60] = CreateBIT(BIT_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::BIT, WFR_M));
    mapcInstructions[61] = CreateAND(AND_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::AND, WFR_M));
    mapcInstructions[62] = CreateROL(ROL_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::ROL));
    mapcInstructions[63] = CreateAND(AND_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::AND));
    mapcInstructions[64] = CreateRTI(RTI_StackImplied, CreateStackRTICycles());
    mapcInstructions[65] = CreateEOR(EOR_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::EOR));
    mapcInstructions[66] = CreateWDM(WDM_Implied, CreateWDMImpliedCycles(&CW65C816::WDM));
    mapcInstructions[67] = CreateEOR(EOR_StackRelative, CreateStackRelativeCycles(&CW65C816::EOR));
    mapcInstructions[68] = CreateMVP(MVP_BlockMove, CreateBlockMoveCycles(&CW65C816::MVP));
    mapcInstructions[69] = CreateEOR(EOR_DirectPage, CreateDirectCycles(&CW65C816::EOR, WFR_M));
    mapcInstructions[70] = CreateLSR(LSR_DirectPage, CreateDirectRMWCycles(&CW65C816::LSR));
    mapcInstructions[71] = CreateEOR(EOR_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::EOR));
    mapcInstructions[72] = CreatePHA(PHA_StackImplied, CreateStackPushCycles(&CW65C816::PHA, WFR_M));
    mapcInstructions[73] = CreateEOR(EOR_Immediate, CreateImmediateCycles(&CW65C816::EOR, WFR_M));
    mapcInstructions[74] = CreateSRA(LSR_Accumulator, CreateAccumulatorCycles(&CW65C816::LSR_A));
    mapcInstructions[75] = CreatePHK(PHK_StackImplied, CreateStackPHKCycles(&CW65C816::PHK));
    mapcInstructions[76] = CreateJMP(JMP_Absolute, CreateAbsoluteJMPCycles());
    mapcInstructions[77] = CreateEOR(EOR_Absolute, CreateAbsoluteCycles(&CW65C816::EOR, WFR_M));
    mapcInstructions[78] = CreateLSR(LSR_Absolute, CreateAbsoluteRMWCycles(&CW65C816::LSR));
    mapcInstructions[79] = CreateEOR(EOR_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::EOR));
    mapcInstructions[80] = CreateBVC(BVC_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BVC));
    mapcInstructions[81] = CreateEOR(EOR_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::EOR));
    mapcInstructions[82] = CreateEOR(EOR_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::EOR));
    mapcInstructions[83] = CreateEOR(EOR_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::EOR));
    mapcInstructions[84] = CreateMVN(MVN_BlockMove, CreateBlockMoveCycles(&CW65C816::MVN));
    mapcInstructions[85] = CreateEOR(EOR_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::EOR, WFR_M));
    mapcInstructions[86] = CreateLSR(LSR_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::LSR));
    mapcInstructions[87] = CreateEOR(EOR_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::EOR));
    mapcInstructions[88] = CreateCLI(CLI_Implied, CreateImpliedCycles(&CW65C816::CLI));
    mapcInstructions[89] = CreateEOR(EOR_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::EOR, WFR_M));
    mapcInstructions[90] = CreatePHY(PHY_StackImplied, CreateStackPushCycles(&CW65C816::PHY, WFR_XY));
    mapcInstructions[91] = CreateTCD(TCD_Implied, CreateImpliedCycles(&CW65C816::TCD));
    mapcInstructions[92] = CreateJMP(JML_AbsoluteLong, CreateAbsoluteLongJMLCycles());
    mapcInstructions[93] = CreateEOR(EOR_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::EOR, WFR_M));
    mapcInstructions[94] = CreateLSR(LSR_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::LSR));
    mapcInstructions[95] = CreateEOR(EOR_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::EOR));
    mapcInstructions[96] = CreateRTS(RTS_StackImplied, CreateStackRTSCycles());
    mapcInstructions[97] = CreateADC(ADC_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::ADC));
    mapcInstructions[98] = CreatePER(PER_StackProgramCounterRelativeLong, CreateStackPERCycles(&CW65C816::PER));
    mapcInstructions[99] = CreateADC(ADC_StackRelative, CreateStackRelativeCycles(&CW65C816::ADC));
    mapcInstructions[100] = CreateSTZ(STZ_DirectPage, CreateDirectWriteCycles(&CW65C816::STZ, WFR_M));
    mapcInstructions[101] = CreateADC(ADC_DirectPage, CreateDirectCycles(&CW65C816::ADC, WFR_M));
    mapcInstructions[102] = CreateROR(ROR_DirectPage, CreateDirectRMWCycles(&CW65C816::ROR));
    mapcInstructions[103] = CreateADC(ADC_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::ADC));
    mapcInstructions[104] = CreatePLA(PLA_Stack, CreateStackPullCycles(&CW65C816::PLA, WFR_M));
    mapcInstructions[105] = CreateADC(ADC_Immediate, CreateImmediateCycles(&CW65C816::ADC, WFR_M));
    mapcInstructions[106] = CreateRRA(ROR_Accumulator, CreateAccumulatorCycles(&CW65C816::ROR_A));
    mapcInstructions[107] = CreateRTL(RTL_StackImplied, CreateStackRTLCycles());
    mapcInstructions[108] = CreateJMP(JMP_AbsoluteIndirect, CreateAbsoluteIndirectJMPCycles());
    mapcInstructions[109] = CreateADC(ADC_Absolute, CreateAbsoluteCycles(&CW65C816::ADC, WFR_M));
    mapcInstructions[110] = CreateROR(ROR_Absolute, CreateAbsoluteRMWCycles(&CW65C816::ROR));
    mapcInstructions[111] = CreateADC(ADC_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::ADC));
    mapcInstructions[112] = CreateBVS(BVS_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BVS));
    mapcInstructions[113] = CreateADC(ADC_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::ADC));
    mapcInstructions[114] = CreateADC(ADC_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::ADC));
    mapcInstructions[115] = CreateADC(ADC_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::ADC));
    mapcInstructions[116] = CreateSTZ(STZ_DirectPageIndexedWithX, CreateDirectIndexedWithXWriteCycles(&CW65C816::STZ, WFR_M));
    mapcInstructions[117] = CreateADC(ADC_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::ADC, WFR_M));
    mapcInstructions[118] = CreateROR(ROR_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::ROR));
    mapcInstructions[119] = CreateADC(ADC_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::ADC));
    mapcInstructions[120] = CreateSEI(SEI_Implied, CreateImpliedCycles(&CW65C816::SEI));
    mapcInstructions[121] = CreateADC(ADC_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::ADC, WFR_M));
    mapcInstructions[122] = CreatePLY(PLY_StackImplied, CreateStackPullCycles(&CW65C816::PLY, WFR_XY));
    mapcInstructions[123] = CreateTDC(TDC_Implied, CreateImpliedCycles(&CW65C816::TDC));
    mapcInstructions[124] = CreateJMP(JMP_AbsoluteIndexedIndirectWithX, CreateAbsoluteIndexedIndirectWithXJMPCycles());
    mapcInstructions[125] = CreateADC(ADC_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::ADC, WFR_M));
    mapcInstructions[126] = CreateROR(ROR_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::ROR));
    mapcInstructions[127] = CreateADC(ADC_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::ADC));
    mapcInstructions[128] = CreateBRA(BRA_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BRA));
    mapcInstructions[129] = CreateSTA(STA_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXWriteCycles(&CW65C816::STA));
    mapcInstructions[130] = CreateBRL(BRL_ProgramCounterRelativeLong, CreateRelativeLongCycles(&CW65C816::BRA));
    mapcInstructions[131] = CreateSTA(STA_StackRelative, CreateStackRelativeWriteCycles(&CW65C816::STA));
    mapcInstructions[132] = CreateSTY(STY_DirectPage, CreateDirectWriteCycles(&CW65C816::STY, WFR_XY));
    mapcInstructions[133] = CreateSTA(STA_DirectPage, CreateDirectWriteCycles(&CW65C816::STA, WFR_M));
    mapcInstructions[134] = CreateSTX(STX_DirectPage, CreateDirectWriteCycles(&CW65C816::STX, WFR_XY));
    mapcInstructions[135] = CreateSTA(STA_DirectPageIndirectLong, CreateDirectIndirectLongWriteCycles(&CW65C816::STA));
    mapcInstructions[136] = CreateDEY(DEY_Implied, CreateImpliedCycles(&CW65C816::DEY));
    mapcInstructions[137] = CreateBIT(BIT_Immediate, CreateImmediateCycles(&CW65C816::BIT_I, WFR_M));
    mapcInstructions[138] = CreateTXA(TXA_Implied, CreateImpliedCycles(&CW65C816::TXA));
    mapcInstructions[139] = CreatePHB(PHB_StackImplied, CreateStackPHBCycles(&CW65C816::PHB));
    mapcInstructions[140] = CreateSTY(STY_Absolute, CreateAbsoluteWriteCycles(&CW65C816::STY, WFR_XY));
    mapcInstructions[141] = CreateSTA(STA_Absolute, CreateAbsoluteWriteCycles(&CW65C816::STA, WFR_M));
    mapcInstructions[142] = CreateSTX(STX_Absolute, CreateAbsoluteWriteCycles(&CW65C816::STX, WFR_XY));
    mapcInstructions[143] = CreateSTA(STA_AbsoluteLong, CreateAbsoluteLongWriteCycles(&CW65C816::STA));
    mapcInstructions[144] = CreateBCC(BCC_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BCC));
    mapcInstructions[145] = CreateSTA(STA_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYWriteCycles(&CW65C816::STA));
    mapcInstructions[146] = CreateSTA(STA_DirectPageIndirect, CreateDirectIndirectWriteCycles(&CW65C816::STA));
    mapcInstructions[147] = CreateSTA(STA_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYWriteCycles(&CW65C816::STA));
    mapcInstructions[148] = CreateSTY(STY_DirectPageIndexedWithX, CreateDirectIndexedWithXWriteCycles(&CW65C816::STY, WFR_XY));
    mapcInstructions[149] = CreateSTA(STA_DirectPageIndexedWithX, CreateDirectIndexedWithXWriteCycles(&CW65C816::STA, WFR_M));
    mapcInstructions[150] = CreateSTX(STX_DirectPageIndexedWithY, CreateDirectIndexedWithYWriteCycles(&CW65C816::STX, WFR_XY));
    mapcInstructions[151] = CreateSTA(STA_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYWriteCycles(&CW65C816::STA));
    mapcInstructions[152] = CreateTYA(TYA_Implied, CreateImpliedCycles(&CW65C816::TYA));
    mapcInstructions[153] = CreateSTA(STA_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYWriteCycles(&CW65C816::STA));
    mapcInstructions[154] = CreateTXS(TXS_Implied, CreateImpliedCycles(&CW65C816::TXS));
    mapcInstructions[155] = CreateTXY(TXY_Implied, CreateImpliedCycles(&CW65C816::TXY));
    mapcInstructions[156] = CreateSTZ(STZ_Absolute, CreateAbsoluteWriteCycles(&CW65C816::STZ, WFR_M));
    mapcInstructions[157] = CreateSTA(STA_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXWriteCycles(&CW65C816::STA));
    mapcInstructions[158] = CreateSTZ(STZ_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXWriteCycles(&CW65C816::STZ));
    mapcInstructions[159] = CreateSTA(STA_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXWriteCycles(&CW65C816::STA));
    mapcInstructions[160] = CreateLDY(LDY_Immediate, CreateImmediateCycles(&CW65C816::LDY, WFR_XY));
    mapcInstructions[161] = CreateLDA(LDA_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::LDA));
    mapcInstructions[162] = CreateLDX(LDX_Immediate, CreateImmediateCycles(&CW65C816::LDX, WFR_XY));
    mapcInstructions[163] = CreateLDA(LDA_StackRelative, CreateStackRelativeCycles(&CW65C816::LDA));
    mapcInstructions[164] = CreateLDY(LDY_DirectPage, CreateDirectCycles(&CW65C816::LDY, WFR_XY));
    mapcInstructions[165] = CreateLDA(LDA_DirectPage, CreateDirectCycles(&CW65C816::LDA, WFR_M));
    mapcInstructions[166] = CreateLDX(LDX_DirectPage, CreateDirectCycles(&CW65C816::LDX, WFR_XY));
    mapcInstructions[167] = CreateLDA(LDA_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::LDA));
    mapcInstructions[168] = CreateTAY(TAY_Implied, CreateImpliedCycles(&CW65C816::TAY));
    mapcInstructions[169] = CreateLDA(LDA_Immediate, CreateImmediateCycles(&CW65C816::LDA, WFR_M));
    mapcInstructions[170] = CreateTAX(TAX_Implied, CreateImpliedCycles(&CW65C816::TAX));
    mapcInstructions[171] = CreatePLB(PLB_StackImplied, CreateStackPLBCycles(&CW65C816::PLB));
    mapcInstructions[172] = CreateLDY(LDY_Absolute, CreateAbsoluteCycles(&CW65C816::LDY, WFR_XY));
    mapcInstructions[173] = CreateLDA(LDA_Absolute, CreateAbsoluteCycles(&CW65C816::LDA, WFR_M));
    mapcInstructions[174] = CreateLDX(LDX_Absolute, CreateAbsoluteCycles(&CW65C816::LDX, WFR_XY));
    mapcInstructions[175] = CreateLDA(LDA_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::LDA));
    mapcInstructions[176] = CreateBCS(BCS_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BCS));
    mapcInstructions[177] = CreateLDA(LDA_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::LDA));
    mapcInstructions[178] = CreateLDA(LDA_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::LDA));
    mapcInstructions[179] = CreateLDA(LDA_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::LDA));
    mapcInstructions[180] = CreateLDY(LDY_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::LDY, WFR_XY));
    mapcInstructions[181] = CreateLDA(LDA_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::LDA, WFR_M));
    mapcInstructions[182] = CreateLDX(LDX_DirectPageIndexedWithY, CreateDirectIndexedWithYCycles(&CW65C816::LDX, WFR_XY));
    mapcInstructions[183] = CreateLDA(LDA_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::LDA));
    mapcInstructions[184] = CreateCLV(CLV_Implied, CreateImpliedCycles(&CW65C816::CLV));
    mapcInstructions[185] = CreateLDA(LDA_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::LDA, WFR_M));
    mapcInstructions[186] = CreateTSX(TSX_Implied, CreateImpliedCycles(&CW65C816::TSX));
    mapcInstructions[187] = CreateTYX(TYX_Implied, CreateImpliedCycles(&CW65C816::TYX));
    mapcInstructions[188] = CreateLDY(LDY_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::LDY, WFR_XY));
    mapcInstructions[189] = CreateLDA(LDA_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::LDA, WFR_M));
    mapcInstructions[190] = CreateLDX(LDX_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::LDX, WFR_XY));
    mapcInstructions[191] = CreateLDA(LDA_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::LDA));
    mapcInstructions[192] = CreateCPY(CPY_Immediate, CreateImmediateCycles(&CW65C816::CPY, WFR_XY));
    mapcInstructions[193] = CreateCMP(CMP_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::CMP));
    mapcInstructions[194] = CreateREP(REP_Immediate, CreateImmediateREPSEPCycles(&CW65C816::REP));
    mapcInstructions[195] = CreateCMP(CMP_StackRelative, CreateStackRelativeCycles(&CW65C816::CMP));
    mapcInstructions[196] = CreateCPY(CPY_DirectPage, CreateDirectCycles(&CW65C816::CPY, WFR_XY));
    mapcInstructions[197] = CreateCMP(CMP_DirectPage, CreateDirectCycles(&CW65C816::CMP, WFR_M));
    mapcInstructions[198] = CreateDEC(DEC_DirectPage, CreateDirectRMWCycles(&CW65C816::DEC));
    mapcInstructions[199] = CreateCMP(CMP_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::CMP));
    mapcInstructions[200] = CreateINY(INY_Implied, CreateImpliedCycles(&CW65C816::INY));
    mapcInstructions[201] = CreateCMP(CMP_Immediate, CreateImmediateCycles(&CW65C816::CMP, WFR_M));
    mapcInstructions[202] = CreateDEX(DEX_Implied, CreateImpliedCycles(&CW65C816::DEX));
    mapcInstructions[203] = CreateWAI(WAI_Implied, CreateWaitForInterruptCycles());
    mapcInstructions[204] = CreateCPY(CPY_Absolute, CreateAbsoluteCycles(&CW65C816::CPY, WFR_XY));
    mapcInstructions[205] = CreateCMP(CMP_Absolute, CreateAbsoluteCycles(&CW65C816::CMP, WFR_M));
    mapcInstructions[206] = CreateDEC(DEC_Absolute, CreateAbsoluteRMWCycles(&CW65C816::DEC));
    mapcInstructions[207] = CreateCMP(CMP_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::CMP));
    mapcInstructions[208] = CreateBNE(BNE_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BNE));
    mapcInstructions[209] = CreateCMP(CMP_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::CMP));
    mapcInstructions[210] = CreateCMP(CMP_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::CMP));
    mapcInstructions[211] = CreateCMP(CMP_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::CMP));
    mapcInstructions[212] = CreatePEI(PEI_StackDirectPageIndirect, CreateStackPEICycles());
    mapcInstructions[213] = CreateCMP(CMP_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::CMP, WFR_M));
    mapcInstructions[214] = CreateDEC(DEC_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::DEC));
    mapcInstructions[215] = CreateCMP(CMP_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::CMP));
    mapcInstructions[216] = CreateCLD(CLD_Implied, CreateImpliedCycles(&CW65C816::CLD));
    mapcInstructions[217] = CreateCMP(CMP_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::CMP, WFR_M));
    mapcInstructions[218] = CreatePHX(PHX_StackImplied, CreateStackPushCycles(&CW65C816::PHX, WFR_XY));
    mapcInstructions[219] = CreateSTP(STP_Implied, CreateStopTheClockCycles(&CW65C816::STP));
    mapcInstructions[220] = CreateJMP(JML_AbsoluteIndirectLong, CreateAbsoluteIndirectJMLCycles());
    mapcInstructions[221] = CreateCMP(CMP_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::CMP, WFR_M));
    mapcInstructions[222] = CreateDEC(DEC_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::DEC));
    mapcInstructions[223] = CreateCMP(CMP_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::CMP));
    mapcInstructions[224] = CreateCPX(CPX_Immediate, CreateImmediateCycles(&CW65C816::CPX, WFR_XY));
    mapcInstructions[225] = CreateSBC(SBC_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::SBC));
    mapcInstructions[226] = CreateSEP(SEP_Immediate, CreateImmediateREPSEPCycles(&CW65C816::SEP));
    mapcInstructions[227] = CreateSBC(SBC_StackRelative, CreateStackRelativeCycles(&CW65C816::SBC));
    mapcInstructions[228] = CreateCPX(CPX_DirectPage, CreateDirectCycles(&CW65C816::CPX, WFR_XY));
    mapcInstructions[229] = CreateSBC(SBC_DirectPage, CreateDirectCycles(&CW65C816::SBC, WFR_M));
    mapcInstructions[230] = CreateINC(INC_DirectPage, CreateDirectRMWCycles(&CW65C816::INC));
    mapcInstructions[231] = CreateSBC(SBC_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::SBC));
    mapcInstructions[232] = CreateINX(INX_Implied, CreateImpliedCycles(&CW65C816::INX));
    mapcInstructions[233] = CreateSBC(SBC_Immediate, CreateImmediateCycles(&CW65C816::SBC, WFR_M));
    mapcInstructions[234] = CreateNOP(NOP_Implied, CreateImpliedCycles(&CW65C816::NOP));
    mapcInstructions[235] = CreateXBA(XBA_Implied, CreateImpliedXBACycles(&CW65C816::XBA));
    mapcInstructions[236] = CreateCPX(CPX_Absolute, CreateAbsoluteCycles(&CW65C816::CPX, WFR_XY));
    mapcInstructions[237] = CreateSBC(SBC_Absolute, CreateAbsoluteCycles(&CW65C816::SBC, WFR_M));
    mapcInstructions[238] = CreateINC(INC_Absolute, CreateAbsoluteRMWCycles(&CW65C816::INC));
    mapcInstructions[239] = CreateSBC(SBC_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::SBC));
    mapcInstructions[240] = CreateBEQ(BEQ_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BEQ));
    mapcInstructions[241] = CreateSBC(SBC_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::SBC));
    mapcInstructions[242] = CreateSBC(SBC_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::SBC));
    mapcInstructions[243] = CreateSBC(SBC_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::SBC));
    mapcInstructions[244] = CreatePEA(PEA_StackImmediate, CreateStackPEACycles());
    mapcInstructions[245] = CreateSBC(SBC_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::SBC, WFR_M));
    mapcInstructions[246] = CreateINC(INC_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::INC));
    mapcInstructions[247] = CreateSBC(SBC_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::SBC));
    mapcInstructions[248] = CreateSED(SED_Implied, CreateImpliedCycles(&CW65C816::SED));
    mapcInstructions[249] = CreateSBC(SBC_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::SBC, WFR_M));
    mapcInstructions[250] = CreatePLX(PLX_StackImplied, CreateStackPullCycles(&CW65C816::PLX, WFR_XY));
    mapcInstructions[251] = CreateXCE(XCE_Implied, CreateImpliedCycles(&CW65C816::XCE));
    mapcInstructions[252] = CreateJSR(JSR_AbsoluteIndexedIndirectWithX, CreateAbsoluteIndexedIndirectWithXJSRCycles());
    mapcInstructions[253] = CreateSBC(SBC_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::SBC, WFR_M));
    mapcInstructions[254] = CreateINC(INC_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::INC));
    mapcInstructions[255] = CreateSBC(SBC_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::SBC));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstructionFactory::ValidateOpCodes(void)
{
    uint16          i;
    CInstruction*   pcInstruction;

    for (i = 0; i <= 255; i++)
    {
        pcInstruction = mapcInstructions[i];
        if (pcInstruction->GetCode() != i)
        {

            gcLogger.Error2(__METHOD__, " OpCode [", pcInstruction->GetName(), "] has uiCode [", ShortToString(pcInstruction->GetCode()), "] but is at index [", ShortToString(i), "].", NULL);
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateBEQ(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "BEQ",
        "Branch if Equal (Z=1)");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateXBA(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "XBA",
        "Exchange B and A Accumulator");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateNOP(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "NOP",
        "No Operation for two cycles.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateINX(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "INX",
        "Increment Index X by One");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateSEP(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "SEP",
        "Set Processor Status Bit");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateBNE(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "BNE",
        "Branch if Not Equal (Z=0)");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateWAI(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "WAI",
        "Wait for Interrupt");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateDEX(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "DEX",
        "Decrement Index X by One.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateINY(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "INY",
        "Increment Index Y by One");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreatePEA(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "PEA",
        "Push Absolute Address");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateINC(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "INC",
        "Increment memory; result in memory and update NZ.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateXCE(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "XCE",
        "Exchange Carry and Emulation Bits");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreatePLX(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "PLX",
        "Pull Index X from Stack");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateSED(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles,
        "SED", "Set Decimal Mode");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateSBC(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "SBC",
        "Subtract memory and carry from A; result in A and update NZC.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateCPX(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "CPX",
        "Compare Memory and Index X");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateSTP(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "STP",
        "Stop the Clock");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreatePHX(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "PHX",
        "Push Index X on Stack");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateCLD(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "CLD",
        "Clear Decimal Mode");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreatePEI(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "PEI",
        "Push Indirect Address");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateDEC(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "DEC",
        "Decrement memory; result in memory and update NZ.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateREP(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "REP",
        "Reset Status Bits");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateCMP(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "CMP",
        "Compare Memory and Accumulator");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateCPY(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "CPY",
        "Compare Memory and Index Y");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateTYX(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "TYX",
        "Transfer Index Y to Index X");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateTSX(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "TSX",
        "Transfer Stack Pointer Register to Index X");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateCLV(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "CLV",
        "Clear Overflow Flag");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateBCS(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "BCS",
        "Branch on Carry Set (C=1)");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreatePLB(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "PLB",
        "Pull Data Bank Register from Stack");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateTAX(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "TAX",
        "Transfer Accumulator in Index X");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateTAY(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "TAY",
        "Transfer Accumulator in Index Y");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateLDX(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "LDX", "Load Index X with Memory");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateBRL(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "BRL",
        "Branch Always Long");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateBRA(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "BRA",
        "Branch Always");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateTDC(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "TDC",
        "Transfer Direct Register to C Accumulator");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreatePLY(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "PLY",
        "Pull Index Y from Stack");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateSEI(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "SEI",
        "Set Interrupt Disable Status");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateBVS(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "BVS", "Branch on Overflow Set (V=1)");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateRTL(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "RTL",
        "Return from Subroutine Long");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateRRA(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "ROR",
        "Rotate accumulator right one bit; update NZC.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreatePLA(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "PLA",
        "Pull Accumulator from Stack");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateROR(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "ROR",
        "Rotate memory right one bit; update NZC.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreatePER(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "PER",
        "Push Program Counter Relative Address");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateADC(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "ADC",
        "Add memory and carry to A; result in A and update NZC.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateRTS(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "RTS",
        "Return from Subroutine");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateTCD(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "TCD",
        "Transfer C Accumulator to Direct Register");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreatePHY(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "PHY",
        "Push Index Y on Stack");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateCLI(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "CLI",
        "Clear Interrupt Disable Bit");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateMVN(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "MVN",
        "Block move next...");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateBVC(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "BVC",
        "Branch on Overflow Clear (V=0)");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateJMP(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "JMP",
        "Jump to New Location");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreatePHK(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "PHK",
        "Push Program Bank Register on Stack");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateSRA(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "LSR",
        "Shift accumulator right one bit; update NZC.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreatePHA(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "PHA",
        "Push Accumulator onto Stack");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateLSR(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles,
        "LSR", "Shift memory right one bit; update NZC.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateMVP(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "MVP",
        "Block move previous...");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateWDM(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "WDM",
        "Reserved for future use");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateEOR(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "EOR",
        "'Exclusive OR' Memory with Accumulator");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateRTI(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "RTI",
        "Return from Interrupt");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateTSC(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "TSC",
        "Transfer Stack Pointer to C Accumulator");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateDEC_A(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "DEC",
        "Decrement accumulator; update NZ.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateSEC(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "SEC",
        "Set Carry Flag");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateBMI(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "BMI",
        "Branch if Result Minus (N=1)");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreatePLP(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "PLP",
        "Pull Processor Status from Stack");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateRLA(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "ROL",
        "Rotate Accumulator One Bit Left.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreatePLD(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "PLD",
        "Pull Direct Register from Stack");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateROL(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "ROL",
        "Rotate Memory One Bit Left.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateLDA(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "LDA",
        "Load Accumulator with Memory");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateLDY(uint16 uiCode, CInstructionCycles* immediateCycles)
{
    INSTRUCTION(uiCode, immediateCycles, "LDY",
        "Load Index Y with Memory");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreatePHB(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "PHB",
        "Push Data Bank Register on Stack");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateTXA(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "TXA",
        "Transfer Index X to Accumulator");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateSTY(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "STY",
        "Store Index Y in Memory");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateBCC(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "BCC",
        "Branch on Carry Clear (C=0)");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateSTX(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "STX",
        "Store Index X in Memory");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateTYA(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "TYA",
        "Transfer Index Y to Accumulator");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateTXS(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "TXS",
        "Transfer Index X to Stack Pointer Register");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateTXY(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "TXY",
        "Transfer Index X to Index Y");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateSTZ(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles,
        "STZ", "Store Zero in Memory");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateSTA(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "STA",
        "Store Accumulator in Memory");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateDEY(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "DEY", "Decrement Index Y by One");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateBIT(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "BIT",
        "Bit Test");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateJSL(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "JSL",
        "Jump long to new location save return address on Stack.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateAND(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "AND",
        "Bitwise AND memory with A; result in A and update NZ.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateJSR(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "JSR",
        "Jump to new location save return address on Stack.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateTCS(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "TCS",
        "Transfer C Accumulator to Stack Pointer");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateINC_A(uint16 uiCode, CInstructionCycles* busCycles)
{
    INSTRUCTION(uiCode, busCycles, "INC",
        "Increment accumulator; update NZ.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateCLC(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "CLC",
        "Clear Carry Flag");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateTRB(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "TRB",
        "Test and Reset Bit");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateBPL(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "BPL",
        "Branch if Result Plus (N=0)");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreatePHD(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "PHD",
        "Push Direct Register on Stack");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateSLA(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "ASL",
        "Shift accumulator left one bit; update NZC.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateASL(uint16 uiCode, CInstructionCycles* directRMWCycles)
{
    INSTRUCTION(uiCode, directRMWCycles, "ASL", "Shift memory left 1 bit; result in memory and update NZC.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreatePHP(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "PHP", "Push Processor Status on Stack");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateTSB(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles,
        "TSB", "Test and Set Bit");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateCOP(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "COP",
        "Force co-processor software interrupt.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateORA(uint16 uiCode, CInstructionCycles* directIndexedIndirectWithXCycles)
{
    INSTRUCTION(uiCode, directIndexedIndirectWithXCycles,
        "ORA", "'OR' memory with A; result in A and update NZ.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateBRK(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "BRK",
        "Force break software interrupt.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstructionFactory::CreateReset(void)
{
    CResetVector* pcVector;

    pcVector = NewMalloc<CResetVector>();
    pcVector->Init();

    mpcReset = NewMalloc<CInstruction>();
    mpcReset->Init(256, CreateStackResetCycles(pcVector, &CW65C816::RES), "RES",
        "Reset the CPU.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstructionFactory::CreateIRQ(void)
{
    CIRQVector* pcVector;

    pcVector = NewMalloc<CIRQVector>();
    pcVector->Init();

    mpcIRQ = NewMalloc<CInstruction>();
    mpcIRQ->Init(256, CreateStackHardwareInterruptCycles(pcVector, &CW65C816::IRQ), "IRQ",
        "Interrupt request.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstructionFactory::CreateNMI(void)
{
    CNMIVector* pcVector;

    pcVector = NewMalloc<CNMIVector>();
    pcVector->Init();

    mpcNMI = NewMalloc<CInstruction>();
    mpcNMI->Init(256, CreateStackHardwareInterruptCycles(pcVector, &CW65C816::NMI), "NMI",
        "Non-maskable interrupt.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstructionFactory::CreateAbort(void)
{
    CNMIVector* pcVector;

    pcVector = NewMalloc<CNMIVector>();
    pcVector->Init();

    mpcAbort = NewMalloc<CInstruction>();
    mpcAbort->Init(256, CreateStackAbortInterruptCycles(pcVector, &CW65C816::ABORT), "ABORT",
        "Stop the current instruction and return processor status to what it was prior to the current instruction.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstructionFactory::CreateFetchNext(void)
{
    mpcFetchNext = NewMalloc<CInstruction>();
    mpcFetchNext->Init(257, CreateFetchOpCodeCycles(), "NEXT",
        "Fetch Opcode from address in program counter.");
}

