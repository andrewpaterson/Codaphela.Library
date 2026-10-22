#include "W65C816InstructionCycleFactory.h"


//The W65C816S instruction functions are members of CW65C816, the operations call CW65CCore.
CW65ExecuteIf16Bit* E16Bit(CW65C816Func fConsumer, EWidthFromRegister eWidth, bool bInitialSide)
{
	return E16Bit(static_cast<CW65CFunc>(fConsumer), eWidth, bInitialSide);
}


CW65ExecuteIf8Bit* E8Bit(CW65C816Func fConsumer, EWidthFromRegister eWidth, bool bInitialSide)
{
	return E8Bit(static_cast<CW65CFunc>(fConsumer), eWidth, bInitialSide);
}


CW65Execute* E(CW65C816Func fConsumer, bool bInitialSide)
{
	return E(static_cast<CW65CFunc>(fConsumer), bInitialSide);
}


CW65InstructionCycles* CreateAbsoluteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//1a
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(DBR(), AA(), NULL),						Operation(Read_DataLow(), E8Bit(fOperation, eWidth, bInitialSide), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), o(1), NULL), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, eWidth, bInitialSide), DONE16Bit(eWidth), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//1a
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(DBR(), AA(), NULL),						Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), o(1), NULL), NULL),	Operation(Write_DataHigh(), DONE16Bit(eWidth), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteJMPCycles(void)
{
	//1b
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCH(), PC_inc(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteJSRCycles(void)
{
	//1c
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCH(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_PCH(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_PCL(), SP_dec(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteRMWCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//1d
	return InstructionCycles(AM_Absolute,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(DBR(), AA(), NULL),						Operation(Read_DataLow(RMW), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), o(1), NULL), NULL),	Operation(Read_DataHigh(RMW), NoteOne(WFR_M), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), oM(1), NULL), NULL),	Operation(IO(RMW), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), o(1), NULL), NULL),	Operation(NoteThree(), Write_DataHigh(RMW), NoteOne(WFR_M), NULL)),
		BusCycle(	Address(DBR(), AA(), NULL),						Operation(NoteThree(), Write_DataLow(RMW), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteIndexedIndirectWithXJMPCycles(void)
{
	//2a
	return InstructionCycles(AM_AbsoluteIndexedIndirectWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), AA(), X(), NULL),		Operation(Read_NewPCL(), NULL)),
		BusCycle(	Address(PBR(), AA(), X(), o(1), NULL),	Operation(Read_NewPCH(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteIndexedIndirectWithXJSRCycles(void)
{
	//2b
	return InstructionCycles(AM_AbsoluteIndexedIndirectWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(S16(), NULL),					Operation(Write_PCH(), SP_dec(), NULL)),
		BusCycle(	Address(S16(), NULL),					Operation(Write_PCL(), SP_dec(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_AAH(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(IO(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), AA(), X(), NULL),		Operation(Read_NewPCL(), NULL)),
		BusCycle(	Address(PBR(), AA(), X(), o(1), NULL),	Operation(Read_NewPCH(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteIndirectJMLCycles(void)
{
	//3a
	return InstructionCycles(AM_AbsoluteIndirectLong,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(AA(), NULL),			Operation(Read_NewPCL(), NULL)),
		BusCycle(	Address(AA(), o(1), NULL),		Operation(Read_NewPCH(), NULL)),
		BusCycle(	Address(AA(), o(2), NULL),		Operation(Read_NewPBR(), PC_e(Address(New_PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteIndirectJMPCycles(void)
{
	//3b
	return InstructionCycles(AM_AbsoluteIndirect,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(AA(), NULL),			Operation(Read_NewPCL(), NULL)),
		BusCycle(	Address(AA(), o(1), NULL),		Operation(Read_NewPCH(), PC_e(Address(PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteLongWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//4a
	return InstructionCycles(AM_AbsoluteLong,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_AAB(), PC_inc(), NULL)),
		BusCycle(	Address(AAB(), AA(), NULL),						Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(AAB(), Span(AA(), o(1), NULL), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteLongCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//4a
	return InstructionCycles(AM_AbsoluteLong,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_AAB(), PC_inc(), NULL)),
		BusCycle(	Address(AAB(), AA(), NULL),						Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(AAB(), Span(AA(), o(1), NULL), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteLongJMLCycles(void)
{
	//4b
	return InstructionCycles(AM_AbsoluteLong,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCH(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPBR(), PC_inc(), PC_e(Address(New_PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteLongJSLCycles(void)
{
	//4c
	return InstructionCycles(AM_AbsoluteLong,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPCH(), PC_inc(), NULL)),
		BusCycle(	Address(S16(), NULL),			Operation(Write_PBR(), SP_dec(), NULL)),
		BusCycle(	Address(S16(), o(1), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_NewPBR(), NULL)),
		BusCycle(	Address(S16(), NULL),			Operation(Write_PCH(), SP_dec(), NULL)),
		BusCycle(	Address(S16(), NULL),			Operation(Write_PCL(), SP_dec(), PC_e(Address(New_PBR(), New_PC(), NULL)), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteLongIndexedWithXCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//5
	return InstructionCycles(AM_AbsoluteLongIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_AAB(), PC_inc(), NULL)),
		BusCycle(	Address(AAB(), Span(AA(), X(), NULL), NULL),		Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(AAB(), Span(AA(), X(), o(1), NULL), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteLongIndexedWithXWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//5
	return InstructionCycles(AM_AbsoluteLongIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_AAB(), PC_inc(), NULL)),
		BusCycle(	Address(AAB(), Span(AA(), X(), NULL), NULL),		Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(AAB(), Span(AA(), X(), o(1), NULL), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteIndexedWithXWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//6a
	return InstructionCycles(AM_AbsoluteIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(DBR(), AAH(), AAL_XL(), NULL),				Operation(IO(), NoteFourX(false), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), X(), NULL), NULL),		Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), X(), o(1), NULL), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteIndexedWithXCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//6a
	return InstructionCycles(AM_AbsoluteIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(DBR(), AAH(), AAL_XL(), NULL),				Operation(IO(), NoteFourX(true), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), X(), NULL), NULL),		Operation(Read_DataLow(), E8Bit(fOperation, eWidth, bInitialSide), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), X(), o(1), NULL), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, eWidth, bInitialSide), DONE16Bit(eWidth), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteIndexedWithXRMWCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//6b
	return InstructionCycles(AM_AbsoluteIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(DBR(), AAH(), AAL_XL(), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), X(), NULL), NULL),		Operation(Read_DataLow(RMW), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), X(), o(1), NULL), NULL),	Operation(Read_DataHigh(RMW), NoteOne(WFR_M), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), X(), oM(1), NULL), NULL),	Operation(IO(RMW), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), X(), o(1), NULL), NULL),	Operation(NoteThree(), Write_DataHigh(RMW), NoteOne(WFR_M), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), X(), NULL), NULL),		Operation(NoteThree(), Write_DataLow(RMW), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteIndexedWithYWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//7
	return InstructionCycles(AM_AbsoluteIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(DBR(), AAH(), AAL_YL(), NULL),				Operation(IO(), NoteFourY(false), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), Y(), NULL), NULL),		Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), Y(), o(1), NULL), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAbsoluteIndexedWithYCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//7
	return InstructionCycles(AM_AbsoluteIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(DBR(), AAH(), AAL_YL(), NULL),				Operation(IO(), NoteFourY(true), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), Y(), NULL), NULL),		Operation(Read_DataLow(), E8Bit(fOperation, eWidth, bInitialSide), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), Y(), o(1), NULL), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, eWidth, bInitialSide), DONE16Bit(eWidth), NULL)),
		NULL);
}


CW65InstructionCycles* CreateAccumulatorCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//8
	return InstructionCycles(AM_Accumulator,
		BusCycle(	Address(PBR(), PC(), NULL),	Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),	Operation(IO(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateBlockMoveCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//9a & 9b
	return InstructionCycles(AM_BlockMove,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_DBR(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAB(), PC_inc(), NULL)),
		BusCycle(	Address(AAB(), X(), NULL),		Operation(Read_DataLow(), NULL)),
		BusCycle(	Address(DBR(), Y(), NULL),		Operation(Write_DataLow(), NULL)),
		BusCycle(	Address(DBR(), Y(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(DBR(), Y(), NULL),		Operation(IO(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//10a
	return InstructionCycles(AM_Direct,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(),  D0(), NULL),			Operation(Read_DataLow(), E8Bit(fOperation, eWidth, bInitialSide), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DP(),  D0(), o(1), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, eWidth, bInitialSide), DONE16Bit(eWidth), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//10a
	return InstructionCycles(AM_Direct,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(),  D0(), NULL),		Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DP(),  D0(), o(1), NULL),	Operation(Write_DataHigh(), DONE16Bit(eWidth), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectRMWCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//10b
	return InstructionCycles(AM_Direct,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(),  D0(), NULL),			Operation(Read_DataLow(RMW), NULL)),
		BusCycle(	Address(DP(),  D0(), o(1), NULL),	Operation(Read_DataHigh(RMW), NoteOne(WFR_M), NULL)),
		BusCycle(	Address(DP(),  D0(), oM(1), NULL),	Operation(IO(RMW), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(DP(),  D0(), o(1), NULL),	Operation(NoteThree(), Write_DataHigh(RMW), NoteOne(WFR_M), NULL)),
		BusCycle(	Address(DP(),  D0(), NULL),			Operation(NoteThree(), Write_DataLow(RMW), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectIndexedIndirectWithXCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//11
	return InstructionCycles(AM_DirectIndexedIndirectWithX,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),				Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(DP(),  D0(), X(), NULL),				Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(),  D0(), X(), o(1), NULL),			Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DBR(), AA(), NULL),						Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), o(1), NULL), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectIndexedIndirectWithXWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//11
	return InstructionCycles(AM_DirectIndexedIndirectWithX,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),				Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),				Operation(IO(), NULL)),
		BusCycle(	Address(DP(),  D0(), X(), NULL),					Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(),  D0(), X(), o(1), NULL),			Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DBR(), AA(), NULL),						Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), o(1), NULL), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectIndirectCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//12
	return InstructionCycles(AM_DirectIndirect,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),				Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(),  D0(), NULL),						Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(),  D0(), o(1), NULL),				Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DBR(), AA(), NULL),						Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), o(1), NULL), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectIndirectWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//12
	return InstructionCycles(AM_DirectIndirect,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),				Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(),  D0(), NULL),						Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(),  D0(), o(1), NULL),				Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DBR(), AA(), NULL),						Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), o(1), NULL), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectIndirectIndexedWithYCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//13
	return InstructionCycles(AM_DirectIndirectIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),					Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(),  D0(), NULL),							Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(),  D0(), o(1), NULL),					Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DBR(), AAH(), AAL_YL(), NULL),				Operation(IO(), NoteFourY(true), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), Y(), NULL), NULL),		Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), Y(), o(1), NULL),	NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectIndirectIndexedWithYWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//13
	return InstructionCycles(AM_DirectIndirectIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),					Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DP(),  D0(), NULL),							Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DP(),  D0(), o(1), NULL),					Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DBR(), AAH(), AAL_YL(), NULL),				Operation(IO(), NoteFourY(false), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), Y(), NULL), NULL),		Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), Y(), o(1), NULL), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectIndirectLongIndexedWithYCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//14
	return InstructionCycles(AM_DirectIndirectLongIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),					Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DPL(), D0(), NULL),							Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DPL(), D0(), o(1), NULL),					Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DPL(), D0(), o(2), NULL),					Operation(Read_AAB(), NULL)),
		BusCycle(	Address(AAB(), Span(AA(), Y(), NULL), NULL),		Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(AAB(), Span(AA(), Y(), o(1), NULL), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectIndirectLongIndexedWithYWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//14
	return InstructionCycles(AM_DirectIndirectLongIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),					Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DPL(), D0(), NULL),							Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DPL(), D0(), o(1), NULL),					Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DPL(), D0(), o(2), NULL),					Operation(Read_AAB(), NULL)),
		BusCycle(	Address(AAB(), Span(AA(), Y(), NULL), NULL),		Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(AAB(), Span(AA(), Y(), o(1), NULL), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectIndirectLongCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//15
	return InstructionCycles(AM_DirectIndirectLong,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),				Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DPL(), D0(), NULL),						Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DPL(), D0(), o(1), NULL),				Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DPL(), D0(), o(2), NULL),				Operation(Read_AAB(), NULL)),
		BusCycle(	Address(AAB(), AA(), NULL),						Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(AAB(), Span(AA(), o(1), NULL), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectIndirectLongWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//15
	return InstructionCycles(AM_DirectIndirectLong,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),				Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DPL(), D0(), NULL),						Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DPL(), D0(), o(1), NULL),				Operation(Read_AAH(), NULL)),
		BusCycle(	Address(DPL(), D0(), o(2), NULL),				Operation(Read_AAB(), NULL)),
		BusCycle(	Address(AAB(), AA(), NULL),						Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(AAB(), Span(AA(), o(1), NULL), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectIndexedWithXCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//16a
	return InstructionCycles(AM_DirectIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),		Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(DP(),  D0(), X(), NULL),		Operation(Read_DataLow(), E8Bit(fOperation, eWidth, bInitialSide), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DP(),  D0(), X(), o(1), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, eWidth, bInitialSide), DONE16Bit(eWidth), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectIndexedWithXWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//16a
	return InstructionCycles(AM_DirectIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),		Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(DP(),  D0(), X(), NULL),		Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DP(),  D0(), X(), o(1), NULL),	Operation(Write_DataHigh(), DONE16Bit(eWidth), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectIndexedWithXRMWCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//16b
	return InstructionCycles(AM_DirectIndexedWithX,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),		Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(DP(),  D0(), X(), NULL),		Operation(Read_DataLow(RMW), NULL)),
		BusCycle(	Address(DP(),  D0(), X(), o(1), NULL),	Operation(Read_DataHigh(RMW), NoteOne(WFR_M), NULL)),
		BusCycle(	Address(DP(),  D0(), X(), oM(1), NULL),	Operation(IO(RMW), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(DP(),  D0(), X(), o(1), NULL),	Operation(NoteThree(), Write_DataHigh(RMW), NoteOne(WFR_M), NULL)),
		BusCycle(	Address(DP(),  D0(), X(), NULL),		Operation(NoteThree(), Write_DataLow(RMW), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectIndexedWithYCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//17
	return InstructionCycles(AM_DirectIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),		Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(DP(),  D0(), Y(), NULL),		Operation(Read_DataLow(), E8Bit(fOperation, eWidth, bInitialSide), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DP(),  D0(), Y(), o(1), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, eWidth, bInitialSide), DONE16Bit(eWidth), NULL)),
		NULL);
}


