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
void TestW65C816DirectPageSetD(CTestW65C816Context* pcContext, uint16 uiDirectPage, bool bEmulation)
{
	// Sets D with TCD from native mode and optionally returns to emulation mode.
	pcContext->SetOpcd(0x0200, CLC_Implied);
	pcContext->SetOpcd(XCE_Implied);
	pcContext->SetOpcd(REP_Immediate);
	pcContext->SetByte(0x20);
	pcContext->SetOpcd(LDA_Immediate);
	pcContext->SetShort(uiDirectPage);
	pcContext->SetOpcd(TCD_Implied);
	pcContext->SetOpcd(LDA_Immediate);
	pcContext->SetShort(0x0000);
	if (bEmulation)
	{
		pcContext->SetOpcd(SEC_Implied);
		pcContext->SetOpcd(XCE_Implied);
	}
	else
	{
		pcContext->SetOpcd(REP_Immediate);
		pcContext->SetByte(0x10);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816DirectPageRun(CTestW65C816Context* pcContext, char* szExpectedLast)
{
	// Only the trace line of the last instruction is compared.
	char*	szSequence;
	size	uiStart;

	pcContext->SetPrint(false, false, true, false, false, true, true, false, false, false, true, false, false, true);
	TestW65C816RunInContext(pcContext, NULL);

	szSequence = pcContext->SequenceText();
	uiStart = strlen(szSequence);
	if (uiStart > 0)
	{
		uiStart--;
	}
	while ((uiStart > 0) && (szSequence[uiStart - 1] != '\n'))
	{
		uiStart--;
	}
	AssertString(szExpectedLast, &szSequence[uiStart]);

	pcContext->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816DirectPageNative(void)
{
	// Datasheet 3.3: direct addresses are D + offset in bank 0 and may cross pages.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	TestW65C816DirectPageSetD(&cTestContext, 0x12f0, false);
	cTestContext.SetOpcd(LDA_DirectPage);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0x1310, 0xbeef);

	TestW65C816DirectPageRun(&cTestContext, "LDA: A.beef  X.0000  DP.12f0\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816DirectPageNativeIndexedWraps(void)
{
	// Datasheet 7.2: direct, X stays in bank 0 and wraps at FFFF.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x20000, 0xea, 0x0200);

	TestW65C816DirectPageSetD(&cTestContext, 0xff00, false);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x0100);
	cTestContext.SetOpcd(LDA_DirectPageIndexedWithX);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0x0010, 0xbeef);
	cTestContext.SetShort(0x10010, 0x9999);

	TestW65C816DirectPageRun(&cTestContext, "LDA: A.beef  X.0100  DP.ff00\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816DirectPageEmulationZeroPageWraps(void)
{
	// Datasheet 7.2.1: emulation mode with D=0000, direct, X wraps within page 0.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDX_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(LDA_DirectPageIndexedWithX);
	cTestContext.SetByte(0xf0);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetByte(0x0010, 0x42);
	cTestContext.SetByte(0x0110, 0x99);

	TestW65C816DirectPageRun(&cTestContext, "LDA: A.0042  X.0020  DP.0000\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816DirectPageEmulationPageWraps(void)
{
	// Datasheet 7.2.2: emulation mode with DL=00 and DH not zero, direct, X wraps
	// within page DH.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	TestW65C816DirectPageSetD(&cTestContext, 0x1200, true);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(LDA_DirectPageIndexedWithX);
	cTestContext.SetByte(0xf0);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetByte(0x1210, 0x42);
	cTestContext.SetByte(0x1310, 0x99);

	TestW65C816DirectPageRun(&cTestContext, "LDA: A.0042  X.0020  DP.1200\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816DirectPageEmulationLowNotZero(void)
{
	// Datasheet 7.2.3: emulation mode with DL not zero, direct, X does not wrap.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	TestW65C816DirectPageSetD(&cTestContext, 0x1201, true);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(LDA_DirectPageIndexedWithX);
	cTestContext.SetByte(0xf0);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetByte(0x1211, 0x99);
	cTestContext.SetByte(0x1311, 0x42);

	TestW65C816DirectPageRun(&cTestContext, "LDA: A.0042  X.0020  DP.1201\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816DirectPageEmulationDirectWraps(void)
{
	// Datasheet 7.2.2: emulation mode with DL=00, a direct indirect (d) pointer at
	// DH:FF wraps to DH:00 for its high byte.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	TestW65C816DirectPageSetD(&cTestContext, 0x1200, true);
	cTestContext.SetOpcd(LDA_DirectPageIndirect);
	cTestContext.SetByte(0xff);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetByte(0x12ff, 0x00);
	cTestContext.SetByte(0x1200, 0x30);
	cTestContext.SetByte(0x1300, 0x40);
	cTestContext.SetByte(0x3000, 0x42);
	cTestContext.SetByte(0x4000, 0x99);

	TestW65C816DirectPageRun(&cTestContext, "LDA: A.0042  X.0000  DP.1200\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816DirectPageEmulationIndirectLongDoesNotWrap(void)
{
	// Datasheet 7.2.1: [d] is the exception, its pointer at 00FF continues into
	// 0100 and 0101.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x20000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDA_DirectPageIndirectLong);
	cTestContext.SetByte(0xff);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetByte(0x00ff, 0x00);
	cTestContext.SetByte(0x0100, 0x30);
	cTestContext.SetByte(0x0101, 0x01);
	cTestContext.SetByte(0x0000, 0x40);
	cTestContext.SetByte(0x13000, 0x42);

	TestW65C816DirectPageRun(&cTestContext, "LDA: A.0042  X.0000  DP.0000\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816DirectPageStore(void)
{
	// STA, STX, STY and STZ to direct addresses in native mode.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	TestW65C816DirectPageSetD(&cTestContext, 0x0300, false);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x1234);
	cTestContext.SetOpcd(STA_DirectPage);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x5678);
	cTestContext.SetOpcd(STX_DirectPage);
	cTestContext.SetByte(0x12);
	cTestContext.SetOpcd(LDY_Immediate);
	cTestContext.SetShort(0x9abc);
	cTestContext.SetOpcd(STY_DirectPage);
	cTestContext.SetByte(0x14);
	cTestContext.SetOpcd(STZ_DirectPage);
	cTestContext.SetByte(0x16);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetPrint(false, false, false, false, false, false, false, false, false, false, false, false, false, true);
	TestW65C816RunInContext(&cTestContext, NULL);

	AssertShortHex((uint16)0x1234, cTestContext.GetShort(0x0310));
	AssertShortHex((uint16)0x5678, cTestContext.GetShort(0x0312));
	AssertShortHex((uint16)0x9abc, cTestContext.GetShort(0x0314));
	AssertShortHex((uint16)0x0000, cTestContext.GetShort(0x0316));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816DirectPage(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();


	TestW65C816DirectPageNative();
	TestW65C816DirectPageNativeIndexedWraps();
	TestW65C816DirectPageEmulationZeroPageWraps();
	TestW65C816DirectPageEmulationPageWraps();
	TestW65C816DirectPageEmulationLowNotZero();
	TestW65C816DirectPageEmulationDirectWraps();
	TestW65C816DirectPageEmulationIndirectLongDoesNotWrap();
	TestW65C816DirectPageStore();


	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

