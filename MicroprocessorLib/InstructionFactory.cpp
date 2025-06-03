#include "BRKVector.h"
#include "COPVector.h"
#include "NMIVector.h"
#include "IRQVector.h"
#include "AbortVector.h"
#include "ResetVector.h"
#include "InstructionFactory.h"


CInstructionFactory	gcInstructionFactory;


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
	return &gcInstructionFactory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstructionFactory::Init(void)
{
    memset(this, 0, sizeof(CInstructionFactory));

	CreateInstructions();
    ValidateOpcodes();

	Instruct(CreateReset());
    Instruct(CreateIRQ());
    Instruct(CreateNMI());
    Instruct(CreateAbort());
    Instruct(CreateFetchNext());
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
}
 

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::GetInstruction(uint16 uiOpcode)
{
    if (uiOpcode < INSTRUCTIONS_SIZE)
    {
        return mapcInstructions[uiOpcode];
    }
    else
    {
        return NULL;
    }
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

    muiInstructions = 0;

    mapcInstructions[0] = Instruct(CreateBRK(BRK_Interrupt, CreateStackSoftwareInterruptCycles(pcBRKVector, &CW65C816::BRK, false)));
    mapcInstructions[1] = Instruct(CreateORA(ORA_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::ORA, false)));
    mapcInstructions[2] = Instruct(CreateCOP(COP_Interrupt, CreateStackSoftwareInterruptCycles(pcCOPVector, &CW65C816::COP, false)));
    mapcInstructions[3] = Instruct(CreateORA(ORA_StackRelative, CreateStackRelativeCycles(&CW65C816::ORA, false)));
    mapcInstructions[4] = Instruct(CreateTSB(TSB_DirectPage, CreateDirectRMWCycles(&CW65C816::TSB, true)));
    mapcInstructions[5] = Instruct(CreateORA(ORA_DirectPage, CreateDirectCycles(&CW65C816::ORA, WFR_M, false)));
    mapcInstructions[6] = Instruct(CreateASL(ASL_DirectPage, CreateDirectRMWCycles(&CW65C816::ASL, true)));
    mapcInstructions[7] = Instruct(CreateORA(ORA_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::ORA, false)));
    mapcInstructions[8] = Instruct(CreatePHP(PHP_StackImplied, CreateStackImpliedPHPCycles(&CW65C816::PHP, true)));
    mapcInstructions[9] = Instruct(CreateORA(ORA_Immediate, CreateImmediateCycles(&CW65C816::ORA, WFR_M, false)));
    mapcInstructions[10] = Instruct(CreateSLA(ASL_Accumulator, CreateAccumulatorCycles(&CW65C816::ASL_A, false)));
    mapcInstructions[11] = Instruct(CreatePHD(PHD_StackImplied, CreateStackPHDCycles(&CW65C816::PHD, true)));
    mapcInstructions[12] = Instruct(CreateTSB(TSB_Absolute, CreateAbsoluteRMWCycles(&CW65C816::TSB, true)));
    mapcInstructions[13] = Instruct(CreateORA(ORA_Absolute, CreateAbsoluteCycles(&CW65C816::ORA, WFR_M, false)));
    mapcInstructions[14] = Instruct(CreateASL(ASL_Absolute, CreateAbsoluteRMWCycles(&CW65C816::ASL, true)));
    mapcInstructions[15] = Instruct(CreateORA(ORA_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::ORA, false)));
    mapcInstructions[16] = Instruct(CreateBPL(BPL_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BPL, false)));
    mapcInstructions[17] = Instruct(CreateORA(ORA_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::ORA, false)));
    mapcInstructions[18] = Instruct(CreateORA(ORA_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::ORA, false)));
    mapcInstructions[19] = Instruct(CreateORA(ORA_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::ORA, false)));
    mapcInstructions[20] = Instruct(CreateTRB(TRB_DirectPage, CreateDirectRMWCycles(&CW65C816::TRB, true)));
    mapcInstructions[21] = Instruct(CreateORA(ORA_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::ORA, WFR_M, false)));
    mapcInstructions[22] = Instruct(CreateASL(ASL_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::ASL, true)));
    mapcInstructions[23] = Instruct(CreateORA(ORA_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::ORA, false)));
    mapcInstructions[24] = Instruct(CreateCLC(CLC_Implied, CreateImpliedCycles(&CW65C816::CLC, false)));
    mapcInstructions[25] = Instruct(CreateORA(ORA_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::ORA, WFR_M, false)));
    mapcInstructions[26] = Instruct(CreateINC_A(INC_Accumulator, CreateAccumulatorCycles(&CW65C816::INC_A, false)));
    mapcInstructions[27] = Instruct(CreateTCS(TCS_Implied, CreateImpliedCycles(&CW65C816::TCS, false)));
    mapcInstructions[28] = Instruct(CreateTRB(TRB_Absolute, CreateAbsoluteRMWCycles(&CW65C816::TRB, true)));
    mapcInstructions[29] = Instruct(CreateORA(ORA_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::ORA, WFR_M, false)));
    mapcInstructions[30] = Instruct(CreateASL(ASL_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::ASL, true)));
    mapcInstructions[31] = Instruct(CreateORA(ORA_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::ORA, false)));
    mapcInstructions[32] = Instruct(CreateJSR(JSR_Absolute, CreateAbsoluteJSRCycles()));
    mapcInstructions[33] = Instruct(CreateAND(AND_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::AND, false)));
    mapcInstructions[34] = Instruct(CreateJSL(JSL_AbsoluteLong, CreateAbsoluteLongJSLCycles()));
    mapcInstructions[35] = Instruct(CreateAND(AND_StackRelative, CreateStackRelativeCycles(&CW65C816::AND, false)));
    mapcInstructions[36] = Instruct(CreateBIT(BIT_DirectPage, CreateDirectCycles(&CW65C816::BIT, WFR_M, false)));
    mapcInstructions[37] = Instruct(CreateAND(AND_DirectPage, CreateDirectCycles(&CW65C816::AND, WFR_M, false)));
    mapcInstructions[38] = Instruct(CreateROL(ROL_DirectPage, CreateDirectRMWCycles(&CW65C816::ROL, true)));
    mapcInstructions[39] = Instruct(CreateAND(AND_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::AND, false)));
    mapcInstructions[40] = Instruct(CreatePLP(PLP_StackImplied, CreateStackPLPCycles(&CW65C816::PLP, false)));
    mapcInstructions[41] = Instruct(CreateAND(AND_Immediate, CreateImmediateCycles(&CW65C816::AND, WFR_M, false)));
    mapcInstructions[42] = Instruct(CreateRLA(ROL_Accumulator, CreateAccumulatorCycles(&CW65C816::ROL_A, false)));
    mapcInstructions[43] = Instruct(CreatePLD(PLD_StackImplied, CreateStackPLDCycles(&CW65C816::PLD, false)));
    mapcInstructions[44] = Instruct(CreateBIT(BIT_Absolute, CreateAbsoluteCycles(&CW65C816::BIT, WFR_M, false)));
    mapcInstructions[45] = Instruct(CreateAND(AND_Absolute, CreateAbsoluteCycles(&CW65C816::AND, WFR_M, false)));
    mapcInstructions[46] = Instruct(CreateROL(ROL_Absolute, CreateAbsoluteRMWCycles(&CW65C816::ROL, true)));
    mapcInstructions[47] = Instruct(CreateAND(AND_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::AND, false)));
    mapcInstructions[48] = Instruct(CreateBMI(BMI_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BMI, false)));
    mapcInstructions[49] = Instruct(CreateAND(AND_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::AND, false)));
    mapcInstructions[50] = Instruct(CreateAND(AND_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::AND, false)));
    mapcInstructions[51] = Instruct(CreateAND(AND_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::AND, false)));
    mapcInstructions[52] = Instruct(CreateBIT(BIT_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::BIT, WFR_M, false)));
    mapcInstructions[53] = Instruct(CreateAND(AND_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::AND, WFR_M, false)));
    mapcInstructions[54] = Instruct(CreateROL(ROL_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::ROL, true)));
    mapcInstructions[55] = Instruct(CreateAND(AND_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::AND, false)));
    mapcInstructions[56] = Instruct(CreateSEC(SEC_Implied, CreateImpliedCycles(&CW65C816::SEC, false)));
    mapcInstructions[57] = Instruct(CreateAND(AND_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::AND, WFR_M, false)));
    mapcInstructions[58] = Instruct(CreateDEC_A(DEC_Accumulator, CreateAccumulatorCycles(&CW65C816::DEC_A, false)));
    mapcInstructions[59] = Instruct(CreateTSC(TSC_Implied, CreateImpliedCycles(&CW65C816::TSC, false)));
    mapcInstructions[60] = Instruct(CreateBIT(BIT_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::BIT, WFR_M, false)));
    mapcInstructions[61] = Instruct(CreateAND(AND_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::AND, WFR_M, false)));
    mapcInstructions[62] = Instruct(CreateROL(ROL_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::ROL, true)));
    mapcInstructions[63] = Instruct(CreateAND(AND_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::AND, false)));
    mapcInstructions[64] = Instruct(CreateRTI(RTI_StackImplied, CreateStackRTICycles()));
    mapcInstructions[65] = Instruct(CreateEOR(EOR_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::EOR, false)));
    mapcInstructions[66] = Instruct(CreateWDM(WDM_Implied, CreateWDMImpliedCycles(&CW65C816::WDM, false)));
    mapcInstructions[67] = Instruct(CreateEOR(EOR_StackRelative, CreateStackRelativeCycles(&CW65C816::EOR, false)));
    mapcInstructions[68] = Instruct(CreateMVP(MVP_BlockMove, CreateBlockMoveCycles(&CW65C816::MVP, false)));
    mapcInstructions[69] = Instruct(CreateEOR(EOR_DirectPage, CreateDirectCycles(&CW65C816::EOR, WFR_M, false)));
    mapcInstructions[70] = Instruct(CreateLSR(LSR_DirectPage, CreateDirectRMWCycles(&CW65C816::LSR, true)));
    mapcInstructions[71] = Instruct(CreateEOR(EOR_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::EOR, false)));
    mapcInstructions[72] = Instruct(CreatePHA(PHA_StackImplied, CreateStackPushCycles(&CW65C816::PHA, WFR_M, true)));
    mapcInstructions[73] = Instruct(CreateEOR(EOR_Immediate, CreateImmediateCycles(&CW65C816::EOR, WFR_M, false)));
    mapcInstructions[74] = Instruct(CreateSRA(LSR_Accumulator, CreateAccumulatorCycles(&CW65C816::LSR_A, false)));
    mapcInstructions[75] = Instruct(CreatePHK(PHK_StackImplied, CreateStackPHKCycles(&CW65C816::PHK, true)));
    mapcInstructions[76] = Instruct(CreateJMP(JMP_Absolute, CreateAbsoluteJMPCycles()));
    mapcInstructions[77] = Instruct(CreateEOR(EOR_Absolute, CreateAbsoluteCycles(&CW65C816::EOR, WFR_M, false)));
    mapcInstructions[78] = Instruct(CreateLSR(LSR_Absolute, CreateAbsoluteRMWCycles(&CW65C816::LSR, true)));
    mapcInstructions[79] = Instruct(CreateEOR(EOR_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::EOR, false)));
    mapcInstructions[80] = Instruct(CreateBVC(BVC_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BVC, false)));
    mapcInstructions[81] = Instruct(CreateEOR(EOR_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::EOR, false)));
    mapcInstructions[82] = Instruct(CreateEOR(EOR_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::EOR, false)));
    mapcInstructions[83] = Instruct(CreateEOR(EOR_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::EOR, false)));
    mapcInstructions[84] = Instruct(CreateMVN(MVN_BlockMove, CreateBlockMoveCycles(&CW65C816::MVN, false)));
    mapcInstructions[85] = Instruct(CreateEOR(EOR_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::EOR, WFR_M, false)));
    mapcInstructions[86] = Instruct(CreateLSR(LSR_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::LSR, true)));
    mapcInstructions[87] = Instruct(CreateEOR(EOR_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::EOR, false)));
    mapcInstructions[88] = Instruct(CreateCLI(CLI_Implied, CreateImpliedCycles(&CW65C816::CLI, true)));
    mapcInstructions[89] = Instruct(CreateEOR(EOR_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::EOR, WFR_M, false)));
    mapcInstructions[90] = Instruct(CreatePHY(PHY_StackImplied, CreateStackPushCycles(&CW65C816::PHY, WFR_XY, true)));
    mapcInstructions[91] = Instruct(CreateTCD(TCD_Implied, CreateImpliedCycles(&CW65C816::TCD, false)));
    mapcInstructions[92] = Instruct(CreateJMP(JML_AbsoluteLong, CreateAbsoluteLongJMLCycles()));
    mapcInstructions[93] = Instruct(CreateEOR(EOR_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::EOR, WFR_M, false)));
    mapcInstructions[94] = Instruct(CreateLSR(LSR_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::LSR, true)));
    mapcInstructions[95] = Instruct(CreateEOR(EOR_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::EOR, false)));
    mapcInstructions[96] = Instruct(CreateRTS(RTS_StackImplied, CreateStackRTSCycles()));
    mapcInstructions[97] = Instruct(CreateADC(ADC_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::ADC, false)));
    mapcInstructions[98] = Instruct(CreatePER(PER_StackProgramCounterRelativeLong, CreateStackPERCycles(&CW65C816::PER, false)));
    mapcInstructions[99] = Instruct(CreateADC(ADC_StackRelative, CreateStackRelativeCycles(&CW65C816::ADC, false)));
    mapcInstructions[100] = Instruct(CreateSTZ(STZ_DirectPage, CreateDirectWriteCycles(&CW65C816::STZ, WFR_M, true)));
    mapcInstructions[101] = Instruct(CreateADC(ADC_DirectPage, CreateDirectCycles(&CW65C816::ADC, WFR_M, false)));
    mapcInstructions[102] = Instruct(CreateROR(ROR_DirectPage, CreateDirectRMWCycles(&CW65C816::ROR, true)));
    mapcInstructions[103] = Instruct(CreateADC(ADC_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::ADC, false)));
    mapcInstructions[104] = Instruct(CreatePLA(PLA_Stack, CreateStackPullCycles(&CW65C816::PLA, WFR_M, false)));
    mapcInstructions[105] = Instruct(CreateADC(ADC_Immediate, CreateImmediateCycles(&CW65C816::ADC, WFR_M, false)));
    mapcInstructions[106] = Instruct(CreateRRA(ROR_Accumulator, CreateAccumulatorCycles(&CW65C816::ROR_A, false)));
    mapcInstructions[107] = Instruct(CreateRTL(RTL_StackImplied, CreateStackRTLCycles()));
    mapcInstructions[108] = Instruct(CreateJMP(JMP_AbsoluteIndirect, CreateAbsoluteIndirectJMPCycles()));
    mapcInstructions[109] = Instruct(CreateADC(ADC_Absolute, CreateAbsoluteCycles(&CW65C816::ADC, WFR_M, false)));
    mapcInstructions[110] = Instruct(CreateROR(ROR_Absolute, CreateAbsoluteRMWCycles(&CW65C816::ROR, true)));
    mapcInstructions[111] = Instruct(CreateADC(ADC_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::ADC, false)));
    mapcInstructions[112] = Instruct(CreateBVS(BVS_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BVS, false)));
    mapcInstructions[113] = Instruct(CreateADC(ADC_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::ADC, false)));
    mapcInstructions[114] = Instruct(CreateADC(ADC_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::ADC, false)));
    mapcInstructions[115] = Instruct(CreateADC(ADC_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::ADC, false)));
    mapcInstructions[116] = Instruct(CreateSTZ(STZ_DirectPageIndexedWithX, CreateDirectIndexedWithXWriteCycles(&CW65C816::STZ, WFR_M, true)));
    mapcInstructions[117] = Instruct(CreateADC(ADC_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::ADC, WFR_M, false)));
    mapcInstructions[118] = Instruct(CreateROR(ROR_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::ROR, true)));
    mapcInstructions[119] = Instruct(CreateADC(ADC_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::ADC, false)));
    mapcInstructions[120] = Instruct(CreateSEI(SEI_Implied, CreateImpliedCycles(&CW65C816::SEI, true)));
    mapcInstructions[121] = Instruct(CreateADC(ADC_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::ADC, WFR_M, false)));
    mapcInstructions[122] = Instruct(CreatePLY(PLY_StackImplied, CreateStackPullCycles(&CW65C816::PLY, WFR_XY, false)));
    mapcInstructions[123] = Instruct(CreateTDC(TDC_Implied, CreateImpliedCycles(&CW65C816::TDC, false)));
    mapcInstructions[124] = Instruct(CreateJMP(JMP_AbsoluteIndexedIndirectWithX, CreateAbsoluteIndexedIndirectWithXJMPCycles()));
    mapcInstructions[125] = Instruct(CreateADC(ADC_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::ADC, WFR_M, false)));
    mapcInstructions[126] = Instruct(CreateROR(ROR_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::ROR, true)));
    mapcInstructions[127] = Instruct(CreateADC(ADC_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::ADC, false)));
    mapcInstructions[128] = Instruct(CreateBRA(BRA_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BRA, false)));
    mapcInstructions[129] = Instruct(CreateSTA(STA_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXWriteCycles(&CW65C816::STA, true)));
    mapcInstructions[130] = Instruct(CreateBRL(BRL_ProgramCounterRelativeLong, CreateRelativeLongCycles(&CW65C816::BRA, false)));
    mapcInstructions[131] = Instruct(CreateSTA(STA_StackRelative, CreateStackRelativeWriteCycles(&CW65C816::STA, true)));
    mapcInstructions[132] = Instruct(CreateSTY(STY_DirectPage, CreateDirectWriteCycles(&CW65C816::STY, WFR_XY, true)));
    mapcInstructions[133] = Instruct(CreateSTA(STA_DirectPage, CreateDirectWriteCycles(&CW65C816::STA, WFR_M, true)));
    mapcInstructions[134] = Instruct(CreateSTX(STX_DirectPage, CreateDirectWriteCycles(&CW65C816::STX, WFR_XY, true)));
    mapcInstructions[135] = Instruct(CreateSTA(STA_DirectPageIndirectLong, CreateDirectIndirectLongWriteCycles(&CW65C816::STA, true)));
    mapcInstructions[136] = Instruct(CreateDEY(DEY_Implied, CreateImpliedCycles(&CW65C816::DEY, false)));
    mapcInstructions[137] = Instruct(CreateBIT(BIT_Immediate, CreateImmediateCycles(&CW65C816::BIT_A, WFR_M, false)));
    mapcInstructions[138] = Instruct(CreateTXA(TXA_Implied, CreateImpliedCycles(&CW65C816::TXA, false)));
    mapcInstructions[139] = Instruct(CreatePHB(PHB_StackImplied, CreateStackPHBCycles(&CW65C816::PHB, true)));
    mapcInstructions[140] = Instruct(CreateSTY(STY_Absolute, CreateAbsoluteWriteCycles(&CW65C816::STY, WFR_XY, true)));
    mapcInstructions[141] = Instruct(CreateSTA(STA_Absolute, CreateAbsoluteWriteCycles(&CW65C816::STA, WFR_M, true)));
    mapcInstructions[142] = Instruct(CreateSTX(STX_Absolute, CreateAbsoluteWriteCycles(&CW65C816::STX, WFR_XY, true)));
    mapcInstructions[143] = Instruct(CreateSTA(STA_AbsoluteLong, CreateAbsoluteLongWriteCycles(&CW65C816::STA, true)));
    mapcInstructions[144] = Instruct(CreateBCC(BCC_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BCC, false)));
    mapcInstructions[145] = Instruct(CreateSTA(STA_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYWriteCycles(&CW65C816::STA, true)));
    mapcInstructions[146] = Instruct(CreateSTA(STA_DirectPageIndirect, CreateDirectIndirectWriteCycles(&CW65C816::STA, true)));
    mapcInstructions[147] = Instruct(CreateSTA(STA_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYWriteCycles(&CW65C816::STA, true)));
    mapcInstructions[148] = Instruct(CreateSTY(STY_DirectPageIndexedWithX, CreateDirectIndexedWithXWriteCycles(&CW65C816::STY, WFR_XY, true)));
    mapcInstructions[149] = Instruct(CreateSTA(STA_DirectPageIndexedWithX, CreateDirectIndexedWithXWriteCycles(&CW65C816::STA, WFR_M, true)));
    mapcInstructions[150] = Instruct(CreateSTX(STX_DirectPageIndexedWithY, CreateDirectIndexedWithYWriteCycles(&CW65C816::STX, WFR_XY, true)));
    mapcInstructions[151] = Instruct(CreateSTA(STA_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYWriteCycles(&CW65C816::STA, true)));
    mapcInstructions[152] = Instruct(CreateTYA(TYA_Implied, CreateImpliedCycles(&CW65C816::TYA, false)));
    mapcInstructions[153] = Instruct(CreateSTA(STA_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYWriteCycles(&CW65C816::STA, true)));
    mapcInstructions[154] = Instruct(CreateTXS(TXS_Implied, CreateImpliedCycles(&CW65C816::TXS, false)));
    mapcInstructions[155] = Instruct(CreateTXY(TXY_Implied, CreateImpliedCycles(&CW65C816::TXY, false)));
    mapcInstructions[156] = Instruct(CreateSTZ(STZ_Absolute, CreateAbsoluteWriteCycles(&CW65C816::STZ, WFR_M, true)));
    mapcInstructions[157] = Instruct(CreateSTA(STA_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXWriteCycles(&CW65C816::STA, true)));
    mapcInstructions[158] = Instruct(CreateSTZ(STZ_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXWriteCycles(&CW65C816::STZ, true)));
    mapcInstructions[159] = Instruct(CreateSTA(STA_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXWriteCycles(&CW65C816::STA, true)));
    mapcInstructions[160] = Instruct(CreateLDY(LDY_Immediate, CreateImmediateCycles(&CW65C816::LDY, WFR_XY, false)));
    mapcInstructions[161] = Instruct(CreateLDA(LDA_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::LDA, false)));
    mapcInstructions[162] = Instruct(CreateLDX(LDX_Immediate, CreateImmediateCycles(&CW65C816::LDX, WFR_XY, false)));
    mapcInstructions[163] = Instruct(CreateLDA(LDA_StackRelative, CreateStackRelativeCycles(&CW65C816::LDA, false)));
    mapcInstructions[164] = Instruct(CreateLDY(LDY_DirectPage, CreateDirectCycles(&CW65C816::LDY, WFR_XY, false)));
    mapcInstructions[165] = Instruct(CreateLDA(LDA_DirectPage, CreateDirectCycles(&CW65C816::LDA, WFR_M, false)));
    mapcInstructions[166] = Instruct(CreateLDX(LDX_DirectPage, CreateDirectCycles(&CW65C816::LDX, WFR_XY, false)));
    mapcInstructions[167] = Instruct(CreateLDA(LDA_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::LDA, false)));
    mapcInstructions[168] = Instruct(CreateTAY(TAY_Implied, CreateImpliedCycles(&CW65C816::TAY, false)));
    mapcInstructions[169] = Instruct(CreateLDA(LDA_Immediate, CreateImmediateCycles(&CW65C816::LDA, WFR_M, false)));
    mapcInstructions[170] = Instruct(CreateTAX(TAX_Implied, CreateImpliedCycles(&CW65C816::TAX, false)));
    mapcInstructions[171] = Instruct(CreatePLB(PLB_StackImplied, CreateStackPLBCycles(&CW65C816::PLB, false)));
    mapcInstructions[172] = Instruct(CreateLDY(LDY_Absolute, CreateAbsoluteCycles(&CW65C816::LDY, WFR_XY, false)));
    mapcInstructions[173] = Instruct(CreateLDA(LDA_Absolute, CreateAbsoluteCycles(&CW65C816::LDA, WFR_M, false)));
    mapcInstructions[174] = Instruct(CreateLDX(LDX_Absolute, CreateAbsoluteCycles(&CW65C816::LDX, WFR_XY, false)));
    mapcInstructions[175] = Instruct(CreateLDA(LDA_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::LDA, false)));
    mapcInstructions[176] = Instruct(CreateBCS(BCS_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BCS, false)));
    mapcInstructions[177] = Instruct(CreateLDA(LDA_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::LDA, false)));
    mapcInstructions[178] = Instruct(CreateLDA(LDA_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::LDA, false)));
    mapcInstructions[179] = Instruct(CreateLDA(LDA_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::LDA, false)));
    mapcInstructions[180] = Instruct(CreateLDY(LDY_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::LDY, WFR_XY, false)));
    mapcInstructions[181] = Instruct(CreateLDA(LDA_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::LDA, WFR_M, false)));
    mapcInstructions[182] = Instruct(CreateLDX(LDX_DirectPageIndexedWithY, CreateDirectIndexedWithYCycles(&CW65C816::LDX, WFR_XY, false)));
    mapcInstructions[183] = Instruct(CreateLDA(LDA_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::LDA, false)));
    mapcInstructions[184] = Instruct(CreateCLV(CLV_Implied, CreateImpliedCycles(&CW65C816::CLV, false)));
    mapcInstructions[185] = Instruct(CreateLDA(LDA_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::LDA, WFR_M, false)));
    mapcInstructions[186] = Instruct(CreateTSX(TSX_Implied, CreateImpliedCycles(&CW65C816::TSX, false)));
    mapcInstructions[187] = Instruct(CreateTYX(TYX_Implied, CreateImpliedCycles(&CW65C816::TYX, false)));
    mapcInstructions[188] = Instruct(CreateLDY(LDY_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::LDY, WFR_XY, false)));
    mapcInstructions[189] = Instruct(CreateLDA(LDA_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::LDA, WFR_M, false)));
    mapcInstructions[190] = Instruct(CreateLDX(LDX_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::LDX, WFR_XY, false)));
    mapcInstructions[191] = Instruct(CreateLDA(LDA_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::LDA, false)));
    mapcInstructions[192] = Instruct(CreateCPY(CPY_Immediate, CreateImmediateCycles(&CW65C816::CPY, WFR_XY, false)));
    mapcInstructions[193] = Instruct(CreateCMP(CMP_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::CMP, false)));
    mapcInstructions[194] = Instruct(CreateREP(REP_Immediate, CreateImmediateREPSEPCycles(&CW65C816::REP, false)));
    mapcInstructions[195] = Instruct(CreateCMP(CMP_StackRelative, CreateStackRelativeCycles(&CW65C816::CMP, false)));
    mapcInstructions[196] = Instruct(CreateCPY(CPY_DirectPage, CreateDirectCycles(&CW65C816::CPY, WFR_XY, false)));
    mapcInstructions[197] = Instruct(CreateCMP(CMP_DirectPage, CreateDirectCycles(&CW65C816::CMP, WFR_M, false)));
    mapcInstructions[198] = Instruct(CreateDEC(DEC_DirectPage, CreateDirectRMWCycles(&CW65C816::DEC, true)));
    mapcInstructions[199] = Instruct(CreateCMP(CMP_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::CMP, false)));
    mapcInstructions[200] = Instruct(CreateINY(INY_Implied, CreateImpliedCycles(&CW65C816::INY, false)));
    mapcInstructions[201] = Instruct(CreateCMP(CMP_Immediate, CreateImmediateCycles(&CW65C816::CMP, WFR_M, false)));
    mapcInstructions[202] = Instruct(CreateDEX(DEX_Implied, CreateImpliedCycles(&CW65C816::DEX, false)));
    mapcInstructions[203] = Instruct(CreateWAI(WAI_Implied, CreateWaitForInterruptCycles()));
    mapcInstructions[204] = Instruct(CreateCPY(CPY_Absolute, CreateAbsoluteCycles(&CW65C816::CPY, WFR_XY, false)));
    mapcInstructions[205] = Instruct(CreateCMP(CMP_Absolute, CreateAbsoluteCycles(&CW65C816::CMP, WFR_M, false)));
    mapcInstructions[206] = Instruct(CreateDEC(DEC_Absolute, CreateAbsoluteRMWCycles(&CW65C816::DEC, true)));
    mapcInstructions[207] = Instruct(CreateCMP(CMP_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::CMP, false)));
    mapcInstructions[208] = Instruct(CreateBNE(BNE_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BNE, false)));
    mapcInstructions[209] = Instruct(CreateCMP(CMP_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::CMP, false)));
    mapcInstructions[210] = Instruct(CreateCMP(CMP_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::CMP, false)));
    mapcInstructions[211] = Instruct(CreateCMP(CMP_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::CMP, false)));
    mapcInstructions[212] = Instruct(CreatePEI(PEI_StackDirectPageIndirect, CreateStackPEICycles()));
    mapcInstructions[213] = Instruct(CreateCMP(CMP_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::CMP, WFR_M, false)));
    mapcInstructions[214] = Instruct(CreateDEC(DEC_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::DEC, true)));
    mapcInstructions[215] = Instruct(CreateCMP(CMP_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::CMP, false)));
    mapcInstructions[216] = Instruct(CreateCLD(CLD_Implied, CreateImpliedCycles(&CW65C816::CLD, false)));
    mapcInstructions[217] = Instruct(CreateCMP(CMP_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::CMP, WFR_M, false)));
    mapcInstructions[218] = Instruct(CreatePHX(PHX_StackImplied, CreateStackPushCycles(&CW65C816::PHX, WFR_XY, true)));
    mapcInstructions[219] = Instruct(CreateSTP(STP_Implied, CreateStopTheClockCycles(&CW65C816::STP, false)));
    mapcInstructions[220] = Instruct(CreateJMP(JML_AbsoluteIndirectLong, CreateAbsoluteIndirectJMLCycles()));
    mapcInstructions[221] = Instruct(CreateCMP(CMP_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::CMP, WFR_M, false)));
    mapcInstructions[222] = Instruct(CreateDEC(DEC_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::DEC, true)));
    mapcInstructions[223] = Instruct(CreateCMP(CMP_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::CMP, false)));
    mapcInstructions[224] = Instruct(CreateCPX(CPX_Immediate, CreateImmediateCycles(&CW65C816::CPX, WFR_XY, false)));
    mapcInstructions[225] = Instruct(CreateSBC(SBC_DirectPageIndexedIndirectWithX, CreateDirectIndexedIndirectWithXCycles(&CW65C816::SBC, false)));
    mapcInstructions[226] = Instruct(CreateSEP(SEP_Immediate, CreateImmediateREPSEPCycles(&CW65C816::SEP, false)));
    mapcInstructions[227] = Instruct(CreateSBC(SBC_StackRelative, CreateStackRelativeCycles(&CW65C816::SBC, false)));
    mapcInstructions[228] = Instruct(CreateCPX(CPX_DirectPage, CreateDirectCycles(&CW65C816::CPX, WFR_XY, false)));
    mapcInstructions[229] = Instruct(CreateSBC(SBC_DirectPage, CreateDirectCycles(&CW65C816::SBC, WFR_M, false)));
    mapcInstructions[230] = Instruct(CreateINC(INC_DirectPage, CreateDirectRMWCycles(&CW65C816::INC, true)));
    mapcInstructions[231] = Instruct(CreateSBC(SBC_DirectPageIndirectLong, CreateDirectIndirectLongCycles(&CW65C816::SBC, false)));
    mapcInstructions[232] = Instruct(CreateINX(INX_Implied, CreateImpliedCycles(&CW65C816::INX, false)));
    mapcInstructions[233] = Instruct(CreateSBC(SBC_Immediate, CreateImmediateCycles(&CW65C816::SBC, WFR_M, false)));
    mapcInstructions[234] = Instruct(CreateNOP(NOP_Implied, CreateImpliedCycles(&CW65C816::NOP, false)));
    mapcInstructions[235] = Instruct(CreateXBA(XBA_Implied, CreateImpliedXBACycles(&CW65C816::XBA, false)));
    mapcInstructions[236] = Instruct(CreateCPX(CPX_Absolute, CreateAbsoluteCycles(&CW65C816::CPX, WFR_XY, false)));
    mapcInstructions[237] = Instruct(CreateSBC(SBC_Absolute, CreateAbsoluteCycles(&CW65C816::SBC, WFR_M, false)));
    mapcInstructions[238] = Instruct(CreateINC(INC_Absolute, CreateAbsoluteRMWCycles(&CW65C816::INC, true)));
    mapcInstructions[239] = Instruct(CreateSBC(SBC_AbsoluteLong, CreateAbsoluteLongCycles(&CW65C816::SBC, false)));
    mapcInstructions[240] = Instruct(CreateBEQ(BEQ_ProgramCounterRelative, CreateRelativeShortCycles(&CW65C816::BEQ, false)));
    mapcInstructions[241] = Instruct(CreateSBC(SBC_DirectPageIndirectIndexedWithY, CreateDirectIndirectIndexedWithYCycles(&CW65C816::SBC, false)));
    mapcInstructions[242] = Instruct(CreateSBC(SBC_DirectPageIndirect, CreateDirectIndirectCycles(&CW65C816::SBC, false)));
    mapcInstructions[243] = Instruct(CreateSBC(SBC_StackRelativeIndirectIndexedWithY, CreateStackRelativeIndirectIndexedWithYCycles(&CW65C816::SBC, false)));
    mapcInstructions[244] = Instruct(CreatePEA(PEA_StackImmediate, CreateStackPEACycles()));
    mapcInstructions[245] = Instruct(CreateSBC(SBC_DirectPageIndexedWithX, CreateDirectIndexedWithXCycles(&CW65C816::SBC, WFR_M, false)));
    mapcInstructions[246] = Instruct(CreateINC(INC_DirectPageIndexedWithX, CreateDirectIndexedWithXRMWCycles(&CW65C816::INC, true)));
    mapcInstructions[247] = Instruct(CreateSBC(SBC_DirectPageIndirectLongIndexedWithY, CreateDirectIndirectLongIndexedWithYCycles(&CW65C816::SBC, false)));
    mapcInstructions[248] = Instruct(CreateSED(SED_Implied, CreateImpliedCycles(&CW65C816::SED, false)));
    mapcInstructions[249] = Instruct(CreateSBC(SBC_AbsoluteIndexedWithY, CreateAbsoluteIndexedWithYCycles(&CW65C816::SBC, WFR_M, false)));
    mapcInstructions[250] = Instruct(CreatePLX(PLX_StackImplied, CreateStackPullCycles(&CW65C816::PLX, WFR_XY, false)));
    mapcInstructions[251] = Instruct(CreateXCE(XCE_Implied, CreateImpliedCycles(&CW65C816::XCE, false)));
    mapcInstructions[252] = Instruct(CreateJSR(JSR_AbsoluteIndexedIndirectWithX, CreateAbsoluteIndexedIndirectWithXJSRCycles()));
    mapcInstructions[253] = Instruct(CreateSBC(SBC_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXCycles(&CW65C816::SBC, WFR_M, false)));
    mapcInstructions[254] = Instruct(CreateINC(INC_AbsoluteIndexedWithX, CreateAbsoluteIndexedWithXRMWCycles(&CW65C816::INC, true)));
    mapcInstructions[255] = Instruct(CreateSBC(SBC_AbsoluteLongIndexedWithX, CreateAbsoluteLongIndexedWithXCycles(&CW65C816::SBC, false)));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::Instruct(CInstruction* pcInstruction)
{
    muiInstructions++;
    return pcInstruction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstructionFactory::ValidateOpcodes(void)
{
    uint16          i;
    CInstruction*   pcInstruction;

    for (i = 0; i <= 255; i++)
    {
        pcInstruction = mapcInstructions[i];
        if (pcInstruction->GetCode() != i)
        {

            gcLogger.Error2(__METHOD__, " Opcode [", pcInstruction->GetName(), "] has uiCode [", ShortToString(pcInstruction->GetCode()), "] but is at index [", ShortToString(i), "].", NULL);
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
        "No Operation for two cycles");
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
        "Decrement Index X by One");
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
        "Increment memory; result in memory and update NZ");
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
        "Subtract memory and carry from A; result in A and update NZC");
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
        "Decrement memory; result in memory and update NZ");
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
        "Rotate accumulator right one bit; update NZC");
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
        "Rotate memory right one bit; update NZC");
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
        "Add memory and carry to A; result in A and update NZC");
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
        "Shift accumulator right one bit; update NZC");
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
        "LSR", "Shift memory right one bit; update NZC");
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
        "Decrement accumulator; update NZ");
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
        "Rotate Accumulator One Bit Left");
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
        "Rotate Memory One Bit Left");
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
        "Jump long to new location save return address on Stack");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateAND(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "AND",
        "Bitwise AND memory with A; result in A and update NZ");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateJSR(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "JSR",
        "Jump to new location save return address on Stack");
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
        "Increment accumulator; update NZ");
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
        "Shift accumulator left one bit; update NZC");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateASL(uint16 uiCode, CInstructionCycles* directRMWCycles)
{
    INSTRUCTION(uiCode, directRMWCycles, "ASL", "Shift memory left 1 bit; result in memory and update NZC");
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
        "Force co-processor software interrupt");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateORA(uint16 uiCode, CInstructionCycles* directIndexedIndirectWithXCycles)
{
    INSTRUCTION(uiCode, directIndexedIndirectWithXCycles,
        "ORA", "'OR' memory with A; result in A and update NZ");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateBRK(uint16 uiCode, CInstructionCycles* pcInstructionCycles)
{
    INSTRUCTION(uiCode, pcInstructionCycles, "BRK",
        "Force software break interrupt");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateReset(void)
{
    CResetVector* pcVector;

    pcVector = NewMalloc<CResetVector>();
    pcVector->Init();

    mpcReset = NewMalloc<CInstruction>();
    mpcReset->Init(muiInstructions, CreateStackResetCycles(pcVector, &CW65C816::RES, false), "RES",
        "Reset the CPU");

    mapcInstructions[muiInstructions] = mpcReset;
    return mpcReset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateIRQ(void)
{
    CIRQVector* pcVector;

    pcVector = NewMalloc<CIRQVector>();
    pcVector->Init();

    mpcIRQ = NewMalloc<CInstruction>();
    mpcIRQ->Init(muiInstructions, CreateStackHardwareInterruptCycles(pcVector, &CW65C816::IRQ, false), "IRQ",
        "Interrupt request");

    mapcInstructions[muiInstructions] = mpcIRQ;
    return mpcIRQ;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateNMI(void)
{
    CNMIVector* pcVector;

    pcVector = NewMalloc<CNMIVector>();
    pcVector->Init();

    mpcNMI = NewMalloc<CInstruction>();
    mpcNMI->Init(muiInstructions, CreateStackHardwareInterruptCycles(pcVector, &CW65C816::NMI, false), "NMI",
        "Non-maskable interrupt");

    mapcInstructions[muiInstructions] = mpcNMI;
    return mpcNMI;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateAbort(void)
{
    CNMIVector* pcVector;

    pcVector = NewMalloc<CNMIVector>();
    pcVector->Init();

    mpcAbort = NewMalloc<CInstruction>();
    mpcAbort->Init(muiInstructions, CreateStackAbortInterruptCycles(pcVector, &CW65C816::ABORT, false), "ABT",
        "Stop the current instruction and return processor status to what it was prior to the current instruction");

    mapcInstructions[muiInstructions] = mpcAbort;
    return mpcAbort;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInstruction* CInstructionFactory::CreateFetchNext(void)
{
    mpcFetchNext = NewMalloc<CInstruction>();
    mpcFetchNext->Init(muiInstructions, CreateFetchOpcodeCycles(), "OPC",
        "Fetch Opcode from address in program counter");

    mapcInstructions[muiInstructions] = mpcFetchNext;
    return mpcFetchNext;
}

