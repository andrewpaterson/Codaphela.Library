#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/NaiveFile.h"
#include "BaseLib/LogToMemory.h"
#include "MicroprocessorLib/W65C816.h"
#include "MicroprocessorLib/W65C816InstructionFactory.h"
#include "MicroprocessorLib/W65C816Meta.h"
#include "TestLib/Assert.h"
#include "TestW65C816Context.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816SubtractSBCImmediateFullLog(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x10000, 0xea);
	cTestContext.SetPrint(true, true, true, true, true, true, true, true, true, true, false, false, true, false);

	cTestContext.SetByte(0xfffc, 0x00);
	cTestContext.SetByte(0xfffd, 0x02);
	cTestContext.SetByte(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x0201, 0x02);
	cTestContext.SetByte(0x0202, SEC_Implied);
	cTestContext.SetByte(0x0203, SBC_Immediate);
	cTestContext.SetByte(0x0204, 0x01);
	cTestContext.SetByte(0x0205, STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(4, uiInstructions);

	AssertString(""\
		"RES: (1)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01ff    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)\n"\
		"RES: (2)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01ff    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)\n"\
		"RES: (3)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01ff    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)\n"\
		"RES: (4)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01fe    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)\n"\
		"RES: (5)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01fd    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)\n"\
		"RES: (6)  Read(AAL)       A.0000  X.0000  Y.0000  PC.00:0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"RES: (7)  Read(AAH)       A.0000  X.0000  Y.0000  PC.00:0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0200  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDA: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0201  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"OPC: (1)  Read(Opcode)    A.0002  X.0000  Y.0000  PC.00:0202  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"SEC: (2)  IO              A.0002  X.0000  Y.0000  PC.00:0203  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"OPC: (1)  Read(Opcode)    A.0002  X.0000  Y.0000  PC.00:0203  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"SBC: (2)  Read(DL)        A.0002  X.0000  Y.0000  PC.00:0204  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"OPC: (1)  Read(Opcode)    A.0001  X.0000  Y.0000  PC.00:0205  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"STP: (2)  IO              A.0001  X.0000  Y.0000  PC.00:0206  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"STP: (3)  IO              A.0001  X.0000  Y.0000  PC.00:0206  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n", cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816SubtractSBCImmediate(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea);

	cTestContext.SetByte(0xfffc, 0x00);
	cTestContext.SetByte(0x02);

	cTestContext.SetOpcd(0x0200, SEC_Implied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(SBC_Immediate);
	cTestContext.SetByte(0x05);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816InContext(&cTestContext, ""\
		"RES: A.0000  X.0000  Y.0000  PC.00:0200    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"SEC: A.0000  X.0000  Y.0000  PC.00:0201    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"LDA: A.0001  X.0000  Y.0000  PC.00:0203    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"SBC: A.00fc  X.0000  Y.0000  PC.00:0205    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"STP: A.00fc  X.0000  Y.0000  PC.00:0206    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816SubtractInContext(CTestW65C816Context* pcContext, char* szExpected)
{
	pcContext->SetPrint(false, false, true, false, false, true, false, false, false, false, false, false, true, true);

	pcContext->SetOpcd(STP_Implied);

	TestW65C816InContext(pcContext, szExpected);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816SubtractImmediateInit(CTestW65C816Context* pcContext, uint8 uiLeft, uint8 uiRight, bool bCarry)
{
	if (bCarry)
	{
		pcContext->SetOpcd(0x0200, SEC_Implied);
	}
	else
	{
		pcContext->SetOpcd(0x0200, CLC_Implied);
	}

	pcContext->SetOpcd(LDA_Immediate);
	pcContext->SetByte(uiLeft);
	pcContext->SetOpcd(SBC_Immediate);
	pcContext->SetByte(uiRight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816SubtractBruceClark01(void)
{
	//0 - 1 = -1, returns V = 0
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	TestW65C816SubtractImmediateInit(&cTestContext, 0, 1, true);

	TestW65C816SubtractInContext(&cTestContext, ""\
		"SEC: A.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"LDA: A.0000    P(Z1 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"SBC: A.00ff    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816SubtractBruceClark02(void)
{
	//-128 - 1 = -129, returns V = 1
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	TestW65C816SubtractImmediateInit(&cTestContext, 0x80, 1, true);

	TestW65C816SubtractInContext(&cTestContext, ""\
		"SEC: A.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"LDA: A.0080    P(Z0 N1 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"SBC: A.007f    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V1 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816SubtractBruceClark03(void)
{
	//127 - -1 = 128, returns V = 1
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	TestW65C816SubtractImmediateInit(&cTestContext, 0x7f, 0xff, true);

	TestW65C816SubtractInContext(&cTestContext, ""\
		"SEC: A.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"LDA: A.007f    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"SBC: A.0080    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V1 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816SubtractBruceClark04(void)
{
	//-64 - 64 - 1 = -129, returns V = 1
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	TestW65C816SubtractImmediateInit(&cTestContext, 0xc0, 0x40, false);

	TestW65C816SubtractInContext(&cTestContext, ""\
		"CLC: A.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDA: A.00c0    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"SBC: A.007f    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V1 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816Subtract(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();


	TestW65C816SubtractSBCImmediateFullLog();
	TestW65C816SubtractSBCImmediate();

	TestW65C816SubtractBruceClark01();
	TestW65C816SubtractBruceClark02();
	TestW65C816SubtractBruceClark03();
	TestW65C816SubtractBruceClark04();


	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

