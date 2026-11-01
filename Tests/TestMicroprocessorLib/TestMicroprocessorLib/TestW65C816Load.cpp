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
void TestW65C816LoadLDAImmediate(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.SetPrint(true, true, true, true, true, true, true, true, true, true, false, false, true, false);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetByte(0xef);
	cTestContext.SetByte(0xcd);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetByte(0x00);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(6, uiInstructions);
	AssertShortHex((uint16)0xcdef, cTestContext.GetShort(0x0100));

	AssertString(""\
		"RES: (1)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01ff    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)\n"\
		"RES: (2)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01ff    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)\n"\
		"RES: (3)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01ff    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)\n"\
		"RES: (4)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01fe    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)\n"\
		"RES: (5)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01fd    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)\n"\
		"RES: (6)  Read(AAL)       A.0000  X.0000  Y.0000  PC.00:0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"RES: (7)  Read(AAH)       A.0000  X.0000  Y.0000  PC.00:0000  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0200  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"CLC: (2)  IO              A.0000  X.0000  Y.0000  PC.00:0201  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0201  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"XCE: (2)  IO              A.0000  X.0000  Y.0000  PC.00:0202  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0202  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0203  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"REP: (3)  IO              A.0000  X.0000  Y.0000  PC.00:0203  S.01fc    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0204  S.01fc    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDA: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0205  S.01fc    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDA: (3)  Read(DH)        A.0000  X.0000  Y.0000  PC.00:0206  S.01fc    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.0000  Y.0000  PC.00:0207  S.01fc    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"STA: (2)  Read(AAL)       A.cdef  X.0000  Y.0000  PC.00:0208  S.01fc    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"STA: (3)  Read(AAH)       A.cdef  X.0000  Y.0000  PC.00:0209  S.01fc    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"STA: (4)  Write(DL)       A.cdef  X.0000  Y.0000  PC.00:020a  S.01fc    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"STA: (5)  Write(DH)       A.cdef  X.0000  Y.0000  PC.00:020a  S.01fc    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.0000  Y.0000  PC.00:020a  S.01fc    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"STP: (2)  IO              A.cdef  X.0000  Y.0000  PC.00:020b  S.01fc    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"STP: (3)  IO              A.cdef  X.0000  Y.0000  PC.00:020b  S.01fc    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n", cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LoadLDAAbsolute(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, true, true, true, true, true, true, true, false, false, true, false);

	cTestContext.SetShort(0x3003, 0xcdef);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDA_Absolute);
	cTestContext.SetByte(0x03);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetByte(0x00);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(6, uiInstructions);
	AssertShortHex((uint16)0xcdef, cTestContext.GetShort(0x0100));

	cTestContext.StripToInstruction("LDA");
	AssertString(""\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0204  S.01fc    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDA: (2)  Read(AAL)       A.0000  X.0000  Y.0000  PC.00:0205  S.01fc    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDA: (3)  Read(AAH)       A.0000  X.0000  Y.0000  PC.00:0206  S.01fc    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDA: (4)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0207  S.01fc    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"LDA: (5)  Read(DH)        A.0000  X.0000  Y.0000  PC.00:0207  S.01fc    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.0000  Y.0000  PC.00:0207  S.01fc    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"STA: (2)  Read(AAL)       A.cdef  X.0000  Y.0000  PC.00:0208  S.01fc    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"STA: (3)  Read(AAH)       A.cdef  X.0000  Y.0000  PC.00:0209  S.01fc    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"STA: (4)  Write(DL)       A.cdef  X.0000  Y.0000  PC.00:020a  S.01fc    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"STA: (5)  Write(DH)       A.cdef  X.0000  Y.0000  PC.00:020a  S.01fc    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.0000  Y.0000  PC.00:020a  S.01fc    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)\n", cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LoadLDAAbsoluteIndexedWithY(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x20000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, true, true, true, true, true, true, false, false, false, false, false);

	cTestContext.SetShort(0x1FFFE, 0xcdef);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDY_Immediate);
	cTestContext.SetShort(0xffff);
	cTestContext.SetOpcd(LDA_AbsoluteIndexedWithY);
	cTestContext.SetShort(0xffff);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetShort(0x0100);
	cTestContext.SetOpcd(STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(7, uiInstructions);
	AssertShortHex((uint16)0xcdef, cTestContext.GetShort(0x0100));

	cTestContext.StripToInstruction("LDY");
	AssertString(""\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0204\n"\
		"LDY: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0205\n"\
		"LDY: (3)  Read(DH)        A.0000  X.0000  Y.0000  PC.00:0206\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.ffff  PC.00:0207\n"\
		"LDA: (2)  Read(AAL)       A.0000  X.0000  Y.ffff  PC.00:0208\n"\
		"LDA: (3)  Read(AAH)       A.0000  X.0000  Y.ffff  PC.00:0209\n"\
		"LDA: (4)  IO              A.0000  X.0000  Y.ffff  PC.00:020a\n"\
		"LDA: (5)  Read(DL)        A.0000  X.0000  Y.ffff  PC.00:020a\n"\
		"LDA: (6)  Read(DH)        A.0000  X.0000  Y.ffff  PC.00:020a\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.0000  Y.ffff  PC.00:020a\n"\
		"STA: (2)  Read(AAL)       A.cdef  X.0000  Y.ffff  PC.00:020b\n"\
		"STA: (3)  Read(AAH)       A.cdef  X.0000  Y.ffff  PC.00:020c\n"\
		"STA: (4)  Write(DL)       A.cdef  X.0000  Y.ffff  PC.00:020d\n"\
		"STA: (5)  Write(DH)       A.cdef  X.0000  Y.ffff  PC.00:020d\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.0000  Y.ffff  PC.00:020d\n", cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LoadLDAAbsoluteIndexedWithX(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x20000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, true, true, true, true, true, true, true, false, false, false, false);
	cTestContext.SetSignalPrint(true, true, true, false, false, false, false, false, false, false, false, false);

	cTestContext.SetShort(0x1FFFE, 0xcdef);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0xffff);
	cTestContext.SetOpcd(LDA_AbsoluteIndexedWithX);
	cTestContext.SetShort(0xffff);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetShort(0x0100);
	cTestContext.SetOpcd(STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(7, uiInstructions);
	AssertShortHex((uint16)0xcdef, cTestContext.GetShort(0x0100));

	cTestContext.StripToInstruction("LDX");
	AssertString(""\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0204  S.01fc    Addr.00:0204  Data.a2\n"\
		"LDX: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0205  S.01fc    Addr.00:0205  Data.ff\n"\
		"LDX: (3)  Read(DH)        A.0000  X.0000  Y.0000  PC.00:0206  S.01fc    Addr.00:0206  Data.ff\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.ffff  Y.0000  PC.00:0207  S.01fc    Addr.00:0207  Data.bd\n"\
		"LDA: (2)  Read(AAL)       A.0000  X.ffff  Y.0000  PC.00:0208  S.01fc    Addr.00:0208  Data.ff\n"\
		"LDA: (3)  Read(AAH)       A.0000  X.ffff  Y.0000  PC.00:0209  S.01fc    Addr.00:0209  Data.ff\n"\
		"LDA: (4)  IO              A.0000  X.ffff  Y.0000  PC.00:020a  S.01fc    Addr.00:fffe  Data.00\n"\
		"LDA: (5)  Read(DL)        A.0000  X.ffff  Y.0000  PC.00:020a  S.01fc    Addr.01:fffe  Data.ef\n"\
		"LDA: (6)  Read(DH)        A.0000  X.ffff  Y.0000  PC.00:020a  S.01fc    Addr.01:ffff  Data.cd\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.ffff  Y.0000  PC.00:020a  S.01fc    Addr.00:020a  Data.8d\n"\
		"STA: (2)  Read(AAL)       A.cdef  X.ffff  Y.0000  PC.00:020b  S.01fc    Addr.00:020b  Data.00\n"\
		"STA: (3)  Read(AAH)       A.cdef  X.ffff  Y.0000  PC.00:020c  S.01fc    Addr.00:020c  Data.01\n"\
		"STA: (4)  Write(DL)       A.cdef  X.ffff  Y.0000  PC.00:020d  S.01fc    Addr.00:0100  Data.ef\n"\
		"STA: (5)  Write(DH)       A.cdef  X.ffff  Y.0000  PC.00:020d  S.01fc    Addr.00:0101  Data.cd\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.ffff  Y.0000  PC.00:020d  S.01fc    Addr.00:020d  Data.db\n", cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LoadLDAAbsoluteLongIndexedWithX(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x30000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, true, true, true, true, true, true, true, false, false, false, false);
	cTestContext.SetSignalPrint(true, true, true, false, false, false, false, false, false, false, false, false);

	cTestContext.SetShort(0x2FFFE, 0xcdef);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0xffff);
	cTestContext.SetOpcd(LDA_AbsoluteLongIndexedWithX);
	cTestContext.SetShort(0xffff);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetShort(0x0100);
	cTestContext.SetOpcd(STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(7, uiInstructions);
	AssertShortHex((uint16)0xcdef, cTestContext.GetShort(0x0100));

	cTestContext.StripToInstruction("LDX");
	AssertString(""\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0204  S.01fc    Addr.00:0204  Data.a2\n"\
		"LDX: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0205  S.01fc    Addr.00:0205  Data.ff\n"\
		"LDX: (3)  Read(DH)        A.0000  X.0000  Y.0000  PC.00:0206  S.01fc    Addr.00:0206  Data.ff\n"\

		"OPC: (1)  Read(Opcode)    A.0000  X.ffff  Y.0000  PC.00:0207  S.01fc    Addr.00:0207  Data.bf\n"\
		"LDA: (2)  Read(AAL)       A.0000  X.ffff  Y.0000  PC.00:0208  S.01fc    Addr.00:0208  Data.ff\n"\
		"LDA: (3)  Read(AAH)       A.0000  X.ffff  Y.0000  PC.00:0209  S.01fc    Addr.00:0209  Data.ff\n"\
		"LDA: (4)  Read(AAB)       A.0000  X.ffff  Y.0000  PC.00:020a  S.01fc    Addr.00:020a  Data.01\n"\
		"LDA: (5)  Read(DL)        A.0000  X.ffff  Y.0000  PC.00:020b  S.01fc    Addr.02:fffe  Data.ef\n"\
		"LDA: (6)  Read(DH)        A.0000  X.ffff  Y.0000  PC.00:020b  S.01fc    Addr.02:ffff  Data.cd\n"\

		"OPC: (1)  Read(Opcode)    A.cdef  X.ffff  Y.0000  PC.00:020b  S.01fc    Addr.00:020b  Data.8d\n"\
		"STA: (2)  Read(AAL)       A.cdef  X.ffff  Y.0000  PC.00:020c  S.01fc    Addr.00:020c  Data.00\n"\
		"STA: (3)  Read(AAH)       A.cdef  X.ffff  Y.0000  PC.00:020d  S.01fc    Addr.00:020d  Data.01\n"\
		"STA: (4)  Write(DL)       A.cdef  X.ffff  Y.0000  PC.00:020e  S.01fc    Addr.00:0100  Data.ef\n"\
		"STA: (5)  Write(DH)       A.cdef  X.ffff  Y.0000  PC.00:020e  S.01fc    Addr.00:0101  Data.cd\n"\

		"OPC: (1)  Read(Opcode)    A.cdef  X.ffff  Y.0000  PC.00:020e  S.01fc    Addr.00:020e  Data.db\n", cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LoadLDAAbsoluteLong(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x20000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, true, true, true, true, true, true, false, false, false, false, false);

	cTestContext.SetShort(0x013003, 0xcdef);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDA_AbsoluteLong);
	cTestContext.SetByte(0x03);
	cTestContext.SetByte(0x30);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetByte(0x00);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(6, uiInstructions);
	AssertShortHex((uint16)0xcdef, cTestContext.GetShort(0x0100));

	cTestContext.StripToInstruction("LDA");
	AssertString(""\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0204\n"\
		"LDA: (2)  Read(AAL)       A.0000  X.0000  Y.0000  PC.00:0205\n"\
		"LDA: (3)  Read(AAH)       A.0000  X.0000  Y.0000  PC.00:0206\n"\
		"LDA: (4)  Read(AAB)       A.0000  X.0000  Y.0000  PC.00:0207\n"\
		"LDA: (5)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0208\n"\
		"LDA: (6)  Read(DH)        A.0000  X.0000  Y.0000  PC.00:0208\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.0000  Y.0000  PC.00:0208\n"\
		"STA: (2)  Read(AAL)       A.cdef  X.0000  Y.0000  PC.00:0209\n"\
		"STA: (3)  Read(AAH)       A.cdef  X.0000  Y.0000  PC.00:020a\n"\
		"STA: (4)  Write(DL)       A.cdef  X.0000  Y.0000  PC.00:020b\n"\
		"STA: (5)  Write(DH)       A.cdef  X.0000  Y.0000  PC.00:020b\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.0000  Y.0000  PC.00:020b\n", cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LoadLDAStackRelative(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, true, true, true, true, true, true, true, false, false, false, false);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x1ff);
	cTestContext.SetOpcd(TCS_Implied);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0xcdef);
	cTestContext.SetOpcd(PHX_StackImplied);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x6789);
	cTestContext.SetOpcd(PHX_StackImplied);
	cTestContext.SetOpcd(LDA_StackRelative);
	cTestContext.SetByte(3);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetShort(0x0100);
	cTestContext.SetOpcd(STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(12, uiInstructions);
	AssertShortHex((uint16)0xcdef, cTestContext.GetShort(0x0100));

	cTestContext.StripToInstruction("LDA");
	AssertString(""\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0204  S.01fc\n"\
		"LDA: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0205  S.01fc\n"\
		"LDA: (3)  Read(DH)        A.0000  X.0000  Y.0000  PC.00:0206  S.01fc\n"\
		"OPC: (1)  Read(Opcode)    A.01ff  X.0000  Y.0000  PC.00:0207  S.01fc\n"\
		"TCS: (2)  IO              A.01ff  X.0000  Y.0000  PC.00:0208  S.01fc\n"\
		"OPC: (1)  Read(Opcode)    A.01ff  X.0000  Y.0000  PC.00:0208  S.01ff\n"\
		"LDX: (2)  Read(DL)        A.01ff  X.0000  Y.0000  PC.00:0209  S.01ff\n"\
		"LDX: (3)  Read(DH)        A.01ff  X.0000  Y.0000  PC.00:020a  S.01ff\n"\
		"OPC: (1)  Read(Opcode)    A.01ff  X.cdef  Y.0000  PC.00:020b  S.01ff\n"\
		"PHX: (2)  IO              A.01ff  X.cdef  Y.0000  PC.00:020c  S.01ff\n"\
		"PHX: (3)  Write(DH)       A.01ff  X.cdef  Y.0000  PC.00:020c  S.01ff\n"\
		"PHX: (4)  Write(DL)       A.01ff  X.cdef  Y.0000  PC.00:020c  S.01fe\n"\
		"OPC: (1)  Read(Opcode)    A.01ff  X.cdef  Y.0000  PC.00:020c  S.01fd\n"\
		"LDX: (2)  Read(DL)        A.01ff  X.cdef  Y.0000  PC.00:020d  S.01fd\n"\
		"LDX: (3)  Read(DH)        A.01ff  X.cdef  Y.0000  PC.00:020e  S.01fd\n"\
		"OPC: (1)  Read(Opcode)    A.01ff  X.6789  Y.0000  PC.00:020f  S.01fd\n"\
		"PHX: (2)  IO              A.01ff  X.6789  Y.0000  PC.00:0210  S.01fd\n"\
		"PHX: (3)  Write(DH)       A.01ff  X.6789  Y.0000  PC.00:0210  S.01fd\n"\
		"PHX: (4)  Write(DL)       A.01ff  X.6789  Y.0000  PC.00:0210  S.01fc\n"\
		"OPC: (1)  Read(Opcode)    A.01ff  X.6789  Y.0000  PC.00:0210  S.01fb\n"\
		"LDA: (2)  Read(D0)        A.01ff  X.6789  Y.0000  PC.00:0211  S.01fb\n"\
		"LDA: (3)  IO              A.01ff  X.6789  Y.0000  PC.00:0212  S.01fb\n"\
		"LDA: (4)  Read(DL)        A.01ff  X.6789  Y.0000  PC.00:0212  S.01fb\n"\
		"LDA: (5)  Read(DH)        A.01ff  X.6789  Y.0000  PC.00:0212  S.01fb\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.6789  Y.0000  PC.00:0212  S.01fb\n"\
		"STA: (2)  Read(AAL)       A.cdef  X.6789  Y.0000  PC.00:0213  S.01fb\n"\
		"STA: (3)  Read(AAH)       A.cdef  X.6789  Y.0000  PC.00:0214  S.01fb\n"\
		"STA: (4)  Write(DL)       A.cdef  X.6789  Y.0000  PC.00:0215  S.01fb\n"\
		"STA: (5)  Write(DH)       A.cdef  X.6789  Y.0000  PC.00:0215  S.01fb\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.6789  Y.0000  PC.00:0215  S.01fb\n", cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LoadLDAStackRelativeIndirectIndexedWithY(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, true, true, true, true, true, true, true, false, false, false, false);

	cTestContext.SetShort(0x3003, 0xcdef);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x1ff);
	cTestContext.SetOpcd(TCS_Implied);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x3001);
	cTestContext.SetOpcd(PHX_StackImplied);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x6789);
	cTestContext.SetOpcd(PHX_StackImplied);
	cTestContext.SetOpcd(LDY_Immediate);
	cTestContext.SetShort(2);
	cTestContext.SetOpcd(LDA_StackRelativeIndirectIndexedWithY);
	cTestContext.SetByte(3);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetShort(0x0100);
	cTestContext.SetOpcd(STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(13, uiInstructions);
	AssertShortHex((uint16)0xcdef, cTestContext.GetShort(0x0100));

	cTestContext.StripToInstruction("LDA");
	AssertString(""\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0204  S.01fc\n"\
		"LDA: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0205  S.01fc\n"\
		"LDA: (3)  Read(DH)        A.0000  X.0000  Y.0000  PC.00:0206  S.01fc\n"\
		"OPC: (1)  Read(Opcode)    A.01ff  X.0000  Y.0000  PC.00:0207  S.01fc\n"\
		"TCS: (2)  IO              A.01ff  X.0000  Y.0000  PC.00:0208  S.01fc\n"\
		"OPC: (1)  Read(Opcode)    A.01ff  X.0000  Y.0000  PC.00:0208  S.01ff\n"\
		"LDX: (2)  Read(DL)        A.01ff  X.0000  Y.0000  PC.00:0209  S.01ff\n"\
		"LDX: (3)  Read(DH)        A.01ff  X.0000  Y.0000  PC.00:020a  S.01ff\n"\
		"OPC: (1)  Read(Opcode)    A.01ff  X.3001  Y.0000  PC.00:020b  S.01ff\n"\
		"PHX: (2)  IO              A.01ff  X.3001  Y.0000  PC.00:020c  S.01ff\n"\
		"PHX: (3)  Write(DH)       A.01ff  X.3001  Y.0000  PC.00:020c  S.01ff\n"\
		"PHX: (4)  Write(DL)       A.01ff  X.3001  Y.0000  PC.00:020c  S.01fe\n"\
		"OPC: (1)  Read(Opcode)    A.01ff  X.3001  Y.0000  PC.00:020c  S.01fd\n"\
		"LDX: (2)  Read(DL)        A.01ff  X.3001  Y.0000  PC.00:020d  S.01fd\n"\
		"LDX: (3)  Read(DH)        A.01ff  X.3001  Y.0000  PC.00:020e  S.01fd\n"\
		"OPC: (1)  Read(Opcode)    A.01ff  X.6789  Y.0000  PC.00:020f  S.01fd\n"\
		"PHX: (2)  IO              A.01ff  X.6789  Y.0000  PC.00:0210  S.01fd\n"\
		"PHX: (3)  Write(DH)       A.01ff  X.6789  Y.0000  PC.00:0210  S.01fd\n"\
		"PHX: (4)  Write(DL)       A.01ff  X.6789  Y.0000  PC.00:0210  S.01fc\n"\
		"OPC: (1)  Read(Opcode)    A.01ff  X.6789  Y.0000  PC.00:0210  S.01fb\n"\
		"LDY: (2)  Read(DL)        A.01ff  X.6789  Y.0000  PC.00:0211  S.01fb\n"\
		"LDY: (3)  Read(DH)        A.01ff  X.6789  Y.0000  PC.00:0212  S.01fb\n"\
		"OPC: (1)  Read(Opcode)    A.01ff  X.6789  Y.0002  PC.00:0213  S.01fb\n"\
		"LDA: (2)  Read(D0)        A.01ff  X.6789  Y.0002  PC.00:0214  S.01fb\n"\
		"LDA: (3)  IO              A.01ff  X.6789  Y.0002  PC.00:0215  S.01fb\n"\
		"LDA: (4)  Read(AAL)       A.01ff  X.6789  Y.0002  PC.00:0215  S.01fb\n"\
		"LDA: (5)  Read(AAH)       A.01ff  X.6789  Y.0002  PC.00:0215  S.01fb\n"\
		"LDA: (6)  IO              A.01ff  X.6789  Y.0002  PC.00:0215  S.01fb\n"\
		"LDA: (7)  Read(DL)        A.01ff  X.6789  Y.0002  PC.00:0215  S.01fb\n"\
		"LDA: (8)  Read(DH)        A.01ff  X.6789  Y.0002  PC.00:0215  S.01fb\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.6789  Y.0002  PC.00:0215  S.01fb\n"\
		"STA: (2)  Read(AAL)       A.cdef  X.6789  Y.0002  PC.00:0216  S.01fb\n"\
		"STA: (3)  Read(AAH)       A.cdef  X.6789  Y.0002  PC.00:0217  S.01fb\n"\
		"STA: (4)  Write(DL)       A.cdef  X.6789  Y.0002  PC.00:0218  S.01fb\n"\
		"STA: (5)  Write(DH)       A.cdef  X.6789  Y.0002  PC.00:0218  S.01fb\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.6789  Y.0002  PC.00:0218  S.01fb\n", cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LoadLDADirectPage(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, true, true, true, true, true, true, false, true, false, false, false);

	cTestContext.SetShort(0x7146, 0xcdef);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x7123);
	cTestContext.SetOpcd(PHX_StackImplied);
	cTestContext.SetOpcd(PLD_StackImplied);
	cTestContext.SetOpcd(LDA_DirectPage);
	cTestContext.SetByte(0x23);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetShort(0x0100);
	cTestContext.SetOpcd(STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(9, uiInstructions);
	AssertShortHex((uint16)0xcdef, cTestContext.GetShort(0x0100));

	cTestContext.StripToInstruction("LDX");
	AssertString(""\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0204  DP.0000\n"\
		"LDX: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0205  DP.0000\n"\
		"LDX: (3)  Read(DH)        A.0000  X.0000  Y.0000  PC.00:0206  DP.0000\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.7123  Y.0000  PC.00:0207  DP.0000\n"\
		"PHX: (2)  IO              A.0000  X.7123  Y.0000  PC.00:0208  DP.0000\n"\
		"PHX: (3)  Write(DH)       A.0000  X.7123  Y.0000  PC.00:0208  DP.0000\n"\
		"PHX: (4)  Write(DL)       A.0000  X.7123  Y.0000  PC.00:0208  DP.0000\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.7123  Y.0000  PC.00:0208  DP.0000\n"\
		"PLD: (2)  IO              A.0000  X.7123  Y.0000  PC.00:0209  DP.0000\n"\
		"PLD: (3)  IO              A.0000  X.7123  Y.0000  PC.00:0209  DP.0000\n"\
		"PLD: (4)  Read(DL)        A.0000  X.7123  Y.0000  PC.00:0209  DP.0000\n"\
		"PLD: (5)  Read(DH)        A.0000  X.7123  Y.0000  PC.00:0209  DP.0000\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.7123  Y.0000  PC.00:0209  DP.7123\n"\
		"LDA: (2)  Read(D0)        A.0000  X.7123  Y.0000  PC.00:020a  DP.7123\n"\
		"LDA: (3)  IO              A.0000  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"LDA: (4)  Read(DL)        A.0000  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"LDA: (5)  Read(DH)        A.0000  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"STA: (2)  Read(AAL)       A.cdef  X.7123  Y.0000  PC.00:020c  DP.7123\n"\
		"STA: (3)  Read(AAH)       A.cdef  X.7123  Y.0000  PC.00:020d  DP.7123\n"\
		"STA: (4)  Write(DL)       A.cdef  X.7123  Y.0000  PC.00:020e  DP.7123\n"\
		"STA: (5)  Write(DH)       A.cdef  X.7123  Y.0000  PC.00:020e  DP.7123\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.7123  Y.0000  PC.00:020e  DP.7123\n", cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LoadLDADirectPageIndirect(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x20000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, true, true, true, true, true, true, false, true, false, false, false);

	cTestContext.SetShort(0x7124, 0x3003);

	cTestContext.SetShort(0x003003, 0xcdef);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x7123);
	cTestContext.SetOpcd(PHX_StackImplied);
	cTestContext.SetOpcd(PLD_StackImplied);
	cTestContext.SetOpcd(LDA_DirectPageIndirect);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetShort(0x0100);
	cTestContext.SetOpcd(STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(9, uiInstructions);
	AssertShortHex((uint16)0xcdef, cTestContext.GetShort(0x0100));

	cTestContext.StripToInstruction("LDX");
	AssertString(""\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0204  DP.0000\n"\
		"LDX: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0205  DP.0000\n"\
		"LDX: (3)  Read(DH)        A.0000  X.0000  Y.0000  PC.00:0206  DP.0000\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.7123  Y.0000  PC.00:0207  DP.0000\n"\
		"PHX: (2)  IO              A.0000  X.7123  Y.0000  PC.00:0208  DP.0000\n"\
		"PHX: (3)  Write(DH)       A.0000  X.7123  Y.0000  PC.00:0208  DP.0000\n"\
		"PHX: (4)  Write(DL)       A.0000  X.7123  Y.0000  PC.00:0208  DP.0000\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.7123  Y.0000  PC.00:0208  DP.0000\n"\
		"PLD: (2)  IO              A.0000  X.7123  Y.0000  PC.00:0209  DP.0000\n"\
		"PLD: (3)  IO              A.0000  X.7123  Y.0000  PC.00:0209  DP.0000\n"\
		"PLD: (4)  Read(DL)        A.0000  X.7123  Y.0000  PC.00:0209  DP.0000\n"\
		"PLD: (5)  Read(DH)        A.0000  X.7123  Y.0000  PC.00:0209  DP.0000\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.7123  Y.0000  PC.00:0209  DP.7123\n"\
		"LDA: (2)  Read(D0)        A.0000  X.7123  Y.0000  PC.00:020a  DP.7123\n"\
		"LDA: (3)  IO              A.0000  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"LDA: (4)  Read(AAL)       A.0000  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"LDA: (5)  Read(AAH)       A.0000  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"LDA: (6)  Read(DL)        A.0000  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"LDA: (7)  Read(DH)        A.0000  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"STA: (2)  Read(AAL)       A.cdef  X.7123  Y.0000  PC.00:020c  DP.7123\n"\
		"STA: (3)  Read(AAH)       A.cdef  X.7123  Y.0000  PC.00:020d  DP.7123\n"\
		"STA: (4)  Write(DL)       A.cdef  X.7123  Y.0000  PC.00:020e  DP.7123\n"\
		"STA: (5)  Write(DH)       A.cdef  X.7123  Y.0000  PC.00:020e  DP.7123\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.7123  Y.0000  PC.00:020e  DP.7123\n", cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LoadLDADirectPageIndexedWithX(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x20000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, true, true, true, true, true, true, false, true, false, false, false);

	cTestContext.SetShort(0x8249, 0xcdef);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x4123);
	cTestContext.SetOpcd(PHX_StackImplied);
	cTestContext.SetOpcd(PLD_StackImplied);
	cTestContext.SetOpcd(LDA_DirectPageIndexedWithX);
	cTestContext.SetByte(0x03);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetShort(0x0100);
	cTestContext.SetOpcd(STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(9, uiInstructions);
	AssertShortHex((uint16)0xcdef, cTestContext.GetShort(0x0100));

	cTestContext.StripToInstruction("LDX");
	AssertString(""\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0204  DP.0000\n"\
		"LDX: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0205  DP.0000\n"\
		"LDX: (3)  Read(DH)        A.0000  X.0000  Y.0000  PC.00:0206  DP.0000\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.4123  Y.0000  PC.00:0207  DP.0000\n"\
		"PHX: (2)  IO              A.0000  X.4123  Y.0000  PC.00:0208  DP.0000\n"\
		"PHX: (3)  Write(DH)       A.0000  X.4123  Y.0000  PC.00:0208  DP.0000\n"\
		"PHX: (4)  Write(DL)       A.0000  X.4123  Y.0000  PC.00:0208  DP.0000\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.4123  Y.0000  PC.00:0208  DP.0000\n"\
		"PLD: (2)  IO              A.0000  X.4123  Y.0000  PC.00:0209  DP.0000\n"\
		"PLD: (3)  IO              A.0000  X.4123  Y.0000  PC.00:0209  DP.0000\n"\
		"PLD: (4)  Read(DL)        A.0000  X.4123  Y.0000  PC.00:0209  DP.0000\n"\
		"PLD: (5)  Read(DH)        A.0000  X.4123  Y.0000  PC.00:0209  DP.0000\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.4123  Y.0000  PC.00:0209  DP.4123\n"\
		"LDA: (2)  Read(D0)        A.0000  X.4123  Y.0000  PC.00:020a  DP.4123\n"\
		"LDA: (3)  IO              A.0000  X.4123  Y.0000  PC.00:020b  DP.4123\n"\
		"LDA: (4)  IO              A.0000  X.4123  Y.0000  PC.00:020b  DP.4123\n"\
		"LDA: (5)  Read(DL)        A.0000  X.4123  Y.0000  PC.00:020b  DP.4123\n"\
		"LDA: (6)  Read(DH)        A.0000  X.4123  Y.0000  PC.00:020b  DP.4123\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.4123  Y.0000  PC.00:020b  DP.4123\n"\
		"STA: (2)  Read(AAL)       A.cdef  X.4123  Y.0000  PC.00:020c  DP.4123\n"\
		"STA: (3)  Read(AAH)       A.cdef  X.4123  Y.0000  PC.00:020d  DP.4123\n"\
		"STA: (4)  Write(DL)       A.cdef  X.4123  Y.0000  PC.00:020e  DP.4123\n"\
		"STA: (5)  Write(DH)       A.cdef  X.4123  Y.0000  PC.00:020e  DP.4123\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.4123  Y.0000  PC.00:020e  DP.4123\n", cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LoadLDADirectPageIndirectLong(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x20000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, true, true, true, true, true, true, false, true, false, false, false);

	cTestContext.SetShort(0x7124, 0x3003);
	cTestContext.SetByte(0x01);

	cTestContext.SetShort(0x013003, 0xcdef);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x7123);
	cTestContext.SetOpcd(PHX_StackImplied);
	cTestContext.SetOpcd(PLD_StackImplied);
	cTestContext.SetOpcd(LDA_DirectPageIndirectLong);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetShort(0x0100);
	cTestContext.SetOpcd(STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(9, uiInstructions);
	AssertShortHex((uint16)0xcdef, cTestContext.GetShort(0x0100));

	cTestContext.StripToInstruction("LDX");
	AssertString(""\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0204  DP.0000\n"\
		"LDX: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0205  DP.0000\n"\
		"LDX: (3)  Read(DH)        A.0000  X.0000  Y.0000  PC.00:0206  DP.0000\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.7123  Y.0000  PC.00:0207  DP.0000\n"\
		"PHX: (2)  IO              A.0000  X.7123  Y.0000  PC.00:0208  DP.0000\n"\
		"PHX: (3)  Write(DH)       A.0000  X.7123  Y.0000  PC.00:0208  DP.0000\n"\
		"PHX: (4)  Write(DL)       A.0000  X.7123  Y.0000  PC.00:0208  DP.0000\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.7123  Y.0000  PC.00:0208  DP.0000\n"\
		"PLD: (2)  IO              A.0000  X.7123  Y.0000  PC.00:0209  DP.0000\n"\
		"PLD: (3)  IO              A.0000  X.7123  Y.0000  PC.00:0209  DP.0000\n"\
		"PLD: (4)  Read(DL)        A.0000  X.7123  Y.0000  PC.00:0209  DP.0000\n"\
		"PLD: (5)  Read(DH)        A.0000  X.7123  Y.0000  PC.00:0209  DP.0000\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.7123  Y.0000  PC.00:0209  DP.7123\n"\
		"LDA: (2)  Read(D0)        A.0000  X.7123  Y.0000  PC.00:020a  DP.7123\n"\
		"LDA: (3)  IO              A.0000  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"LDA: (4)  Read(AAL)       A.0000  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"LDA: (5)  Read(AAH)       A.0000  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"LDA: (6)  Read(AAB)       A.0000  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"LDA: (7)  Read(DL)        A.0000  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"LDA: (8)  Read(DH)        A.0000  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.7123  Y.0000  PC.00:020b  DP.7123\n"\
		"STA: (2)  Read(AAL)       A.cdef  X.7123  Y.0000  PC.00:020c  DP.7123\n"\
		"STA: (3)  Read(AAH)       A.cdef  X.7123  Y.0000  PC.00:020d  DP.7123\n"\
		"STA: (4)  Write(DL)       A.cdef  X.7123  Y.0000  PC.00:020e  DP.7123\n"\
		"STA: (5)  Write(DH)       A.cdef  X.7123  Y.0000  PC.00:020e  DP.7123\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.7123  Y.0000  PC.00:020e  DP.7123\n", cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LoadLDADirectPageIndirectIndexedWithY(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x30000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, true, true, true, true, true, true, true, true, true, false, false);

	cTestContext.SetShort(0x4554, 0xeeff);
	cTestContext.SetShort(0x2cdef, 0xcdef);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x1ff);
	cTestContext.SetOpcd(TCS_Implied);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x4455);
	cTestContext.SetOpcd(PHX_StackImplied);
	cTestContext.SetOpcd(PLD_StackImplied);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(1);
	cTestContext.SetOpcd(PHX_StackImplied);
	cTestContext.SetOpcd(PLB_StackImplied);
	cTestContext.SetOpcd(LDY_Immediate);
	cTestContext.SetShort(0xdef0);
	cTestContext.SetOpcd(LDA_DirectPageIndirectIndexedWithY);
	cTestContext.SetByte(0xff);
	cTestContext.SetOpcd(STA_AbsoluteLong);
	cTestContext.SetShort(0x0100);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(15, uiInstructions);
	AssertShortHex((uint16)0xcdef, cTestContext.GetShort(0x0100));

	cTestContext.StripToInstruction("LDA");
	AssertString(""\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0204  S.01fc  DP.0000  DB.00\n"\
		"LDA: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0205  S.01fc  DP.0000  DB.00\n"\
		"LDA: (3)  Read(DH)        A.0000  X.0000  Y.0000  PC.00:0206  S.01fc  DP.0000  DB.00\n"\

		"OPC: (1)  Read(Opcode)    A.01ff  X.0000  Y.0000  PC.00:0207  S.01fc  DP.0000  DB.00\n"\
		"TCS: (2)  IO              A.01ff  X.0000  Y.0000  PC.00:0208  S.01fc  DP.0000  DB.00\n"\

		"OPC: (1)  Read(Opcode)    A.01ff  X.0000  Y.0000  PC.00:0208  S.01ff  DP.0000  DB.00\n"\
		"LDX: (2)  Read(DL)        A.01ff  X.0000  Y.0000  PC.00:0209  S.01ff  DP.0000  DB.00\n"\
		"LDX: (3)  Read(DH)        A.01ff  X.0000  Y.0000  PC.00:020a  S.01ff  DP.0000  DB.00\n"\

		"OPC: (1)  Read(Opcode)    A.01ff  X.4455  Y.0000  PC.00:020b  S.01ff  DP.0000  DB.00\n"\
		"PHX: (2)  IO              A.01ff  X.4455  Y.0000  PC.00:020c  S.01ff  DP.0000  DB.00\n"\
		"PHX: (3)  Write(DH)       A.01ff  X.4455  Y.0000  PC.00:020c  S.01ff  DP.0000  DB.00\n"\
		"PHX: (4)  Write(DL)       A.01ff  X.4455  Y.0000  PC.00:020c  S.01fe  DP.0000  DB.00\n"\

		"OPC: (1)  Read(Opcode)    A.01ff  X.4455  Y.0000  PC.00:020c  S.01fd  DP.0000  DB.00\n"\
		"PLD: (2)  IO              A.01ff  X.4455  Y.0000  PC.00:020d  S.01fd  DP.0000  DB.00\n"\
		"PLD: (3)  IO              A.01ff  X.4455  Y.0000  PC.00:020d  S.01fd  DP.0000  DB.00\n"\
		"PLD: (4)  Read(DL)        A.01ff  X.4455  Y.0000  PC.00:020d  S.01fd  DP.0000  DB.00\n"\
		"PLD: (5)  Read(DH)        A.01ff  X.4455  Y.0000  PC.00:020d  S.01fe  DP.0000  DB.00\n"\

		"OPC: (1)  Read(Opcode)    A.01ff  X.4455  Y.0000  PC.00:020d  S.01ff  DP.4455  DB.00\n"\
		"LDX: (2)  Read(DL)        A.01ff  X.4455  Y.0000  PC.00:020e  S.01ff  DP.4455  DB.00\n"\
		"LDX: (3)  Read(DH)        A.01ff  X.4455  Y.0000  PC.00:020f  S.01ff  DP.4455  DB.00\n"\

		"OPC: (1)  Read(Opcode)    A.01ff  X.0001  Y.0000  PC.00:0210  S.01ff  DP.4455  DB.00\n"\
		"PHX: (2)  IO              A.01ff  X.0001  Y.0000  PC.00:0211  S.01ff  DP.4455  DB.00\n"\
		"PHX: (3)  Write(DH)       A.01ff  X.0001  Y.0000  PC.00:0211  S.01ff  DP.4455  DB.00\n"\
		"PHX: (4)  Write(DL)       A.01ff  X.0001  Y.0000  PC.00:0211  S.01fe  DP.4455  DB.00\n"\

		"OPC: (1)  Read(Opcode)    A.01ff  X.0001  Y.0000  PC.00:0211  S.01fd  DP.4455  DB.00\n"\
		"PLB: (2)  IO              A.01ff  X.0001  Y.0000  PC.00:0212  S.01fd  DP.4455  DB.00\n"\
		"PLB: (3)  IO              A.01ff  X.0001  Y.0000  PC.00:0212  S.01fd  DP.4455  DB.00\n"\
		"PLB: (4)  Read(DL)        A.01ff  X.0001  Y.0000  PC.00:0212  S.01fd  DP.4455  DB.00\n"\

		"OPC: (1)  Read(Opcode)    A.01ff  X.0001  Y.0000  PC.00:0212  S.01fe  DP.4455  DB.01\n"\
		"LDY: (2)  Read(DL)        A.01ff  X.0001  Y.0000  PC.00:0213  S.01fe  DP.4455  DB.01\n"\
		"LDY: (3)  Read(DH)        A.01ff  X.0001  Y.0000  PC.00:0214  S.01fe  DP.4455  DB.01\n"\

		"OPC: (1)  Read(Opcode)    A.01ff  X.0001  Y.def0  PC.00:0215  S.01fe  DP.4455  DB.01\n"\
		"LDA: (2)  Read(D0)        A.01ff  X.0001  Y.def0  PC.00:0216  S.01fe  DP.4455  DB.01\n"\
		"LDA: (3)  IO              A.01ff  X.0001  Y.def0  PC.00:0217  S.01fe  DP.4455  DB.01\n"\
		"LDA: (4)  Read(AAL)       A.01ff  X.0001  Y.def0  PC.00:0217  S.01fe  DP.4455  DB.01\n"\
		"LDA: (5)  Read(AAH)       A.01ff  X.0001  Y.def0  PC.00:0217  S.01fe  DP.4455  DB.01\n"\
		"LDA: (6)  IO              A.01ff  X.0001  Y.def0  PC.00:0217  S.01fe  DP.4455  DB.01\n"\
		"LDA: (7)  Read(DL)        A.01ff  X.0001  Y.def0  PC.00:0217  S.01fe  DP.4455  DB.01\n"\
		"LDA: (8)  Read(DH)        A.01ff  X.0001  Y.def0  PC.00:0217  S.01fe  DP.4455  DB.01\n"\

		"OPC: (1)  Read(Opcode)    A.cdef  X.0001  Y.def0  PC.00:0217  S.01fe  DP.4455  DB.01\n"\
		"STA: (2)  Read(AAL)       A.cdef  X.0001  Y.def0  PC.00:0218  S.01fe  DP.4455  DB.01\n"\
		"STA: (3)  Read(AAH)       A.cdef  X.0001  Y.def0  PC.00:0219  S.01fe  DP.4455  DB.01\n"\
		"STA: (4)  Read(AAB)       A.cdef  X.0001  Y.def0  PC.00:021a  S.01fe  DP.4455  DB.01\n"\
		"STA: (5)  Write(DL)       A.cdef  X.0001  Y.def0  PC.00:021b  S.01fe  DP.4455  DB.01\n"\
		"STA: (6)  Write(DH)       A.cdef  X.0001  Y.def0  PC.00:021b  S.01fe  DP.4455  DB.01\n"\

		"OPC: (1)  Read(Opcode)    A.cdef  X.0001  Y.def0  PC.00:021b  S.01fe  DP.4455  DB.01\n", cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LoadLDADirectPageIndirectLongIndexedWithY(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x30000, 0xea, 0x0200);
	cTestContext.SetPrint(true, true, true, true, true, true, true, true, true, true, true, true, true, false);
	cTestContext.SetSignalPrint(true, true, true, true, true, true, true, true, true, true, true, true);
	cTestContext.SetOperationPrint(true, true, true);

	cTestContext.SetShort(0x4554, 0xeeff);
	cTestContext.SetByte(0x01);

	cTestContext.SetShort(0x2cdef, 0xcdef);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x4455);
	cTestContext.SetOpcd(PHX_StackImplied);
	cTestContext.SetOpcd(PLD_StackImplied);
	cTestContext.SetOpcd(LDY_Immediate);
	cTestContext.SetShort(0xdef0);
	cTestContext.SetOpcd(LDA_DirectPageIndirectLongIndexedWithY);
	cTestContext.SetByte(0xff);
	cTestContext.SetOpcd(STA_AbsoluteLong);
	cTestContext.SetShort(0x0100);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(10, uiInstructions);
	AssertShortHex((uint16)0xcdef, cTestContext.GetShort(0x0100));

	AssertString(""\
		"RES: (1)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01ff  DP.0000  DB.00    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)    Addr.00:0000  Data.ea  RWB1 VPA1 VDA1 VPB1 E1 RDY1 MLB1 M1 X1    PBR,PC        IO                                                             Reset the CPU.\n"\
		"RES: (2)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01ff  DP.0000  DB.00    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)    Addr.00:0000  Data.00  RWB1 VPA0 VDA0 VPB1 E1 RDY1 MLB1 M1 X1    PBR,PC        IO                                                             Reset the CPU.\n"\
		"RES: (3)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01ff  DP.0000  DB.00    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)    Addr.00:01ff  Data.00  RWB1 VPA0 VDA0 VPB1 E1 RDY1 MLB1 M1 X1    S             IO SP--                                                        Reset the CPU.\n"\
		"RES: (4)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01fe  DP.0000  DB.00    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)    Addr.00:01fe  Data.00  RWB1 VPA0 VDA0 VPB1 E1 RDY1 MLB1 M1 X1    S             IO SP--                                                        Reset the CPU.\n"\
		"RES: (5)  IO              A.0000  X.0000  Y.0000  PC.00:0000  S.01fd  DP.0000  DB.00    P(Z0 N0 D0 I0 M.8 X.8 C0 E1 V0 B0)    Addr.00:01fd  Data.00  RWB1 VPA0 VDA0 VPB1 E1 RDY1 MLB1 M1 X1    S             IO SP-- Execute(HELP!)                                         Reset the CPU.\n"\
		"RES: (6)  Read(AAL)       A.0000  X.0000  Y.0000  PC.00:0000  S.01fc  DP.0000  DB.00    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)    Addr.00:fffc  Data.00  RWB1 VPA0 VDA1 VPB0 E1 RDY1 MLB1 M1 X1    VA            Read(AAL)                                                      Reset the CPU.\n"\
		"RES: (7)  Read(AAH)       A.0000  X.0000  Y.0000  PC.00:0000  S.01fc  DP.0000  DB.00    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)    Addr.00:fffd  Data.02  RWB1 VPA0 VDA1 VPB0 E1 RDY1 MLB1 M1 X1    VA+1          Read(AAH) Set(PC) DONE                                         Reset the CPU.\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0200  S.01fc  DP.0000  DB.00    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)    Addr.00:0200  Data.18  RWB1 VPA1 VDA1 VPB1 E1 RDY1 MLB1 M1 X1    PBR,PC        Read(Opcode) PC++                                              Fetch Opcode from address in program counter.\n"\
		"CLC: (2)  IO              A.0000  X.0000  Y.0000  PC.00:0201  S.01fc  DP.0000  DB.00    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)    Addr.00:0201  Data.00  RWB1 VPA0 VDA0 VPB1 E1 RDY1 MLB1 M1 X1    PBR,PC        IO Execute(HELP!) DONE                                         Clear Carry Flag.\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0201  S.01fc  DP.0000  DB.00    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)    Addr.00:0201  Data.fb  RWB1 VPA1 VDA1 VPB1 E1 RDY1 MLB1 M1 X1    PBR,PC        Read(Opcode) PC++                                              Fetch Opcode from address in program counter.\n"\
		"XCE: (2)  IO              A.0000  X.0000  Y.0000  PC.00:0202  S.01fc  DP.0000  DB.00    P(Z0 N0 D0 I1 M.8 X.8 C0 E1 V0 B0)    Addr.00:0202  Data.00  RWB1 VPA0 VDA0 VPB1 E1 RDY1 MLB1 M1 X1    PBR,PC        IO Execute(HELP!) DONE                                         Exchange Carry and Emulation Bits.\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0202  S.01fc  DP.0000  DB.00    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)    Addr.00:0202  Data.c2  RWB1 VPA1 VDA1 VPB1 E0 RDY1 MLB1 M1 X1    PBR,PC        Read(Opcode) PC++                                              Fetch Opcode from address in program counter.\n"\
		"REP: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0203  S.01fc  DP.0000  DB.00    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)    Addr.00:0203  Data.30  RWB1 VPA1 VDA0 VPB1 E0 RDY1 MLB1 M1 X1    PBR,PC        Read(DL)                                                       Reset Status Bits.\n"\
		"REP: (3)  IO              A.0000  X.0000  Y.0000  PC.00:0203  S.01fc  DP.0000  DB.00    P(Z0 N0 D0 I1 M.8 X.8 C1 E0 V0 ..)    Addr.00:0203  Data.00  RWB1 VPA0 VDA0 VPB1 E0 RDY1 MLB1 M1 X1    PBR,PC        IO PC++ Execute(HELP!) DONE                                    Reset Status Bits.\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0204  S.01fc  DP.0000  DB.00    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:0204  Data.a2  RWB1 VPA1 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        Read(Opcode) PC++                                              Fetch Opcode from address in program counter.\n"\
		"LDX: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0205  S.01fc  DP.0000  DB.00    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:0205  Data.55  RWB1 VPA1 VDA0 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        PC++ Read(DL) Execute_If_8Bit_XY(HELP!) DONE_If_8Bit_XY        Load Index X with Memory.\n"\
		"LDX: (3)  Read(DH)        A.0000  X.0000  Y.0000  PC.00:0206  S.01fc  DP.0000  DB.00    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:0206  Data.44  RWB1 VPA1 VDA0 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        PC++ Read(DH) Execute_If_16Bit_XY(HELP!) DONE_If_16Bit_XY      Load Index X with Memory.\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.4455  Y.0000  PC.00:0207  S.01fc  DP.0000  DB.00    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:0207  Data.da  RWB1 VPA1 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        Read(Opcode) PC++                                              Fetch Opcode from address in program counter.\n"\
		"PHX: (2)  IO              A.0000  X.4455  Y.0000  PC.00:0208  S.01fc  DP.0000  DB.00    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:0208  Data.00  RWB1 VPA0 VDA0 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        IO Execute(HELP!)                                              Push Index X on Stack.\n"\
		"PHX: (3)  Write(DH)       A.0000  X.4455  Y.0000  PC.00:0208  S.01fc  DP.0000  DB.00    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:01fc  Data.44  RWB0 VPA0 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    S             Write(DH) SP-- Note(1)                                         Push Index X on Stack.\n"\
		"PHX: (4)  Write(DL)       A.0000  X.4455  Y.0000  PC.00:0208  S.01fb  DP.0000  DB.00    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:01fb  Data.55  RWB0 VPA0 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    S             Write(DL) SP-- DONE                                            Push Index X on Stack.\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.4455  Y.0000  PC.00:0208  S.01fa  DP.0000  DB.00    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:0208  Data.2b  RWB1 VPA1 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        Read(Opcode) PC++                                              Fetch Opcode from address in program counter.\n"\
		"PLD: (2)  IO              A.0000  X.4455  Y.0000  PC.00:0209  S.01fa  DP.0000  DB.00    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:0209  Data.00  RWB1 VPA0 VDA0 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        IO                                                             Pull Direct Register from Stack.\n"\
		"PLD: (3)  IO              A.0000  X.4455  Y.0000  PC.00:0209  S.01fa  DP.0000  DB.00    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:0209  Data.00  RWB1 VPA0 VDA0 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        IO                                                             Pull Direct Register from Stack.\n"\
		"PLD: (4)  Read(DL)        A.0000  X.4455  Y.0000  PC.00:0209  S.01fa  DP.0000  DB.00    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:01fb  Data.55  RWB1 VPA0 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    S+1           Read(DL) SP++                                                  Pull Direct Register from Stack.\n"\
		"PLD: (5)  Read(DH)        A.0000  X.4455  Y.0000  PC.00:0209  S.01fb  DP.0000  DB.00    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:01fc  Data.44  RWB1 VPA0 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    S+1           Read(DH) SP++ Execute(HELP!) DONE                              Pull Direct Register from Stack.\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.4455  Y.0000  PC.00:0209  S.01fc  DP.4455  DB.00    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:0209  Data.a0  RWB1 VPA1 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        Read(Opcode) PC++                                              Fetch Opcode from address in program counter.\n"\
		"LDY: (2)  Read(DL)        A.0000  X.4455  Y.0000  PC.00:020a  S.01fc  DP.4455  DB.00    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:020a  Data.f0  RWB1 VPA1 VDA0 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        PC++ Read(DL) Execute_If_8Bit_XY(HELP!) DONE_If_8Bit_XY        Load Index Y with Memory.\n"\
		"LDY: (3)  Read(DH)        A.0000  X.4455  Y.0000  PC.00:020b  S.01fc  DP.4455  DB.00    P(Z0 N0 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:020b  Data.de  RWB1 VPA1 VDA0 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        PC++ Read(DH) Execute_If_16Bit_XY(HELP!) DONE_If_16Bit_XY      Load Index Y with Memory.\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.4455  Y.def0  PC.00:020c  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:020c  Data.b7  RWB1 VPA1 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        Read(Opcode) PC++                                              Fetch Opcode from address in program counter.\n"\
		"LDA: (2)  Read(D0)        A.0000  X.4455  Y.def0  PC.00:020d  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:020d  Data.ff  RWB1 VPA1 VDA0 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        Read(D0) PC++                                                  Load Accumulator with Memory.\n"\
		"LDA: (3)  IO              A.0000  X.4455  Y.def0  PC.00:020e  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:020d  Data.00  RWB1 VPA0 VDA0 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC-1      IO Note(2)                                                     Load Accumulator with Memory.\n"\
		"LDA: (4)  Read(AAL)       A.0000  X.4455  Y.def0  PC.00:020e  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:4554  Data.ff  RWB1 VPA0 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    D+D0          Read(AAL)                                                      Load Accumulator with Memory.\n"\
		"LDA: (5)  Read(AAH)       A.0000  X.4455  Y.def0  PC.00:020e  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:4555  Data.ee  RWB1 VPA0 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    D+D0+1        Read(AAH)                                                      Load Accumulator with Memory.\n"\
		"LDA: (6)  Read(AAB)       A.0000  X.4455  Y.def0  PC.00:020e  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:4556  Data.01  RWB1 VPA0 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    D+D0+2        Read(AAB)                                                      Load Accumulator with Memory.\n"\
		"LDA: (7)  Read(DL)        A.0000  X.4455  Y.def0  PC.00:020e  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.02:cdef  Data.ef  RWB1 VPA0 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    AAB,AA+Y      Read(DL) Execute_If_8Bit_M(HELP!) DONE_If_8Bit_M               Load Accumulator with Memory.\n"\
		"LDA: (8)  Read(DH)        A.0000  X.4455  Y.def0  PC.00:020e  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.02:cdf0  Data.cd  RWB1 VPA0 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    AAB,AA+Y+1    Read(DH) Execute_If_16Bit_M(HELP!) DONE_If_16Bit_M             Load Accumulator with Memory.\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.4455  Y.def0  PC.00:020e  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:020e  Data.8f  RWB1 VPA1 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        Read(Opcode) PC++                                              Fetch Opcode from address in program counter.\n"\
		"STA: (2)  Read(AAL)       A.cdef  X.4455  Y.def0  PC.00:020f  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:020f  Data.00  RWB1 VPA1 VDA0 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        Read(AAL) PC++                                                 Store Accumulator in Memory.\n"\
		"STA: (3)  Read(AAH)       A.cdef  X.4455  Y.def0  PC.00:0210  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:0210  Data.01  RWB1 VPA1 VDA0 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        Read(AAH) PC++                                                 Store Accumulator in Memory.\n"\
		"STA: (4)  Read(AAB)       A.cdef  X.4455  Y.def0  PC.00:0211  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:0211  Data.00  RWB1 VPA1 VDA0 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        Read(AAB) PC++                                                 Store Accumulator in Memory.\n"\
		"STA: (5)  Write(DL)       A.cdef  X.4455  Y.def0  PC.00:0212  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:0100  Data.ef  RWB0 VPA0 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    AAB,AA        Execute(HELP!) Write(DL) DONE_If_8Bit_M                        Store Accumulator in Memory.\n"\
		"STA: (6)  Write(DH)       A.cdef  X.4455  Y.def0  PC.00:0212  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:0101  Data.cd  RWB0 VPA0 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    AAB,AA+1      Write(DH) DONE_If_16Bit_M                                      Store Accumulator in Memory.\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.4455  Y.def0  PC.00:0212  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:0212  Data.db  RWB1 VPA1 VDA1 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        Read(Opcode) PC++                                              Fetch Opcode from address in program counter.\n"\
		"STP: (2)  IO              A.cdef  X.4455  Y.def0  PC.00:0213  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:0213  Data.00  RWB1 VPA0 VDA0 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        IO                                                             Stop the Clock.\n"\
		"STP: (3)  IO              A.cdef  X.4455  Y.def0  PC.00:0213  S.01fc  DP.4455  DB.00    P(Z0 N1 D0 I1 M16 X16 C1 E0 V0 ..)    Addr.00:0213  Data.00  RWB1 VPA0 VDA0 VPB1 E0 RDY1 MLB1 M0 X0    PBR,PC        IO Execute(HELP!) DONE                                         Stop the Clock.\n", cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816LoadLDADirectPageIndexedIndirectWithX(void)
{
	CW65C816Meta			cMPU;
	uint64					uiInstructions;
	CTestW65C816Context		cTestContext;

	CW65C816InstructionFactory::GetInstance()->Init();

	cTestContext.Init(0x10000, 0xea, 0x0200);
	cTestContext.SetPrint(false, false, true, true, true, true, true, true, true, false, true, false, false, false);

	cTestContext.SetShort(0x0007, 0x3003);
	cTestContext.SetShort(0x3003, 0xcdef);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(4);
	cTestContext.SetOpcd(LDA_DirectPageIndexedIndirectWithX);
	cTestContext.SetByte(3);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetShort(0x0100);
	cTestContext.SetOpcd(STP_Implied);

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cTestContext);

	uiInstructions = cTestContext.Run(&cMPU);

	cMPU.Kill();

	AssertLong(7, uiInstructions);
	AssertShortHex((uint16)0xcdef, cTestContext.GetShort(0x0100));

	cTestContext.StripToInstruction("LDX");
	AssertString(""\
		"OPC: (1)  Read(Opcode)    A.0000  X.0000  Y.0000  PC.00:0204  DP.0000\n"\
		"LDX: (2)  Read(DL)        A.0000  X.0000  Y.0000  PC.00:0205  DP.0000\n"\
		"LDX: (3)  Read(DH)        A.0000  X.0000  Y.0000  PC.00:0206  DP.0000\n"\
		"OPC: (1)  Read(Opcode)    A.0000  X.0004  Y.0000  PC.00:0207  DP.0000\n"\
		"LDA: (2)  Read(D0)        A.0000  X.0004  Y.0000  PC.00:0208  DP.0000\n"\
		"LDA: (4)  IO              A.0000  X.0004  Y.0000  PC.00:0209  DP.0000\n"\
		"LDA: (5)  Read(AAL)       A.0000  X.0004  Y.0000  PC.00:0209  DP.0000\n"\
		"LDA: (6)  Read(AAH)       A.0000  X.0004  Y.0000  PC.00:0209  DP.0000\n"\
		"LDA: (7)  Read(DL)        A.0000  X.0004  Y.0000  PC.00:0209  DP.0000\n"\
		"LDA: (8)  Read(DH)        A.0000  X.0004  Y.0000  PC.00:0209  DP.0000\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.0004  Y.0000  PC.00:0209  DP.0000\n"\
		"STA: (2)  Read(AAL)       A.cdef  X.0004  Y.0000  PC.00:020a  DP.0000\n"\
		"STA: (3)  Read(AAH)       A.cdef  X.0004  Y.0000  PC.00:020b  DP.0000\n"\
		"STA: (4)  Write(DL)       A.cdef  X.0004  Y.0000  PC.00:020c  DP.0000\n"\
		"STA: (5)  Write(DH)       A.cdef  X.0004  Y.0000  PC.00:020c  DP.0000\n"\
		"OPC: (1)  Read(Opcode)    A.cdef  X.0004  Y.0000  PC.00:020c  DP.0000\n", cTestContext.SequenceText());

	cTestContext.Kill();

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816Load(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();


	TestW65C816LoadLDAImmediate();
	TestW65C816LoadLDAAbsolute();
	TestW65C816LoadLDAAbsoluteLong();
	TestW65C816LoadLDAAbsoluteIndexedWithY();
	TestW65C816LoadLDAAbsoluteIndexedWithX();
	TestW65C816LoadLDAAbsoluteLongIndexedWithX();
	TestW65C816LoadLDAStackRelative();
	TestW65C816LoadLDAStackRelativeIndirectIndexedWithY();
	TestW65C816LoadLDADirectPage();
	TestW65C816LoadLDADirectPageIndirect();
	TestW65C816LoadLDADirectPageIndexedWithX();
	TestW65C816LoadLDADirectPageIndirectLong();
	TestW65C816LoadLDADirectPageIndirectIndexedWithY();
	TestW65C816LoadLDADirectPageIndirectLongIndexedWithY();
	TestW65C816LoadLDADirectPageIndexedIndirectWithX();


	TestStatistics();
	NumberKill();
	TypeConverterKill();
}



