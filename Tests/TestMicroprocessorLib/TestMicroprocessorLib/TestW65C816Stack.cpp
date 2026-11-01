#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "MicroprocessorLib/W65C816.h"
#include "MicroprocessorLib/W65C816InstructionFactory.h"
#include "MicroprocessorLib/W65C816Meta.h"
#include "TestLib/Assert.h"
#include "TestW65C816Context.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816StackSetPrint(CTestW65C816Context* pcContext)
{
	pcContext->SetPrint(false, false, true, false, false, true, true, true, true, true, false, false, false, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816StackPushPullEightBit(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, false, false, true, true, true, false, true, false, false, true, true);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x92);
	cTestContext.SetOpcd(PHA_StackImplied);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetByte(0x34);
	cTestContext.SetOpcd(PHX_StackImplied);
	cTestContext.SetOpcd(LDY_Immediate);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(PHY_StackImplied);
	cTestContext.SetOpcd(PLA_Stack);
	cTestContext.SetOpcd(PLY_StackImplied);
	cTestContext.SetOpcd(PLX_StackImplied);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816StackSetPrint(&cTestContext);
	TestW65C816RunInContext(&cTestContext, ""\
		"LDA: A.0092  X.0000  Y.0000  S.01fc    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"PHA: A.0092  X.0000  Y.0000  S.01fb    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDX: A.0092  X.0034  Y.0000  S.01fb    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"PHX: A.0092  X.0034  Y.0000  S.01fa    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDY: A.0092  X.0034  Y.0000  S.01fa    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"PHY: A.0092  X.0034  Y.0000  S.01f9    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"PLA: A.0000  X.0034  Y.0000  S.01fa    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"PLY: A.0000  X.0034  Y.0034  S.01fb    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"PLX: A.0000  X.0092  Y.0034  S.01fc    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");

	AssertByteHex((uint8)0x92, cTestContext.GetByte(0x01fc));
	AssertByteHex((uint8)0x34, cTestContext.GetByte(0x01fb));
	AssertByteHex((uint8)0x00, cTestContext.GetByte(0x01fa));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816StackPushPullSixteenBit(void)
{
	// The high byte is pushed first so the value is little endian in memory.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x1234);
	cTestContext.SetOpcd(PHA_StackImplied);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x5678);
	cTestContext.SetOpcd(PHX_StackImplied);
	cTestContext.SetOpcd(LDY_Immediate);
	cTestContext.SetShort(0x9abc);
	cTestContext.SetOpcd(PHY_StackImplied);
	cTestContext.SetOpcd(PLA_Stack);
	cTestContext.SetOpcd(PLX_StackImplied);
	cTestContext.SetOpcd(PLY_StackImplied);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816StackSetPrint(&cTestContext);
	TestW65C816RunInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000  PC.00:0201  S.01fc\n"\
		"XCE: A.0000  X.0000  Y.0000  PC.00:0202  S.01fc\n"\
		"REP: A.0000  X.0000  Y.0000  PC.00:0204  S.01fc\n"\
		"LDA: A.1234  X.0000  Y.0000  PC.00:0207  S.01fc\n"\
		"PHA: A.1234  X.0000  Y.0000  PC.00:0208  S.01fa\n"\
		"LDX: A.1234  X.5678  Y.0000  PC.00:020b  S.01fa\n"\
		"PHX: A.1234  X.5678  Y.0000  PC.00:020c  S.01f8\n"\
		"LDY: A.1234  X.5678  Y.9abc  PC.00:020f  S.01f8\n"\
		"PHY: A.1234  X.5678  Y.9abc  PC.00:0210  S.01f6\n"\
		"PLA: A.9abc  X.5678  Y.9abc  PC.00:0211  S.01f8\n"\
		"PLX: A.9abc  X.5678  Y.9abc  PC.00:0212  S.01fa\n"\
		"PLY: A.9abc  X.5678  Y.1234  PC.00:0213  S.01fc\n");

	AssertShortHex((uint16)0x1234, cTestContext.GetShort(0x01fb));
	AssertShortHex((uint16)0x5678, cTestContext.GetShort(0x01f9));
	AssertShortHex((uint16)0x9abc, cTestContext.GetShort(0x01f7));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816StackPushPullProcessorStatus(void)
{
	// Datasheet 7.20: PHP and PLP are always 8 bit operations.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, false, false, false, false, false, false, true, false, false, true, true);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(SEC_Implied);
	cTestContext.SetOpcd(PHP_StackImplied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x00c3);
	cTestContext.SetOpcd(PHA_StackImplied);
	cTestContext.SetOpcd(PLP_StackImplied);
	cTestContext.SetOpcd(PLP_StackImplied);
	cTestContext.SetOpcd(PLP_StackImplied);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816RunInContext(&cTestContext, ""\
		"CLC: S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: S.01fc    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"SEC: S.01fc    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"PHP: S.01fb    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDA: S.01fb    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"PHA: S.01f9    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"PLP: S.01fa    P(Z1 N1 D0 I0 M16 X16 C1 E0 V1 ..)\n"\
		"PLP: S.01fb    P(Z0 N0 D0 I0 M16 X16 C0 E0 V0 ..)\n"\
		"PLP: S.01fc    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n");

	AssertByteHex((uint8)0x05, cTestContext.GetByte(0x01fc));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816StackDataAndProgramBank(void)
{
	// Datasheet 7.8.2: PHK, PHB and PLB can be used in emulation mode.  PLB sets N and Z.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, false, false, true, false, false, false, true, false, true, true, true);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x85);
	cTestContext.SetOpcd(PHA_StackImplied);
	cTestContext.SetOpcd(PLB_StackImplied);
	cTestContext.SetOpcd(PHB_StackImplied);
	cTestContext.SetOpcd(PHK_StackImplied);
	cTestContext.SetOpcd(PLB_StackImplied);
	cTestContext.SetOpcd(PLA_Stack);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816RunInContext(&cTestContext, ""\
		"LDA: A.0085  S.01fc  DB.00    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"PHA: A.0085  S.01fb  DB.00    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"PLB: A.0085  S.01fc  DB.85    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"PHB: A.0085  S.01fb  DB.85    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"PHK: A.0085  S.01fa  DB.85    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"PLB: A.0085  S.01fb  DB.00    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"PLA: A.0085  S.01fc  DB.00    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");

	AssertByteHex((uint8)0x85, cTestContext.GetByte(0x01fc));
	AssertByteHex((uint8)0x00, cTestContext.GetByte(0x01fb));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816StackDirectPage(void)
{
	// PHD and PLD always move 16 bits.  PLD sets N and Z from the 16 bit value.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, false, false, false, false, false, false, true, true, false, true, true);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x8034);
	cTestContext.SetOpcd(TCD_Implied);
	cTestContext.SetOpcd(PHD_StackImplied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x0000);
	cTestContext.SetOpcd(TCD_Implied);
	cTestContext.SetOpcd(PLD_StackImplied);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816RunInContext(&cTestContext, ""\
		"CLC: S.01fc  DP.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: S.01fc  DP.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: S.01fc  DP.0000    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"LDA: S.01fc  DP.0000    P(Z0 N1 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"TCD: S.01fc  DP.8034    P(Z0 N1 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"PHD: S.01fa  DP.8034    P(Z0 N1 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"LDA: S.01fa  DP.8034    P(Z1 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"TCD: S.01fa  DP.0000    P(Z1 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"PLD: S.01fc  DP.8034    P(Z0 N1 D0 I1 M16 X.8 C1 E0 V0 ..)\n");

	AssertShortHex((uint16)0x8034, cTestContext.GetShort(0x01fb));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816StackPEA(void)
{
	// PEA is three bytes long and pushes its 16 bit operand, high byte first.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, PEA_StackImmediate);
	cTestContext.SetShort(0x1234);
	cTestContext.SetOpcd(PLA_Stack);
	cTestContext.SetOpcd(PLX_StackImplied);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816StackSetPrint(&cTestContext);
	TestW65C816RunInContext(&cTestContext, ""\
		"PEA: A.0000  X.0000  Y.0000  PC.00:0203  S.01fa\n"\
		"PLA: A.0034  X.0000  Y.0000  PC.00:0204  S.01fb\n"\
		"PLX: A.0034  X.0012  Y.0000  PC.00:0205  S.01fc\n");

	AssertShortHex((uint16)0x1234, cTestContext.GetShort(0x01fb));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816StackPEI(void)
{
	// PEI pushes the 16 bit value at the direct page address.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, PEI_StackDirectPageIndirect);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(PLA_Stack);
	cTestContext.SetOpcd(PLX_StackImplied);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0x0010, 0x5678);

	TestW65C816StackSetPrint(&cTestContext);
	TestW65C816RunInContext(&cTestContext, ""\
		"PEI: A.0000  X.0000  Y.0000  PC.00:0202  S.01fa\n"\
		"PLA: A.0078  X.0000  Y.0000  PC.00:0203  S.01fb\n"\
		"PLX: A.0078  X.0056  Y.0000  PC.00:0204  S.01fc\n");

	AssertShortHex((uint16)0x5678, cTestContext.GetShort(0x01fb));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816StackPER(void)
{
	// PER pushes the address of the next instruction plus the 16 bit displacement.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, PER_StackProgramCounterRelativeLong);
	cTestContext.SetShort(0x0010);
	cTestContext.SetOpcd(PER_StackProgramCounterRelativeLong);
	cTestContext.SetShort(0xfff0);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816StackSetPrint(&cTestContext);
	TestW65C816RunInContext(&cTestContext, ""\
		"PER: A.0000  X.0000  Y.0000  PC.00:0203  S.01fa\n"\
		"PER: A.0000  X.0000  Y.0000  PC.00:0206  S.01f8\n");

	AssertShortHex((uint16)0x0213, cTestContext.GetShort(0x01fb));
	AssertShortHex((uint16)0x01f6, cTestContext.GetShort(0x01f9));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816StackJSRAndRTS(void)
{
	// JSR pushes the address of its last byte, RTS pulls it and adds one.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, JSR_Absolute);
	cTestContext.SetShort(0x0300);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetOpcd(0x0300, LDA_Immediate);
	cTestContext.SetByte(0x55);
	cTestContext.SetOpcd(RTS_StackImplied);

	TestW65C816StackSetPrint(&cTestContext);
	TestW65C816RunInContext(&cTestContext, ""\
		"JSR: A.0000  X.0000  Y.0000  PC.00:0300  S.01fa\n"\
		"LDA: A.0055  X.0000  Y.0000  PC.00:0302  S.01fa\n"\
		"RTS: A.0055  X.0000  Y.0000  PC.00:0203  S.01fc\n");

	AssertShortHex((uint16)0x0202, cTestContext.GetShort(0x01fb));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816StackJSRAbsoluteIndexedIndirect(void)
{
	// Datasheet 7.9: JSR (a,x) reads its address table from the program bank.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDX_Immediate);
	cTestContext.SetByte(0x02);
	cTestContext.SetOpcd(JSR_AbsoluteIndexedIndirectWithX);
	cTestContext.SetShort(0x0400);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0x0402, 0x0300);

	cTestContext.SetOpcd(0x0300, RTS_StackImplied);

	TestW65C816StackSetPrint(&cTestContext);
	TestW65C816RunInContext(&cTestContext, ""\
		"LDX: A.0000  X.0002  Y.0000  PC.00:0202  S.01fc\n"\
		"JSR: A.0000  X.0002  Y.0000  PC.00:0300  S.01fa\n"\
		"RTS: A.0000  X.0002  Y.0000  PC.00:0205  S.01fc\n");

	AssertShortHex((uint16)0x0204, cTestContext.GetShort(0x01fb));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816StackJSLAndRTL(void)
{
	// JSL pushes PBR and then the address of its last byte.  RTL pulls them back.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x20000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, JSL_AbsoluteLong);
	cTestContext.SetShort(0x8000);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetOpcd(0x18000, PHK_StackImplied);
	cTestContext.SetOpcd(PLA_Stack);
	cTestContext.SetOpcd(RTL_StackImplied);

	TestW65C816StackSetPrint(&cTestContext);
	TestW65C816RunInContext(&cTestContext, ""\
		"JSL: A.0000  X.0000  Y.0000  PC.01:8000  S.01f9\n"\
		"PHK: A.0000  X.0000  Y.0000  PC.01:8001  S.01f8\n"\
		"PLA: A.0001  X.0000  Y.0000  PC.01:8002  S.01f9\n"\
		"RTL: A.0001  X.0000  Y.0000  PC.00:0204  S.01fc\n");

	AssertByteHex((uint8)0x00, cTestContext.GetByte(0x01fc));
	AssertShortHex((uint16)0x0203, cTestContext.GetShort(0x01fa));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816StackEmulationPageWrap(void)
{
	// Datasheet 7.1: in emulation mode the stack is 000100 to 0001FF, PHA and PLA wrap
	// within page one.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDX_Immediate);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(TXS_Implied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetByte(0xaa);
	cTestContext.SetOpcd(PHA_StackImplied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetByte(0xbb);
	cTestContext.SetOpcd(PHA_StackImplied);
	cTestContext.SetOpcd(PLA_Stack);
	cTestContext.SetOpcd(PLA_Stack);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816StackSetPrint(&cTestContext);
	TestW65C816RunInContext(&cTestContext, ""\
		"LDX: A.0000  X.0000  Y.0000  PC.00:0202  S.01fc\n"\
		"TXS: A.0000  X.0000  Y.0000  PC.00:0203  S.0100\n"\
		"LDA: A.00aa  X.0000  Y.0000  PC.00:0205  S.0100\n"\
		"PHA: A.00aa  X.0000  Y.0000  PC.00:0206  S.01ff\n"\
		"LDA: A.00bb  X.0000  Y.0000  PC.00:0208  S.01ff\n"\
		"PHA: A.00bb  X.0000  Y.0000  PC.00:0209  S.01fe\n"\
		"PLA: A.00bb  X.0000  Y.0000  PC.00:020a  S.01ff\n"\
		"PLA: A.00aa  X.0000  Y.0000  PC.00:020b  S.0100\n");

	AssertByteHex((uint8)0xaa, cTestContext.GetByte(0x0100));
	AssertByteHex((uint8)0xbb, cTestContext.GetByte(0x01ff));
	AssertByteHex((uint8)0xea, cTestContext.GetByte(0x00ff));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816StackEmulationJSLLeavesPage(void)
{
	// Datasheet 7.1: JSL accesses three bytes and in emulation mode can go below
	// page one.  S stays in page one.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDX_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(TXS_Implied);
	cTestContext.SetOpcd(JSL_AbsoluteLong);
	cTestContext.SetShort(0x0300);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetOpcd(0x0300, STP_Implied);
	cTestContext.muiInstructions--;

	TestW65C816StackSetPrint(&cTestContext);
	TestW65C816RunInContext(&cTestContext, ""\
		"LDX: A.0000  X.0001  Y.0000  PC.00:0202  S.01fc\n"\
		"TXS: A.0000  X.0001  Y.0000  PC.00:0203  S.0101\n"\
		"JSL: A.0000  X.0001  Y.0000  PC.00:0300  S.01fe\n");

	AssertByteHex((uint8)0x00, cTestContext.GetByte(0x0101));
	AssertByteHex((uint8)0x02, cTestContext.GetByte(0x0100));
	AssertByteHex((uint8)0x06, cTestContext.GetByte(0x00ff));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816Stack(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();


	TestW65C816StackPushPullEightBit();
	TestW65C816StackPushPullSixteenBit();
	TestW65C816StackPushPullProcessorStatus();
	TestW65C816StackDataAndProgramBank();
	TestW65C816StackDirectPage();
	TestW65C816StackPEA();
	TestW65C816StackPEI();
	TestW65C816StackPER();
	TestW65C816StackJSRAndRTS();
	TestW65C816StackJSRAbsoluteIndexedIndirect();
	TestW65C816StackJSLAndRTL();
	TestW65C816StackEmulationPageWrap();
	TestW65C816StackEmulationJSLLeavesPage();


	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

