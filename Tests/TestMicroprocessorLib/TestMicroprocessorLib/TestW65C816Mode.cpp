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
void TestW65C816ModeInContext(CTestW65C816Context* pcContext, char* szExpected)
{
	pcContext->SetPrint(false, false, true, false, false, true, true, true, false, true, false, false, true, true);
	TestW65C816InContext(pcContext, szExpected);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816ModeREPAndSEPNative(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(SEP_Immediate);
	cTestContext.SetByte(0xc3);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0xc3);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(SEP_Immediate);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816ModeInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"SEP: A.0000  X.0000  Y.0000  S.01fc    P(Z1 N1 D0 I1 M.8 X.8 C1 E0 V1 ..)\n"\
		"REP: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E0 V0 ..)\n"\
		"REP: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M16 X16 C0 E0 V0 ..)\n"\
		"SEP: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M16 X.8 C0 E0 V0 ..)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816ModeREPAndSEPEmulation(void)
{
	// Datasheet 7.8.3.1: REP and SEP cannot change M and X in emulation mode.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(SEP_Immediate);
	cTestContext.SetByte(0x09);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0xff);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816ModeInContext(&cTestContext, ""\
		"REP: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"SEP: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D1 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"REP: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816ModeXCE(void)
{
	// XCE swaps the C and E flags.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(SEC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816ModeInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"XCE: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"SEC: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"XCE: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816ModeEmulationWithC(CTestW65C816Context* pcContext, uint32 uiAddress, uint16 uiC)
{
	// Loads all 16 bits of C in native mode and then returns to emulation mode.
	// The A column only shows the low byte while M=1, B is still held.
	pcContext->SetOpcd(uiAddress, CLC_Implied);
	pcContext->SetOpcd(XCE_Implied);
	pcContext->SetOpcd(REP_Immediate);
	pcContext->SetByte(0x20);
	pcContext->SetOpcd(LDA_Immediate);
	pcContext->SetShort(uiC);
	pcContext->SetOpcd(SEC_Implied);
	pcContext->SetOpcd(XCE_Implied);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816ModeSwitchToEmulation(void)
{
	// Datasheet 7.10: switching to emulation mode sets M and X, forces SH to 01 and
	// clears XH and YH.  The low bytes of S, X and Y and both bytes of A are kept.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x1ff0);
	cTestContext.SetOpcd(TXS_Implied);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x1234);
	cTestContext.SetOpcd(LDY_Immediate);
	cTestContext.SetShort(0x5678);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0xabcd);
	cTestContext.SetOpcd(SEC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816ModeInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDX: A.0000  X.1ff0  Y.0000  S.01fc    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"TXS: A.0000  X.1ff0  Y.0000  S.1ff0    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDX: A.0000  X.1234  Y.0000  S.1ff0    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDY: A.0000  X.1234  Y.5678  S.1ff0    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDA: A.abcd  X.1234  Y.5678  S.1ff0    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"SEC: A.abcd  X.1234  Y.5678  S.1ff0    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"XCE: A.00cd  X.0034  Y.0078  S.01f0    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"CLC: A.00cd  X.0034  Y.0078  S.01f0    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.00cd  X.0034  Y.0078  S.01f0    P(Z0 N1 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.abcd  X.0034  Y.0078  S.01f0    P(Z0 N1 D0 I1 M16 X.8 C1 E0 V0 ..)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816ModeSetIndexEightBit(void)
{
	// Setting X (here with PLP) clears the high bytes of the index registers.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x1234);
	cTestContext.SetOpcd(LDY_Immediate);
	cTestContext.SetShort(0x5678);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(PHA_StackImplied);
	cTestContext.SetOpcd(PLP_StackImplied);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816ModeInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X16 C1 E0 V0 ..)\n"\
		"LDX: A.0000  X.1234  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X16 C1 E0 V0 ..)\n"\
		"LDY: A.0000  X.1234  Y.5678  S.01fc    P(Z0 N0 D0 I1 M.8 X16 C1 E0 V0 ..)\n"\
		"LDA: A.0010  X.1234  Y.5678  S.01fc    P(Z0 N0 D0 I1 M.8 X16 C1 E0 V0 ..)\n"\
		"PHA: A.0010  X.1234  Y.5678  S.01fb    P(Z0 N0 D0 I1 M.8 X16 C1 E0 V0 ..)\n"\
		"PLP: A.0010  X.0034  Y.0078  S.01fc    P(Z0 N0 D0 I0 M16 X.8 C0 E0 V0 ..)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816ModeAccumulatorBKept(void)
{
	// Datasheet 7.23: the B accumulator keeps its value while M=1, so after
	// switching back to 16 bits A is $2301 and not $0001.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, false, false, true, false, false, false, false, false, false, false, true);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x2345);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(SEP_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetShort(0x3002);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetShort(0x3004);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816RunInContext(&cTestContext, ""\
		"CLC: A.0000\n"\
		"XCE: A.0000\n"\
		"REP: A.0000\n"\
		"LDA: A.2345\n"\
		"STA: A.2345\n"\
		"SEP: A.0045\n"\
		"LDA: A.0001\n"\
		"STA: A.0001\n"\
		"REP: A.2301\n"\
		"STA: A.2301\n");

	AssertShortHex((uint16)0x2345, cTestContext.GetShort(0x3000));
	AssertByteHex((uint8)0x01, cTestContext.GetByte(0x3002));
	AssertByteHex((uint8)0xea, cTestContext.GetByte(0x3003));
	AssertShortHex((uint16)0x2301, cTestContext.GetShort(0x3004));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816ModeXBAEightBit(void)
{
	// XBA swaps B and A.  N and Z come from the new A (low byte).
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x80);
	cTestContext.SetOpcd(XBA_Implied);
	cTestContext.SetOpcd(XBA_Implied);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816ModeInContext(&cTestContext, ""\
		"LDA: A.0080  X.0000  Y.0000  S.01fc    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XBA: A.0000  X.0000  Y.0000  S.01fc    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XBA: A.0080  X.0000  Y.0000  S.01fc    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816ModeXBASixteenBit(void)
{
	// N and Z come from the low byte even when M=0.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x1280);
	cTestContext.SetOpcd(XBA_Implied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x8000);
	cTestContext.SetOpcd(XBA_Implied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x00ff);
	cTestContext.SetOpcd(XBA_Implied);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816ModeInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"LDA: A.1280  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"XBA: A.8012  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"LDA: A.8000  X.0000  Y.0000  S.01fc    P(Z0 N1 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"XBA: A.0080  X.0000  Y.0000  S.01fc    P(Z0 N1 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"LDA: A.00ff  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"XBA: A.ff00  X.0000  Y.0000  S.01fc    P(Z1 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816ModeTransferAccumulatorToSixteenBitIndex(void)
{
	// Datasheet 7.20: transfers take the full 16 bits of the source.  The destination
	// size decides how much is stored.  M=1, X=0 so TAX copies B as well as A.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	TestW65C816ModeEmulationWithC(&cTestContext, 0x0200, 0x9234);
	cTestContext.SetOpcd(CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(TAX_Implied);
	cTestContext.SetOpcd(TAY_Implied);
	cTestContext.SetOpcd(LDY_Immediate);
	cTestContext.SetShort(0x5678);
	cTestContext.SetOpcd(TYA_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816ModeInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"LDA: A.9234  X.0000  Y.0000  S.01fc    P(Z0 N1 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"SEC: A.9234  X.0000  Y.0000  S.01fc    P(Z0 N1 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"XCE: A.0034  X.0000  Y.0000  S.01fc    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"CLC: A.0034  X.0000  Y.0000  S.01fc    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0034  X.0000  Y.0000  S.01fc    P(Z0 N1 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0034  X.0000  Y.0000  S.01fc    P(Z0 N1 D0 I1 M.8 X16 C1 E0 V0 ..)\n"\
		"TAX: A.0034  X.9234  Y.0000  S.01fc    P(Z0 N1 D0 I1 M.8 X16 C1 E0 V0 ..)\n"\
		"TAY: A.0034  X.9234  Y.9234  S.01fc    P(Z0 N1 D0 I1 M.8 X16 C1 E0 V0 ..)\n"\
		"LDY: A.0034  X.9234  Y.5678  S.01fc    P(Z0 N0 D0 I1 M.8 X16 C1 E0 V0 ..)\n"\
		"TYA: A.0078  X.9234  Y.5678  S.01fc    P(Z0 N0 D0 I1 M.8 X16 C1 E0 V0 ..)\n"\
		"REP: A.9278  X.9234  Y.5678  S.01fc    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816ModeTransferIndexToSixteenBitAccumulator(void)
{
	// Datasheet 7.20: M=0, X=1.  The 8 bit index register is transferred as a 16 bit
	// value with a zero high byte, so TXA clears B.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x1280);
	cTestContext.SetOpcd(TAX_Implied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0xffff);
	cTestContext.SetOpcd(TXA_Implied);
	cTestContext.SetOpcd(TXY_Implied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0xffff);
	cTestContext.SetOpcd(TYA_Implied);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816ModeInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"LDA: A.1280  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"TAX: A.1280  X.0080  Y.0000  S.01fc    P(Z0 N1 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"LDA: A.ffff  X.0080  Y.0000  S.01fc    P(Z0 N1 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"TXA: A.0080  X.0080  Y.0000  S.01fc    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"TXY: A.0080  X.0080  Y.0080  S.01fc    P(Z0 N1 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"LDA: A.ffff  X.0080  Y.0080  S.01fc    P(Z0 N1 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"TYA: A.0080  X.0080  Y.0080  S.01fc    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816ModeTransferStackEmulation(void)
{
	// Datasheet 7.21: in emulation mode TCS forces 01 into SH.  TSC always transfers
	// all 16 bits of S into C.  TXS and TCS do not change the flags.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	TestW65C816ModeEmulationWithC(&cTestContext, 0x0200, 0x1280);
	cTestContext.SetOpcd(TCS_Implied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(TSC_Implied);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(TXS_Implied);
	cTestContext.SetOpcd(TSX_Implied);
	cTestContext.SetOpcd(CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816ModeInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"LDA: A.1280  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"SEC: A.1280  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"XCE: A.0080  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"TCS: A.0080  X.0000  Y.0000  S.0180    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDA: A.0000  X.0000  Y.0000  S.0180    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"TSC: A.0080  X.0000  Y.0000  S.0180    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDX: A.0080  X.0000  Y.0000  S.0180    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"TXS: A.0080  X.0000  Y.0000  S.0100    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"TSX: A.0080  X.0000  Y.0000  S.0100    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"CLC: A.0080  X.0000  Y.0000  S.0100    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0080  X.0000  Y.0000  S.0100    P(Z1 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0180  X.0000  Y.0000  S.0100    P(Z1 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816ModeTransferStackNative(void)
{
	// Datasheet 7.21: in native mode TCS copies B into SH even when M=1, and TSC
	// copies SH into B.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	TestW65C816ModeEmulationWithC(&cTestContext, 0x0200, 0x8200);
	cTestContext.SetOpcd(CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(TCS_Implied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(TSC_Implied);
	cTestContext.SetOpcd(TSX_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816ModeInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"LDA: A.8200  X.0000  Y.0000  S.01fc    P(Z0 N1 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"SEC: A.8200  X.0000  Y.0000  S.01fc    P(Z0 N1 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"XCE: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"CLC: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  X.0000  Y.0000  S.01fc    P(Z0 N1 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"TCS: A.0000  X.0000  Y.0000  S.8200    P(Z0 N1 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"LDA: A.0001  X.0000  Y.0000  S.8200    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"TSC: A.0000  X.0000  Y.0000  S.8200    P(Z0 N1 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"TSX: A.0000  X.0000  Y.0000  S.8200    P(Z1 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.8200  X.0000  Y.0000  S.8200    P(Z1 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816ModeTransferDirectPage(void)
{
	// Datasheet 7.20: TCD and TDC are always 16 bit transfers.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, false, false, true, false, false, false, false, true, false, true, true);

	TestW65C816ModeEmulationWithC(&cTestContext, 0x0200, 0x1234);
	cTestContext.SetOpcd(TCD_Implied);
	cTestContext.SetOpcd(CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x0000);
	cTestContext.SetOpcd(SEC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(TDC_Implied);
	cTestContext.SetOpcd(CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816InContext(&cTestContext, ""\
		"CLC: A.0000  DP.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  DP.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0000  DP.0000    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"LDA: A.1234  DP.0000    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"SEC: A.1234  DP.0000    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"XCE: A.0034  DP.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"TCD: A.0034  DP.1234    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"CLC: A.0034  DP.1234    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0034  DP.1234    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.1234  DP.1234    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"LDA: A.0000  DP.1234    P(Z1 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"SEC: A.0000  DP.1234    P(Z1 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n"\
		"XCE: A.0000  DP.1234    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"TDC: A.0034  DP.1234    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"CLC: A.0034  DP.1234    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0034  DP.1234    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.1234  DP.1234    P(Z0 N0 D0 I1 M16 X.8 C1 E0 V0 ..)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816Mode(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();


	TestW65C816ModeREPAndSEPNative();
	TestW65C816ModeREPAndSEPEmulation();
	TestW65C816ModeXCE();
	TestW65C816ModeSwitchToEmulation();
	TestW65C816ModeSetIndexEightBit();
	TestW65C816ModeAccumulatorBKept();
	TestW65C816ModeXBAEightBit();
	TestW65C816ModeXBASixteenBit();
	TestW65C816ModeTransferAccumulatorToSixteenBitIndex();
	TestW65C816ModeTransferIndexToSixteenBitAccumulator();
	TestW65C816ModeTransferStackEmulation();
	TestW65C816ModeTransferStackNative();
	TestW65C816ModeTransferDirectPage();


	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

