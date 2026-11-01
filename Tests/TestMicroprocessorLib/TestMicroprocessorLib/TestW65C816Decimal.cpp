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
void TestW65C816DecimalInContext(CTestW65C816Context* pcContext, char* szExpected)
{
	// Datasheet table 7-1: N, V and Z are valid in decimal mode on the W65C816S.
	pcContext->SetPrint(false, false, true, false, false, true, false, false, false, false, false, false, true, true);
	pcContext->SetOpcd(STP_Implied);
	TestW65C816InContext(pcContext, szExpected);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816DecimalAdd(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, SED_Implied);
	cTestContext.SetOpcd(CLC_Implied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetByte(0x09);
	cTestContext.SetOpcd(ADC_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(ADC_Immediate);
	cTestContext.SetByte(0x80);
	cTestContext.SetOpcd(ADC_Immediate);
	cTestContext.SetByte(0x09);
	cTestContext.SetOpcd(SEC_Implied);
	cTestContext.SetOpcd(ADC_Immediate);
	cTestContext.SetByte(0x00);

	TestW65C816DecimalInContext(&cTestContext, ""\
		"SED: A.0000    P(Z0 N0 D1 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"CLC: A.0000    P(Z0 N0 D1 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDA: A.0009    P(Z0 N0 D1 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"ADC: A.0010    P(Z0 N0 D1 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"ADC: A.0090    P(Z0 N1 D1 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"ADC: A.0099    P(Z0 N1 D1 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"SEC: A.0099    P(Z0 N1 D1 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"ADC: A.0000    P(Z1 N0 D1 I1 M.8 X.8 C1 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816DecimalSubtract(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, SED_Implied);
	cTestContext.SetOpcd(SEC_Implied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(SBC_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(SBC_Immediate);
	cTestContext.SetByte(0x09);
	cTestContext.SetOpcd(SBC_Immediate);
	cTestContext.SetByte(0x01);

	TestW65C816DecimalInContext(&cTestContext, ""\
		"SED: A.0000    P(Z0 N0 D1 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"SEC: A.0000    P(Z0 N0 D1 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"LDA: A.0010    P(Z0 N0 D1 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"SBC: A.0009    P(Z0 N0 D1 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"SBC: A.0000    P(Z1 N0 D1 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"SBC: A.0099    P(Z0 N1 D1 I1 M.8 X.8 C0 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816DecimalAdd16Bit(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x31);
	cTestContext.SetOpcd(SED_Implied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x1999);
	cTestContext.SetOpcd(ADC_Immediate);
	cTestContext.SetShort(0x0001);
	cTestContext.SetOpcd(ADC_Immediate);
	cTestContext.SetShort(0x8000);

	TestW65C816DecimalInContext(&cTestContext, ""\
		"CLC: A.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0000    P(Z0 N0 D0 I1 M16 X16 C0 E0 V0 ..)\n"\
		"SED: A.0000    P(Z0 N0 D1 I1 M16 X16 C0 E0 V0 ..)\n"\
		"LDA: A.1999    P(Z0 N0 D1 I1 M16 X16 C0 E0 V0 ..)\n"\
		"ADC: A.2000    P(Z0 N0 D1 I1 M16 X16 C0 E0 V0 ..)\n"\
		"ADC: A.0000    P(Z1 N0 D1 I1 M16 X16 C1 E0 V0 ..)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816DecimalSubtract16Bit(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(SED_Implied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x2000);
	cTestContext.SetOpcd(SBC_Immediate);
	cTestContext.SetShort(0x0001);
	cTestContext.SetOpcd(SBC_Immediate);
	cTestContext.SetShort(0x2000);

	TestW65C816DecimalInContext(&cTestContext, ""\
		"CLC: A.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0000    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"SED: A.0000    P(Z0 N0 D1 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDA: A.2000    P(Z0 N0 D1 I1 M16 X16 C1 E0 V0 ..)\n"\
		"SBC: A.1999    P(Z0 N0 D1 I1 M16 X16 C1 E0 V0 ..)\n"\
		"SBC: A.9999    P(Z0 N1 D1 I1 M16 X16 C0 E0 V0 ..)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816Decimal(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();


	TestW65C816DecimalAdd();
	TestW65C816DecimalSubtract();
	TestW65C816DecimalAdd16Bit();
	TestW65C816DecimalSubtract16Bit();


	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