CW65InstructionCycles* CreateDirectIndexedWithYWriteCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//17
	return InstructionCycles(AM_DirectIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),				Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),		Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(DP(),  D0(), Y(), NULL),		Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(DP(),  D0(), Y(), o(1), NULL),	Operation(Write_DataHigh(), DONE16Bit(eWidth), NULL)),
		NULL);
}


CW65InstructionCycles* CreateImmediateREPSEPCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//18 and note 1: REP and SEP are always 3 cycles, the third is internal with the address bus at PC+1.
	return InstructionCycles(AM_Immediate,
		BusCycle(	Address(PBR(), PC(), NULL),	Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),	Operation(Read_DataLow(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),	Operation(IO(), PC_inc(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateImmediateCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//18
	return InstructionCycles(AM_Immediate,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(PC_inc(), Read_DataLow(), E8Bit(fOperation, eWidth, bInitialSide), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(PC_inc(), Read_DataHigh(), E16Bit(fOperation, eWidth, bInitialSide), DONE16Bit(eWidth), NULL)),
		NULL);
}


CW65InstructionCycles* CreateImpliedCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//19a
	return InstructionCycles(AM_Implied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateWDMImpliedCycles(CW65C816Func fOperation, bool bInitialSide)
{
	return InstructionCycles(AM_Implied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(PC_inc(), IO(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateImpliedXBACycles(CW65C816Func fOperation, bool bInitialSide)
{
	//19b
	return InstructionCycles(AM_Implied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStopTheClockCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//19c
	return InstructionCycles(AM_StopTheClock,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateWaitForInterruptCycles()
{
	//19d
	return InstructionCycles(AM_WaitForInterrupt,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(WaitOperation(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateRelativeShortCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//20
	return InstructionCycles(AM_Relative,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_DataLow(), PC_inc(), E(fOperation, bInitialSide), NULL)),  //Done if branch not taken
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(IO(), NoteSix(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(PC_e(Address(PBR(), PC(), SignedDataLow(), NULL)), IO(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateRelativeLongCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//21
	return InstructionCycles(AM_RelativeLong,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_DataLow(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_DataHigh(), PC_inc(), E(fOperation, bInitialSide), NULL)),  //Done if branch not taken
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(PC_e(Address(PBR(), PC(), SignedData(), NULL)), IO(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackAbortInterruptCycles(CW65InterruptVector* pcInterruptVector, CW65C816Func fOperation, bool bInitialSide)
{
	//22a
	return InstructionCycles(AM_StackInterruptHardware,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(IO(true, true, true), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(IO(), RestoreAbortValues(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PBR(), SP_dec(), PBR_e(0), NoteSeven(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PCH(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PCL(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PS(), SP_dec(), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(VA(pcInterruptVector), NULL),			Operation(Read_AAVL(), NULL)),
		BusCycle(	Address(VA(pcInterruptVector), o(1), NULL),		Operation(Read_AAVH(), PC_e(Address(AA(), NULL)), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackPullCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//22b
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S1(), NULL),			Operation(Read_DataLow(), SP_inc(), E8Bit(fOperation, eWidth, bInitialSide), DONE8Bit(eWidth), NULL)),
		BusCycle(	Address(S1(), NULL),			Operation(Read_DataHigh(), SP_inc(), E16Bit(fOperation, eWidth, bInitialSide), DONE16Bit(eWidth), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackPLBCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//22b
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S16(), o(1), NULL),		Operation(SP_inc(), Read_DataLow(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackPLDCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//22b
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S16(), o(1), NULL),		Operation(Read_DataLow(), SP_inc(), NULL)),
		BusCycle(	Address(S16(), o(1), NULL),		Operation(Read_DataHigh(), SP_inc(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackPLPCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//22b
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S1(), NULL),			Operation(Read_DataLow(), SP_inc(), E(fOperation, bInitialSide), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackImpliedPHPCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//22c
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(E(fOperation, bInitialSide), Write_DataLow(), SP_dec(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackPushCycles(CW65C816Func fOperation, EWidthFromRegister eWidth, bool bInitialSide)
{
	//22c
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_DataHigh(), SP_dec(), NoteOne(eWidth), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(Write_DataLow(), SP_dec(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackPHBCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//22c
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(E(fOperation, bInitialSide), Write_DataLow(), SP_dec(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackPHDCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//22c
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S16(), NULL),			Operation(E(fOperation, bInitialSide), Write_DataHigh(), SP_dec(), NULL)),
		BusCycle(	Address(S16(), NULL),			Operation(Write_DataLow(), SP_dec(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackPHKCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//22c
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S(), NULL),				Operation(E(fOperation, bInitialSide), Write_DataLow(), SP_dec(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackPEACycles()
{
	//22d
	return InstructionCycles(AM_StackImmediate,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAL(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_AAH(), PC_inc(), NULL)),
		BusCycle(	Address(S16(), NULL),			Operation(Write_AAH(), SP_dec(), NULL)),
		BusCycle(	Address(S16(), NULL),			Operation(Write_AAL(), SP_dec(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackPEICycles()
{
	//22e
	return InstructionCycles(AM_StackDirectIndirect,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(IO(), NoteTwo(), NULL)),
		BusCycle(	Address(DPL(), D0(), NULL),			Operation(Read_AAL(), NULL)),
		BusCycle(	Address(DPL(), D0(), o(1), NULL),	Operation(Read_AAH(), NULL)),
		BusCycle(	Address(S16(), NULL),				Operation(Write_AAH(), SP_dec(), NULL)),
		BusCycle(	Address(S16(), NULL),				Operation(Write_AAL(), SP_dec(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackPERCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//22f
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_DataLow(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Read_DataHigh(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(IO(), NULL)),
		BusCycle(	Address(S16(), NULL),			Operation(E(fOperation, bInitialSide), Write_DataHigh(), SP_dec(), NULL)),
		BusCycle(	Address(S16(), NULL),			Operation(Write_DataLow(), SP_dec(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackRTICycles()
{
	//22g
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S1(), NULL),			Operation(Read_DataLow(), SP_inc(), NULL)),  //Processor status, applied at the end.
		BusCycle(	Address(S1(), NULL),			Operation(Read_NewPCL(), SP_inc(), NULL)),
		BusCycle(	Address(S1(), NULL),			Operation(Read_NewPCH(), SP_inc(), PC_e(Address(PBR(), New_PC(), NULL)), E(&CW65C816::RTIStatusIfEmulation, false), DONE_Emulation(), NULL)),
		BusCycle(	Address(S1(), NULL),			Operation(NoteSeven(), Read_NewPBR(), SP_inc(), PC_e(Address(New_PBR(), New_PC(), NULL)), E(&CW65C816::RTIStatus, false), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackRTSCycles()
{
	//22h
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S1(), NULL),			Operation(Read_NewPCL(), SP_inc(), NULL)),
		BusCycle(	Address(S1(), NULL),			Operation(Read_NewPCH(), SP_inc(), NULL)),
		BusCycle(	Address(S(), NULL), 			Operation(IO(), PC_e(Address(PBR(), New_PC(), NULL)), PC_inc(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackRTLCycles()
{
	//22i
	return InstructionCycles(AM_StackImplied,
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),		Operation(IO(), NULL)),
		BusCycle(	Address(S16(), o(1), NULL),		Operation(Read_NewPCL(), SP_inc(), NULL)),
		BusCycle(	Address(S16(), o(1), NULL),		Operation(Read_NewPCH(), SP_inc(), NULL)),
		BusCycle(	Address(S16(), o(1), NULL),		Operation(Read_NewPBR(), SP_inc(), PC_e(Address(New_PBR(), New_PC(), NULL)), PC_inc(), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackSoftwareInterruptCycles(CW65InterruptVector* pcInterruptVector, CW65C816Func fOperation, bool bInitialSide)
{
	//22j
	return InstructionCycles(AM_StackInterruptSoftware,
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),						Operation(IO(true, false, true), PC_inc(), NULL)),  //Read and ignore the signature byte.
		BusCycle(	Address(S(), NULL),								Operation(Write_PBR(), SP_dec(), PBR_e(0), NoteSeven(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PCH(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PCL(), SP_dec(), NULL)),
		BusCycle(	Address(S(), NULL),								Operation(Write_PSB(), SP_dec(), E(fOperation, bInitialSide), NULL)),
		BusCycle(	Address(VA(pcInterruptVector), NULL),			Operation(Read_AAVL(), NULL)),
		BusCycle(	Address(VA(pcInterruptVector), o(1), NULL),		Operation(Read_AAVH(), PC_e(Address(AA(), NULL)), DONE(), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackRelativeCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//23
	return InstructionCycles(AM_StackRelative,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(IO(), NULL)),
		BusCycle(	Address(S(), D0(), NULL),			Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(S(), D0(), o(1), NULL),		Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackRelativeWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//23
	return InstructionCycles(AM_StackRelative,
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),			Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),	Operation(IO(), NULL)),
		BusCycle(	Address(S(), D0(), NULL),			Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(S(), D0(), o(1), NULL),		Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackRelativeIndirectIndexedWithYCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//24
	return InstructionCycles(AM_StackRelativeIndirectIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),					Operation(IO(), NULL)),
		BusCycle(	Address(S(), D0(), NULL),							Operation(Read_AAL(), NULL)),
		BusCycle(	Address(S(), D0(), o(1), NULL),						Operation(Read_AAH(), NULL)),
		BusCycle(	Address(S(), D0(), o(1), NULL),						Operation(IO(), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), Y(), NULL), NULL),		Operation(Read_DataLow(), E8Bit(fOperation, WFR_M, bInitialSide), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), Y(), o(1), NULL), NULL),	Operation(Read_DataHigh(), E16Bit(fOperation, WFR_M, bInitialSide), DONE16Bit(WFR_M), NULL)),
		NULL);
}


CW65InstructionCycles* CreateStackRelativeIndirectIndexedWithYWriteCycles(CW65C816Func fOperation, bool bInitialSide)
{
	//24
	return InstructionCycles(AM_StackRelativeIndirectIndexedWithY,
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Opcode(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), NULL),							Operation(Read_D0(), PC_inc(), NULL)),
		BusCycle(	Address(PBR(), PC(), o(-1), NULL),					Operation(IO(), NULL)),
		BusCycle(	Address(S(), D0(), NULL),							Operation(Read_AAL(), NULL)),
		BusCycle(	Address(S(), D0(), o(1), NULL),						Operation(Read_AAH(), NULL)),
		BusCycle(	Address(S(), D0(), o(1), NULL),						Operation(IO(), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), Y(), NULL), NULL),		Operation(E(fOperation, bInitialSide), Write_DataLow(), DONE8Bit(WFR_M), NULL)),
		BusCycle(	Address(DBR(), Span(AA(), Y(), o(1), NULL), NULL),	Operation(Write_DataHigh(), DONE16Bit(WFR_M), NULL)),
		NULL);
}

