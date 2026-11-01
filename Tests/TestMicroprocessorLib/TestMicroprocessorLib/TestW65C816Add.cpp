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
void TestW65C816AddADCImmediateFullLog(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x10000, 0xea);
	cTestContext.SetPrint(true, true, true, true, true, true, true, true, true, true, false, false, false, false);

	cTestContext.SetByte(0xfffc, 0x00);
	cTestContext.SetByte(0xfffd, 0x02);
	cTestContext.SetByte(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x0201, 0x01);
	cTestContext.SetByte(0x0202, ADC_Immediate);
	cTestContext.SetByte(0x0203, 0x02);
	cTestContext.SetByte(0x0204, STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(3, uiInstructions);

	AssertString(""\
		"RES: (1)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01ff\n"\
		"RES: (2)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01ff\n"\
		"RES: (3)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01ff\n"\
		"RES: (4)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01fe\n"\
		"RES: (5)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01fd\n"\
		"RES: (6)  Read(AAL)       A.0000  X.0000  Y.0000  PC.00:0000  S.01fc\n"\
		"RES: (7)  Read(AAH)       A.0000  X.0000  Y.0000  PC.00:0000  S.01fc\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0200  S.01fc\n"\
		"LDA: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0201  S.01fc\n"\
		"OPC: (1)  Read(Opcode)    A.0001  X.0000  Y.0000  PC.00:0202  S.01fc\n"\
		"ADC: (2)  Read(DL)        A.0001  X.0000  Y.0000  PC.00:0203  S.01fc\n"\
		"OPC: (1)  Read(Opcode)    A.0003  X.0000  Y.0000  PC.00:0204  S.01fc\n"\
		"STP: (2)  IO              A.0003  X.0000  Y.0000  PC.00:0205  S.01fc\n"\
		"STP: (3)  IO              A.0003  X.0000  Y.0000  PC.00:0205  S.01fc\n" , cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816AddADCImmediate(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea);

	cTestContext.SetByte(0xfffc, 0x00);
	cTestContext.SetByte(0x02);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(ADC_Immediate);
	cTestContext.SetByte(0x05);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816InContext(&cTestContext, ""\
		"RES: A.0000  X.0000  Y.0000  PC.00:0200    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDA: A.0001  X.0000  Y.0000  PC.00:0202    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"ADC: A.0006  X.0000  Y.0000  PC.00:0204    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"STP: A.0006  X.0000  Y.0000  PC.00:0205    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816AddADCImmediateOverflow(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea);

	cTestContext.SetByte(0xfffc, 0x00);
	cTestContext.SetByte(0x02);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x80);
	cTestContext.SetOpcd(ADC_Immediate);
	cTestContext.SetByte(0x80);
	cTestContext.SetOpcd(0x0204, STP_Implied);

	TestW65C816InContext(&cTestContext, ""\
		"RES: A.0000  X.0000  Y.0000  PC.00:0200    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDA: A.0080  X.0000  Y.0000  PC.00:0202    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"ADC: A.0000  X.0000  Y.0000  PC.00:0204    P(Z1 N0 D0 I1 M.8 X.8 C1 E1 V1 B0)\n"\
		"STP: A.0000  X.0000  Y.0000  PC.00:0205    P(Z1 N0 D0 I1 M.8 X.8 C1 E1 V1 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816AddADCAbsoluteLongWithCarryPositive(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea);

	cTestContext.SetByte(0xfffc, 0x00);
	cTestContext.SetByte(0x02);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x7c);
	cTestContext.SetOpcd(SEC_Implied);
	cTestContext.SetOpcd(ADC_AbsoluteLong);
	cTestContext.SetByte(0x05);
	cTestContext.SetByte(0x30);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetByte(0x3005, 0x02);

	TestW65C816InContext(&cTestContext, ""\
		"RES: A.0000  X.0000  Y.0000  PC.00:0200    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDA: A.007c  X.0000  Y.0000  PC.00:0202    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"SEC: A.007c  X.0000  Y.0000  PC.00:0203    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"ADC: A.007f  X.0000  Y.0000  PC.00:0207    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"STP: A.007f  X.0000  Y.0000  PC.00:0208    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816AddADCIndexedIndirectWithXWithCarryPositive(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea);

	cTestContext.SetByte(0xfffc, 0x00);
	cTestContext.SetByte(0x02);

	cTestContext.SetByte(0x0050, 0x05);
	cTestContext.SetByte(0x30);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0xfe);
	cTestContext.SetOpcd(SEC_Implied);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(ADC_DirectPageIndexedIndirectWithX);
	cTestContext.SetByte(0x40);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetByte(0x3005, 0x03);

	TestW65C816InContext(&cTestContext, ""\
		"RES: A.0000  X.0000  Y.0000  PC.00:0200    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDA: A.00fe  X.0000  Y.0000  PC.00:0202    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"SEC: A.00fe  X.0000  Y.0000  PC.00:0203    P(Z0 N1 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"LDX: A.00fe  X.0010  Y.0000  PC.00:0205    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"ADC: A.0002  X.0010  Y.0000  PC.00:0207    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"STP: A.0002  X.0010  Y.0000  PC.00:0208    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816AddADCAbsoluteLongWithCarryNegative(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea);

	cTestContext.SetByte(0xfffc, 0x00);
	cTestContext.SetByte(0x02);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x7d);
	cTestContext.SetOpcd(SEC_Implied);
	cTestContext.SetOpcd(ADC_AbsoluteLong);
	cTestContext.SetByte(0x05);
	cTestContext.SetByte(0x30);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetByte(0x3005, 0x02);

	TestW65C816InContext(&cTestContext, ""\
		"RES: A.0000  X.0000  Y.0000  PC.00:0200    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDA: A.007d  X.0000  Y.0000  PC.00:0202    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"SEC: A.007d  X.0000  Y.0000  PC.00:0203    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"ADC: A.0080  X.0000  Y.0000  PC.00:0207    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V1 B0)\n"\
		"STP: A.0080  X.0000  Y.0000  PC.00:0208    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V1 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816AddInContext(CTestW65C816Context* pcContext, char* szExpected)
{
	pcContext->SetPrint(false, false, true, false, false, true, false, false, false, false, false, false, true, true);

	pcContext->SetOpcd(STP_Implied);

	TestW65C816InContext(pcContext, szExpected);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816AddImmediateInit(CTestW65C816Context* pcContext, uint8 uiLeft, uint8 uiRight, bool bCarry)
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
	pcContext->SetOpcd(ADC_Immediate);
	pcContext->SetByte(uiRight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816AddBruceClark01(void)
{
	// 1 + 1 = 2, returns C = 0, V = 0
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	TestW65C816AddImmediateInit(&cTestContext, 1, 1, false);

	TestW65C816AddInContext(&cTestContext, ""\
		"CLC: A.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDA: A.0001    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"ADC: A.0002    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816AddBruceClark02(void)
{
	//1 + -1 = 0, returns C = 1, V = 0
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	TestW65C816AddImmediateInit(&cTestContext, 1, 0xff, false);

	TestW65C816AddInContext(&cTestContext, ""\
		"CLC: A.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDA: A.0001    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"ADC: A.0000    P(Z1 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816AddBruceClark03(void)
{
	//127 + 1 = 128, returns C = 0, V = 1
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	TestW65C816AddImmediateInit(&cTestContext, 0x7f, 1, false);

	TestW65C816AddInContext(&cTestContext, ""\
		"CLC: A.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDA: A.007f    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"ADC: A.0080    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V1 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816AddBruceClark04(void)
{
	//-128 + -1 = -129, returns C = 1, V = 1
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	TestW65C816AddImmediateInit(&cTestContext, 0x80, 0xff, false);

	TestW65C816AddInContext(&cTestContext, ""\
		"CLC: A.0000    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"LDA: A.0080    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"ADC: A.007f    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V1 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816AddBruceClark05(void)
{
	//63 + 64 + 1 = 128, returns C = 0, V = 1
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);
	TestW65C816AddImmediateInit(&cTestContext, 0x3f, 0x40, true);

	TestW65C816AddInContext(&cTestContext, ""\
		"SEC: A.0000    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"LDA: A.003f    P(Z0 N0 D0 I1 M.8 X.8 C1 E1 V0 B0)\n"\
		"ADC: A.0080    P(Z0 N1 D0 I1 M.8 X.8 C0 E1 V1 B0)\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816Add(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();


	TestW65C816AddADCImmediateFullLog();
	TestW65C816AddADCImmediate();
	TestW65C816AddADCImmediateOverflow();
	TestW65C816AddADCAbsoluteLongWithCarryPositive();
	TestW65C816AddADCIndexedIndirectWithXWithCarryPositive();
	TestW65C816AddADCAbsoluteLongWithCarryNegative();

	TestW65C816AddBruceClark01();
	TestW65C816AddBruceClark02();
	TestW65C816AddBruceClark03();
	TestW65C816AddBruceClark04();
	TestW65C816AddBruceClark05();


	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

