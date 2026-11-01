#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "MicroprocessorLib/W65C816.h"
#include "MicroprocessorLib/W65C816InstructionFactory.h"
#include "MicroprocessorLib/W65C816Meta.h"
#include "TestLib/Assert.h"
#include "TestW65C816Context.h"


class CTestW65C816InterruptContext : public CTestW65C816Context
{
public:
	uint32	muiIRQAddress;
	uint32	muiNMIAddress;
	bool	mbHoldIRQ;
	size	muiVectorPulls;

	void Init(uint32 uiMemorySize, uint8 uiFill, uint16 uiResetAddress)
	{
		CTestW65C816Context::Init(uiMemorySize, uiFill, uiResetAddress);
		muiIRQAddress = 0xffffffff;
		muiNMIAddress = 0xffffffff;
		mbHoldIRQ = false;
		muiVectorPulls = 0;
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816InterruptTickHigh(CW65C816Meta* pcMPU, void* pvContext)
{
	// Pulls IRQB or NMIB low when the opcode at the given address is fetched and
	// lets it go when the interrupt vector is read (VPB low), as interrupt
	// acknowledge hardware would.  Vector pulls are counted, the reset vector
	// pull included.
	CTestW65C816InterruptContext*	pcContext;
	uint32							uiAddress;
	bool							bOpcodeFetch;

	pcContext = (CTestW65C816InterruptContext*)pvContext;

	uiAddress = pcMPU->GetAddress()->Get();
	uiAddress |= pcMPU->GetBank() << 16;
	bOpcodeFetch = pcMPU->GetVDA()->IsHigh() && pcMPU->GetVPA()->IsHigh();

	if (bOpcodeFetch && (uiAddress == pcContext->muiIRQAddress))
	{
		pcMPU->GetIRQB()->Set(false);
	}
	if (bOpcodeFetch && (uiAddress == pcContext->muiNMIAddress))
	{
		pcMPU->GetNMIB()->Set(false);
	}

	if (pcMPU->GetVPB()->IsLow())
	{
		if (pcMPU->GetAddress()->Get() & 1)
		{
			pcContext->muiVectorPulls++;
		}
		if (!pcContext->mbHoldIRQ)
		{
			pcMPU->GetIRQB()->Set(true);
		}
		pcMPU->GetNMIB()->Set(true);
	}

	TestW65C81ContextTickHigh(pcMPU, pvContext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816InterruptRunInContext(CTestW65C816InterruptContext* pcContext, char* szExpected)
{
	CW65C816Meta	cMPU;
	uint64			uiInstructions;

	CW65C816InstructionFactory::GetInstance()->Init();

	pcContext->SetPrint(false, false, true, false, false, true, false, false, true, true, false, false, true, true);
	pcContext->muiMaxInstructions = 10000;

	cMPU.Init(TestW65C816InterruptTickHigh, TestW65C81ContextTickLow, pcContext);
	if (pcContext->mbHoldIRQ)
	{
		cMPU.GetIRQB()->Set(false);
	}

	uiInstructions = pcContext->Run(&cMPU);

	cMPU.Kill();

	AssertLong(pcContext->muiInstructions, uiInstructions);
	AssertString(szExpected, pcContext->SequenceText());

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816InterruptBRKEmulation(void)
{
	// Emulation mode BRK: PBR is not pushed (7.11.2), the pushed status has B set,
	// the return address skips the signature byte (7.22), I is set and D is
	// cleared (7.12) and the vector is 00FFFE (table 5-2).
	CTestW65C816InterruptContext	cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, SED_Implied);
	cTestContext.SetOpcd(CLI_Implied);
	cTestContext.SetOpcd(BRK_Interrupt);
	cTestContext.SetByte(0x55);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0xfffe, 0x0300);
	cTestContext.SetOpcd(0x0300, RTI_StackImplied);

	TestW65C816InterruptRunInContext(&cTestContext, ""\
		"SED: A.0000  PC.00:0201  S.01fc    P(Z0 N0 D1 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"CLI: A.0000  PC.00:0202  S.01fc    P(Z0 N0 D1 I0 M.8 X.8 C0 E1 V0 B0)\n"\
		"BRK: A.0000  PC.00:0300  S.01f9    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B1)\n"\
		"RTI: A.0000  PC.00:0204  S.01fc    P(Z0 N0 D1 I0 M.8 X.8 C0 E1 V0 B1)\n");

	AssertShortHex((uint16)0x0204, cTestContext.GetShort(0x01fb));
	AssertByteHex((uint8)0x38, cTestContext.GetByte(0x01fa));
	AssertByteHex((uint8)0xea, cTestContext.GetByte(0x01f9));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816InterruptBRKNative(void)
{
	// Native mode BRK: PBR is pushed (7.11.1), there is no B flag, the vector is
	// 00FFE6 (table 5-3).  RTI pulls PBR as well.
	CTestW65C816InterruptContext	cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(SED_Implied);
	cTestContext.SetOpcd(CLI_Implied);
	cTestContext.SetOpcd(BRK_Interrupt);
	cTestContext.SetByte(0x55);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0xfffe, 0x0500);
	cTestContext.SetShort(0xffe6, 0x0300);
	cTestContext.SetOpcd(0x0300, RTI_StackImplied);

	TestW65C816InterruptRunInContext(&cTestContext, ""\
		"CLC: A.0000  PC.00:0201  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  PC.00:0202  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"SED: A.0000  PC.00:0203  S.01fc    P(Z0 N0 D1 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"CLI: A.0000  PC.00:0204  S.01fc    P(Z0 N0 D1 I0 M.8 X.8 C1 E0 V0 ..)\n"\
		"BRK: A.0000  PC.00:0300  S.01f8    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"RTI: A.0000  PC.00:0206  S.01fc    P(Z0 N0 D1 I0 M.8 X.8 C1 E0 V0 ..)\n");

	AssertByteHex((uint8)0x00, cTestContext.GetByte(0x01fc));
	AssertShortHex((uint16)0x0206, cTestContext.GetShort(0x01fa));
	AssertByteHex((uint8)0x39, cTestContext.GetByte(0x01f9));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816InterruptBRKNativeFromBank(void)
{
	// Datasheet 7.11.1: in native mode the program bank is pushed and cleared, RTI
	// pulls it back.
	CTestW65C816InterruptContext	cTestContext;

	cTestContext.Init(0x20000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(JML_AbsoluteLong);
	cTestContext.SetShort(0x8000);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(0x18000, BRK_Interrupt);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0xffe6, 0x0300);
	cTestContext.SetOpcd(0x0300, RTI_StackImplied);

	TestW65C816InterruptRunInContext(&cTestContext, ""\
		"CLC: A.0000  PC.00:0201  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  PC.00:0202  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"JMP: A.0000  PC.01:8000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"BRK: A.0000  PC.00:0300  S.01f8    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"RTI: A.0000  PC.01:8002  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n");

	AssertByteHex((uint8)0x01, cTestContext.GetByte(0x01fc));
	AssertShortHex((uint16)0x8002, cTestContext.GetShort(0x01fa));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816InterruptBRKEmulationFromBank(void)
{
	// Datasheet 7.11.2 and 7.11.3: in emulation mode the program bank is cleared but
	// not pushed, so RTI returns into bank 0.
	CTestW65C816InterruptContext	cTestContext;

	cTestContext.Init(0x20000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, JML_AbsoluteLong);
	cTestContext.SetShort(0x8000);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(0x18000, BRK_Interrupt);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(0x8002, STP_Implied);

	cTestContext.SetShort(0xfffe, 0x0300);
	cTestContext.SetOpcd(0x0300, RTI_StackImplied);

	TestW65C816InterruptRunInContext(&cTestContext, ""\
		"JMP: A.0000  PC.01:8000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"BRK: A.0000  PC.00:0300  S.01f9    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B1)\n"\
		"RTI: A.0000  PC.00:8002  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B1)\n");

	AssertShortHex((uint16)0x8002, cTestContext.GetShort(0x01fb));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816InterruptCOPEmulation(void)
{
	// COP uses vector 00FFF4 in emulation mode.
	CTestW65C816InterruptContext	cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, COP_Interrupt);
	cTestContext.SetByte(0x12);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0xfffe, 0x0500);
	cTestContext.SetShort(0xfff4, 0x0300);
	cTestContext.SetOpcd(0x0300, RTI_StackImplied);

	TestW65C816InterruptRunInContext(&cTestContext, ""\
		"COP: A.0000  PC.00:0300  S.01f9    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"RTI: A.0000  PC.00:0202  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");

	AssertShortHex((uint16)0x0202, cTestContext.GetShort(0x01fb));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816InterruptCOPNative(void)
{
	// COP uses vector 00FFE4 in native mode.
	CTestW65C816InterruptContext	cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(COP_Interrupt);
	cTestContext.SetByte(0x12);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0xfff4, 0x0500);
	cTestContext.SetShort(0xffe4, 0x0300);
	cTestContext.SetOpcd(0x0300, RTI_StackImplied);

	TestW65C816InterruptRunInContext(&cTestContext, ""\
		"CLC: A.0000  PC.00:0201  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  PC.00:0202  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"COP: A.0000  PC.00:0300  S.01f8    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"RTI: A.0000  PC.00:0204  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n");

	AssertByteHex((uint8)0x00, cTestContext.GetByte(0x01fc));
	AssertShortHex((uint16)0x0204, cTestContext.GetShort(0x01fa));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816InterruptRTINativeRestoresWidths(void)
{
	// RTI pulls the status register so M and X come back from the stack.
	CTestW65C816InterruptContext	cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(BRK_Interrupt);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0xffe6, 0x0300);
	cTestContext.SetOpcd(0x0300, REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(RTI_StackImplied);

	TestW65C816InterruptRunInContext(&cTestContext, ""\
		"CLC: A.0000  PC.00:0201  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  PC.00:0202  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"BRK: A.0000  PC.00:0300  S.01f8    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: A.0000  PC.00:0302  S.01f8    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"RTI: A.0000  PC.00:0204  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n");

	AssertByteHex((uint8)0x35, cTestContext.GetByte(0x01f9));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816InterruptIRQEmulation(void)
{
	// A hardware IRQ pushes the address of the next instruction and a status with
	// B clear, then uses vector 00FFFE.  VPB is low for the two vector reads.
	CTestW65C816InterruptContext	cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.muiIRQAddress = 0x0202;

	cTestContext.SetOpcd(0x0200, CLI_Implied);
	cTestContext.SetOpcd(SED_Implied);
	cTestContext.SetOpcd(NOP_Implied);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0xfffe, 0x0300);
	cTestContext.SetOpcd(0x0300, RTI_StackImplied);
	cTestContext.muiInstructions++;  // The interrupt.  It has no opcode fetch so the trace shows it on the NOP line.

	TestW65C816InterruptRunInContext(&cTestContext, ""\
		"CLI: A.0000  PC.00:0201  S.01fc    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)\n"\
		"SED: A.0000  PC.00:0202  S.01fc    P(Z0 N0 D1 I0 M.8 X.8 C0 E1 V0 B0)\n"\
		"NOP: A.0000  PC.00:0300  S.01f9    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"RTI: A.0000  PC.00:0203  S.01fc    P(Z0 N0 D1 I0 M.8 X.8 C0 E1 V0 B0)\n");

	AssertShortHex((uint16)0x0203, cTestContext.GetShort(0x01fb));
	AssertByteHex((uint8)0x28, cTestContext.GetByte(0x01fa));
	AssertSize(2, cTestContext.muiVectorPulls);

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816InterruptIRQNative(void)
{
	// Native mode IRQ pushes PBR and uses vector 00FFEE.
	CTestW65C816InterruptContext	cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.muiIRQAddress = 0x0203;

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(CLI_Implied);
	cTestContext.SetOpcd(NOP_Implied);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0xfffe, 0x0500);
	cTestContext.SetShort(0xffee, 0x0300);
	cTestContext.SetOpcd(0x0300, RTI_StackImplied);
	cTestContext.muiInstructions++;  // The interrupt.  It has no opcode fetch so the trace shows it on the NOP line.

	TestW65C816InterruptRunInContext(&cTestContext, ""\
		"CLC: A.0000  PC.00:0201  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  PC.00:0202  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"CLI: A.0000  PC.00:0203  S.01fc    P(Z0 N0 D0 I0 M.8 X.8 C1 E0 V0 ..)\n"\
		"NOP: A.0000  PC.00:0300  S.01f8    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"RTI: A.0000  PC.00:0204  S.01fc    P(Z0 N0 D0 I0 M.8 X.8 C1 E0 V0 ..)\n");

	AssertByteHex((uint8)0x00, cTestContext.GetByte(0x01fc));
	AssertShortHex((uint16)0x0204, cTestContext.GetShort(0x01fa));
	AssertByteHex((uint8)0x31, cTestContext.GetByte(0x01f9));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816InterruptIRQMasked(void)
{
	// IRQB held low is ignored while I is set.
	CTestW65C816InterruptContext	cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.mbHoldIRQ = true;

	cTestContext.SetOpcd(0x0200, SEI_Implied);
	cTestContext.SetOpcd(NOP_Implied);
	cTestContext.SetOpcd(NOP_Implied);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0xfffe, 0x0300);
	cTestContext.SetOpcd(0x0300, STP_Implied);
	cTestContext.muiInstructions--;

	TestW65C816InterruptRunInContext(&cTestContext, ""\
		"SEI: A.0000  PC.00:0201  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"NOP: A.0000  PC.00:0202  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"NOP: A.0000  PC.00:0203  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");

	AssertSize(1, cTestContext.muiVectorPulls);

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816InterruptNMIEmulation(void)
{
	// NMI is taken even when I is set and uses vector 00FFFA.
	CTestW65C816InterruptContext	cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.muiNMIAddress = 0x0201;

	cTestContext.SetOpcd(0x0200, SEI_Implied);
	cTestContext.SetOpcd(NOP_Implied);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0xfffe, 0x0500);
	cTestContext.SetShort(0xfffa, 0x0300);
	cTestContext.SetOpcd(0x0300, RTI_StackImplied);
	cTestContext.muiInstructions++;  // The interrupt.  It has no opcode fetch so the trace shows it on the NOP line.

	TestW65C816InterruptRunInContext(&cTestContext, ""\
		"SEI: A.0000  PC.00:0201  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"NOP: A.0000  PC.00:0300  S.01f9    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"RTI: A.0000  PC.00:0202  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");

	AssertShortHex((uint16)0x0202, cTestContext.GetShort(0x01fb));
	AssertSize(2, cTestContext.muiVectorPulls);

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816InterruptNMINative(void)
{
	// Native mode NMI uses vector 00FFEA.
	CTestW65C816InterruptContext	cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.muiNMIAddress = 0x0202;

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(NOP_Implied);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0xfffa, 0x0500);
	cTestContext.SetShort(0xffea, 0x0300);
	cTestContext.SetOpcd(0x0300, RTI_StackImplied);
	cTestContext.muiInstructions++;  // The interrupt.  It has no opcode fetch so the trace shows it on the NOP line.

	TestW65C816InterruptRunInContext(&cTestContext, ""\
		"CLC: A.0000  PC.00:0201  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: A.0000  PC.00:0202  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"NOP: A.0000  PC.00:0300  S.01f8    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"RTI: A.0000  PC.00:0203  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n");

	AssertShortHex((uint16)0x0203, cTestContext.GetShort(0x01fa));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816Interrupt(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();


	TestW65C816InterruptBRKEmulation();
	TestW65C816InterruptBRKNative();
	TestW65C816InterruptBRKNativeFromBank();
	TestW65C816InterruptBRKEmulationFromBank();
	TestW65C816InterruptCOPEmulation();
	TestW65C816InterruptCOPNative();
	TestW65C816InterruptRTINativeRestoresWidths();
	TestW65C816InterruptIRQEmulation();
	TestW65C816InterruptIRQNative();
	TestW65C816InterruptIRQMasked();
	TestW65C816InterruptNMIEmulation();
	TestW65C816InterruptNMINative();


	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

