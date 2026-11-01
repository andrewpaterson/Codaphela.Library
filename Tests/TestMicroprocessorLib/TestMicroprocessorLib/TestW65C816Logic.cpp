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
void TestW65C816LogicSetPrint(CTestW65C816Context* pcContext)
{
	pcContext->SetPrint(false, false, true, false, false, true, true, true, false, false, false, false, true, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicInContext(CTestW65C816Context* pcContext, char* szExpected)
{
	TestW65C816LogicSetPrint(pcContext);
	TestW65C816InContext(pcContext, szExpected);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicNative(CTestW65C816Context* pcContext)
{
	pcContext->SetOpcd(0x0200, CLC_Implied);
	pcContext->SetOpcd(XCE_Implied);
	pcContext->SetOpcd(REP_Immediate);
	pcContext->SetByte(0x30);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicAND(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0xf0);
	cTestContext.SetOpcd(AND_Immediate);
	cTestContext.SetByte(0x3c);
	cTestContext.SetOpcd(AND_Immediate);
	cTestContext.SetByte(0x0f);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetByte(0xff);
	cTestContext.SetOpcd(AND_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetByte(0x3000, 0x81);

	TestW65C816LogicInContext(&cTestContext, ""\
		"LDA: A.00f0  X.0000  Y.0000    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"AND: A.0030  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"AND: A.0000  X.0000  Y.0000    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDA: A.00ff  X.0000  Y.0000    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"AND: A.0081  X.0000  Y.0000    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicORAAndEOR(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x0f);
	cTestContext.SetOpcd(ORA_Immediate);
	cTestContext.SetByte(0x80);
	cTestContext.SetOpcd(EOR_Immediate);
	cTestContext.SetByte(0x8f);
	cTestContext.SetOpcd(EOR_Immediate);
	cTestContext.SetByte(0x55);
	cTestContext.SetOpcd(ORA_Immediate);
	cTestContext.SetByte(0xaa);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816LogicInContext(&cTestContext, ""\
		"LDA: A.000f  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"ORA: A.008f  X.0000  Y.0000    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"EOR: A.0000  X.0000  Y.0000    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"EOR: A.0055  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"ORA: A.00ff  X.0000  Y.0000    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicLogical16Bit(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	TestW65C816LogicNative(&cTestContext);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0xff00);
	cTestContext.SetOpcd(AND_Immediate);
	cTestContext.SetShort(0x8f0f);
	cTestContext.SetOpcd(ORA_Immediate);
	cTestContext.SetShort(0x00f0);
	cTestContext.SetOpcd(EOR_Immediate);
	cTestContext.SetShort(0x8ff0);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816LogicInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDA: A.ff00  X.0000  Y.0000    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"AND: A.8f00  X.0000  Y.0000    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"ORA: A.8ff0  X.0000  Y.0000    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"EOR: A.0000  X.0000  Y.0000    P(Z1 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicBITImmediate(void)
{
	// Datasheet Table 5-5 note 2: BIT immediate does not affect N and V.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x7f);
	cTestContext.SetOpcd(ADC_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(BIT_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(CLV_Implied);
	cTestContext.SetOpcd(BIT_Immediate);
	cTestContext.SetByte(0xc0);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816LogicInContext(&cTestContext, ""\
		"LDA: A.007f  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"ADC: A.0080  X.0000  Y.0000    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V1 B0)\n"\
		"BIT: A.0080  X.0000  Y.0000    P(Z1 N1 D0 I1 M.8 X.8 C0 E1 V1 B0)\n"\
		"CLV: A.0080  X.0000  Y.0000    P(Z1 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"BIT: A.0080  X.0000  Y.0000    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicBITMemory(void)
{
	// BIT with a memory operand copies bits 7 and 6 of memory into N and V.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(BIT_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(BIT_DirectPage);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(BIT_Absolute);
	cTestContext.SetShort(0x3001);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetByte(0x3000, 0xc0);
	cTestContext.SetByte(0x0010, 0x41);
	cTestContext.SetByte(0x3001, 0x80);

	TestW65C816LogicInContext(&cTestContext, ""\
		"LDA: A.0001  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"BIT: A.0001  X.0000  Y.0000    P(Z1 N1 D0 I1 M.8 X.8 C0 E1 V1 B0)\n"\
		"BIT: A.0001  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V1 B0)\n"\
		"BIT: A.0001  X.0000  Y.0000    P(Z1 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicBITMemory16Bit(void)
{
	// With M=0 BIT copies bits 15 and 14 of memory into N and V.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	TestW65C816LogicNative(&cTestContext);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x0001);
	cTestContext.SetOpcd(BIT_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(BIT_Absolute);
	cTestContext.SetShort(0x3002);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0x3000, 0xc000);
	cTestContext.SetShort(0x3002, 0x00c1);

	TestW65C816LogicInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDA: A.0001  X.0000  Y.0000    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"BIT: A.0001  X.0000  Y.0000    P(Z1 N1 D0 I1 M16 X16 C1 E0 V1 ..)\n"\
		"BIT: A.0001  X.0000  Y.0000    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicTSBAndTRB(void)
{
	// TSB: M | A -> M, TRB: M & ~A -> M.  Z is set from A & M before the change.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x0f);
	cTestContext.SetOpcd(TSB_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(TSB_DirectPage);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(TRB_Absolute);
	cTestContext.SetShort(0x3001);
	cTestContext.SetOpcd(TRB_DirectPage);
	cTestContext.SetByte(0x11);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetByte(0x3000, 0xb0);
	cTestContext.SetByte(0x0010, 0x81);
	cTestContext.SetByte(0x3001, 0xff);
	cTestContext.SetByte(0x0011, 0xc0);

	TestW65C816LogicSetPrint(&cTestContext);
	TestW65C816RunInContext(&cTestContext, ""\
		"LDA: A.000f  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"TSB: A.000f  X.0000  Y.0000    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"TSB: A.000f  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDA: A.0030  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"TRB: A.0030  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"TRB: A.0030  X.0000  Y.0000    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");

	AssertByteHex((uint8)0xbf, cTestContext.GetByte(0x3000));
	AssertByteHex((uint8)0x8f, cTestContext.GetByte(0x0010));
	AssertByteHex((uint8)0xcf, cTestContext.GetByte(0x3001));
	AssertByteHex((uint8)0xc0, cTestContext.GetByte(0x0011));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicCompare(void)
{
	// CMP, CPX, CPY: C = register >= memory (unsigned), Z = equal, N = bit 7 of the difference.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x40);
	cTestContext.SetOpcd(CMP_Immediate);
	cTestContext.SetByte(0x40);
	cTestContext.SetOpcd(CMP_Immediate);
	cTestContext.SetByte(0x41);
	cTestContext.SetOpcd(CMP_Immediate);
	cTestContext.SetByte(0x3f);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetByte(0x80);
	cTestContext.SetOpcd(CPX_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(LDY_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(CPY_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetByte(0x3000, 0xff);

	TestW65C816LogicInContext(&cTestContext, ""\
		"LDA: A.0040  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"CMP: A.0040  X.0000  Y.0000    P(Z1 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"CMP: A.0040  X.0000  Y.0000    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"CMP: A.0040  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"LDX: A.0040  X.0080  Y.0000    P(Z0 N1 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"CPX: A.0040  X.0080  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"LDY: A.0040  X.0080  Y.0001    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"CPY: A.0040  X.0080  Y.0001    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicCompare16Bit(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	TestW65C816LogicNative(&cTestContext);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x1234);
	cTestContext.SetOpcd(CMP_Immediate);
	cTestContext.SetShort(0x1235);
	cTestContext.SetOpcd(CMP_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x8000);
	cTestContext.SetOpcd(CPX_Immediate);
	cTestContext.SetShort(0x8000);
	cTestContext.SetOpcd(LDY_Immediate);
	cTestContext.SetShort(0x0100);
	cTestContext.SetOpcd(CPY_Immediate);
	cTestContext.SetShort(0x00ff);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0x3000, 0x1234);

	TestW65C816LogicInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDA: A.1234  X.0000  Y.0000    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"CMP: A.1234  X.0000  Y.0000    P(Z0 N1 D0 I1 M16 X16 C0 E0 V0 ..)\n"\
		"CMP: A.1234  X.0000  Y.0000    P(Z1 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDX: A.1234  X.8000  Y.0000    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"CPX: A.1234  X.8000  Y.0000    P(Z1 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDY: A.1234  X.8000  Y.0100    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"CPY: A.1234  X.8000  Y.0100    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicShiftAccumulator(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x81);
	cTestContext.SetOpcd(ASL_Accumulator);
	cTestContext.SetOpcd(LSR_Accumulator);
	cTestContext.SetOpcd(LSR_Accumulator);
	cTestContext.SetOpcd(SEC_Implied);
	cTestContext.SetOpcd(ROR_Accumulator);
	cTestContext.SetOpcd(ROL_Accumulator);
	cTestContext.SetOpcd(ROL_Accumulator);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816LogicInContext(&cTestContext, ""\
		"LDA: A.0081  X.0000  Y.0000    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"ASL: A.0002  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"LSR: A.0001  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LSR: A.0000  X.0000  Y.0000    P(Z1 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"SEC: A.0000  X.0000  Y.0000    P(Z1 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"ROR: A.0080  X.0000  Y.0000    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"ROL: A.0000  X.0000  Y.0000    P(Z1 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"ROL: A.0001  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicShiftAccumulator16Bit(void)
{
	// With M=0 the shifts and rotates move bit 15 into and out of the carry.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	TestW65C816LogicNative(&cTestContext);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x8001);
	cTestContext.SetOpcd(ASL_Accumulator);
	cTestContext.SetOpcd(ROR_Accumulator);
	cTestContext.SetOpcd(LSR_Accumulator);
	cTestContext.SetOpcd(ROL_Accumulator);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816LogicInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDA: A.8001  X.0000  Y.0000    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"ASL: A.0002  X.0000  Y.0000    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"ROR: A.8001  X.0000  Y.0000    P(Z0 N1 D0 I1 M16 X16 C0 E0 V0 ..)\n"\
		"LSR: A.4000  X.0000  Y.0000    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"ROL: A.8001  X.0000  Y.0000    P(Z0 N1 D0 I1 M16 X16 C0 E0 V0 ..)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicShiftMemory(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, ASL_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(LSR_DirectPage);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(ROL_AbsoluteIndexedWithX);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(ROR_DirectPageIndexedWithX);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetByte(0x3000, 0xc1);
	cTestContext.SetByte(0x0010, 0x02);
	cTestContext.SetByte(0x3001, 0x40);
	cTestContext.SetByte(0x0011, 0x03);

	TestW65C816LogicSetPrint(&cTestContext);
	TestW65C816RunInContext(&cTestContext, ""\
		"ASL: A.0000  X.0000  Y.0000    P(Z0 N1 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"LSR: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDX: A.0000  X.0001  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"ROL: A.0000  X.0001  Y.0000    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"ROR: A.0000  X.0001  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n");

	AssertByteHex((uint8)0x82, cTestContext.GetByte(0x3000));
	AssertByteHex((uint8)0x01, cTestContext.GetByte(0x0010));
	AssertByteHex((uint8)0x80, cTestContext.GetByte(0x3001));
	AssertByteHex((uint8)0x01, cTestContext.GetByte(0x0011));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicIncrementDecrement(void)
{
	// INC and DEC wrap and set N and Z but leave C alone.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0xff);
	cTestContext.SetOpcd(INC_Accumulator);
	cTestContext.SetOpcd(DEC_Accumulator);
	cTestContext.SetOpcd(DEX_Implied);
	cTestContext.SetOpcd(INX_Implied);
	cTestContext.SetOpcd(DEY_Implied);
	cTestContext.SetOpcd(INY_Implied);
	cTestContext.SetOpcd(INY_Implied);
	cTestContext.SetOpcd(INC_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(DEC_DirectPage);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetByte(0x3000, 0x7f);
	cTestContext.SetByte(0x0010, 0x00);

	TestW65C816LogicSetPrint(&cTestContext);
	TestW65C816RunInContext(&cTestContext, ""\
		"LDA: A.00ff  X.0000  Y.0000    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"INC: A.0000  X.0000  Y.0000    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"DEC: A.00ff  X.0000  Y.0000    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"DEX: A.00ff  X.00ff  Y.0000    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"INX: A.00ff  X.0000  Y.0000    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"DEY: A.00ff  X.0000  Y.00ff    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"INY: A.00ff  X.0000  Y.0000    P(Z1 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"INY: A.00ff  X.0000  Y.0001    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"INC: A.00ff  X.0000  Y.0001    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"DEC: A.00ff  X.0000  Y.0001    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");

	AssertByteHex((uint8)0x80, cTestContext.GetByte(0x3000));
	AssertByteHex((uint8)0xff, cTestContext.GetByte(0x0010));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicIncrementDecrement16Bit(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	TestW65C816LogicNative(&cTestContext);
	cTestContext.SetOpcd(DEX_Implied);
	cTestContext.SetOpcd(INY_Implied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x00ff);
	cTestContext.SetOpcd(INC_Accumulator);
	cTestContext.SetOpcd(INC_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(DEC_Absolute);
	cTestContext.SetShort(0x3002);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0x3000, 0xffff);
	cTestContext.SetShort(0x3002, 0x0100);

	TestW65C816LogicSetPrint(&cTestContext);
	TestW65C816RunInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"DEX: A.0000  X.ffff  Y.0000    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"INY: A.0000  X.ffff  Y.0001    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDA: A.00ff  X.ffff  Y.0001    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"INC: A.0100  X.ffff  Y.0001    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"INC: A.0100  X.ffff  Y.0001    P(Z1 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"DEC: A.0100  X.ffff  Y.0001    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n");

	AssertShortHex((uint16)0x0000, cTestContext.GetShort(0x3000));
	AssertShortHex((uint16)0x00ff, cTestContext.GetShort(0x3002));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LogicFlagInstructions(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, SEC_Implied);
	cTestContext.SetOpcd(SED_Implied);
	cTestContext.SetOpcd(CLI_Implied);
	cTestContext.SetOpcd(CLD_Implied);
	cTestContext.SetOpcd(SEI_Implied);
	cTestContext.SetOpcd(CLC_Implied);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816LogicInContext(&cTestContext, ""\
		"SEC: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"SED: A.0000  X.0000  Y.0000    P(Z0 N0 D1 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"CLI: A.0000  X.0000  Y.0000    P(Z0 N0 D1 I0 M.8 X.8 C1 E1 V0 B0)\n"\
		"CLD: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I0 M.8 X.8 C1 E1 V0 B0)\n"\
		"SEI: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"CLC: A.0000  X.0000  Y.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816Logic(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();


	TestW65C816LogicAND();
	TestW65C816LogicORAAndEOR();
	TestW65C816LogicLogical16Bit();
	TestW65C816LogicBITImmediate();
	TestW65C816LogicBITMemory();
	TestW65C816LogicBITMemory16Bit();
	TestW65C816LogicTSBAndTRB();
	TestW65C816LogicCompare();
	TestW65C816LogicCompare16Bit();
	TestW65C816LogicShiftAccumulator();
	TestW65C816LogicShiftAccumulator16Bit();
	TestW65C816LogicShiftMemory();
	TestW65C816LogicIncrementDecrement();
	TestW65C816LogicIncrementDecrement16Bit();
	TestW65C816LogicFlagInstructions();


	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

