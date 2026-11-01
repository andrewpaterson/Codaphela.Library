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
void TestW65C816BranchInContext(CTestW65C816Context* pcContext, char* szExpected)
{
	pcContext->SetPrint(false, false, true, false, false, true, false, false, true, false, false, false, false, true);
	TestW65C816InContext(pcContext, szExpected);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BranchTakenAndNotTaken(uint8 uiSetFlag, uint8 uiBranchIfSet, uint8 uiBranchIfClear, char* szSet, char* szBranchIfSet, char* szBranchIfClear)
{
	// Each branch is tried with the flag set.  The branch that is not taken falls
	// through to the next instruction, the branch that is taken skips the LDA.
	CTestW65C816Context		cTestContext;
	CChars					szExpected;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, uiSetFlag);
	if (uiSetFlag == LDA_Immediate)
	{
		cTestContext.SetByte(0x80);
	}
	cTestContext.SetOpcd(uiBranchIfClear);
	cTestContext.SetByte(0x02);
	cTestContext.SetOpcd(uiBranchIfSet);
	cTestContext.SetByte(0x02);
	cTestContext.SetByte(LDA_Immediate);
	cTestContext.SetByte(0x11);
	cTestContext.SetOpcd(STP_Implied);

	szExpected.Init();
	szExpected.Append(szSet);
	szExpected.Append(szBranchIfClear);
	szExpected.Append(szBranchIfSet);

	TestW65C816BranchInContext(&cTestContext, szExpected.Text());

	szExpected.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BranchConditional(void)
{
	TestW65C816BranchTakenAndNotTaken(SEC_Implied, BCS_ProgramCounterRelative, BCC_ProgramCounterRelative,
		"SEC: A.0000  PC.00:0201\n",
		"BCS: A.0000  PC.00:0207\n",
		"BCC: A.0000  PC.00:0203\n");

	TestW65C816BranchTakenAndNotTaken(LDA_Immediate, BMI_ProgramCounterRelative, BPL_ProgramCounterRelative,
		"LDA: A.0080  PC.00:0202\n",
		"BMI: A.0080  PC.00:0208\n",
		"BPL: A.0080  PC.00:0204\n");

	TestW65C816BranchTakenAndNotTaken(INX_Implied, BNE_ProgramCounterRelative, BEQ_ProgramCounterRelative,
		"INX: A.0000  PC.00:0201\n",
		"BNE: A.0000  PC.00:0207\n",
		"BEQ: A.0000  PC.00:0203\n");

	TestW65C816BranchTakenAndNotTaken(TAX_Implied, BEQ_ProgramCounterRelative, BNE_ProgramCounterRelative,
		"TAX: A.0000  PC.00:0201\n",
		"BEQ: A.0000  PC.00:0207\n",
		"BNE: A.0000  PC.00:0203\n");

	TestW65C816BranchTakenAndNotTaken(CLC_Implied, BCC_ProgramCounterRelative, BCS_ProgramCounterRelative,
		"CLC: A.0000  PC.00:0201\n",
		"BCC: A.0000  PC.00:0207\n",
		"BCS: A.0000  PC.00:0203\n");

	TestW65C816BranchTakenAndNotTaken(CLV_Implied, BVC_ProgramCounterRelative, BVS_ProgramCounterRelative,
		"CLV: A.0000  PC.00:0201\n",
		"BVC: A.0000  PC.00:0207\n",
		"BVS: A.0000  PC.00:0203\n");

	TestW65C816BranchTakenAndNotTaken(TAY_Implied, BPL_ProgramCounterRelative, BMI_ProgramCounterRelative,
		"TAY: A.0000  PC.00:0201\n",
		"BPL: A.0000  PC.00:0207\n",
		"BMI: A.0000  PC.00:0203\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BranchOverflowSet(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x7f);
	cTestContext.SetOpcd(ADC_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(BVC_ProgramCounterRelative);
	cTestContext.SetByte(0x02);
	cTestContext.SetOpcd(BVS_ProgramCounterRelative);
	cTestContext.SetByte(0x02);
	cTestContext.SetByte(LDA_Immediate);
	cTestContext.SetByte(0x11);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816BranchInContext(&cTestContext, ""\
		"LDA: A.007f  PC.00:0202\n"\
		"ADC: A.0080  PC.00:0204\n"\
		"BVC: A.0080  PC.00:0206\n"\
		"BVS: A.0080  PC.00:020a\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BranchBackward(void)
{
	// A count down loop.  The offset is relative to the instruction after the branch.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDX_Immediate);
	cTestContext.SetByte(0x03);
	cTestContext.SetOpcd(DEX_Implied);
	cTestContext.SetOpcd(BNE_ProgramCounterRelative);
	cTestContext.SetByte(0xfd);
	cTestContext.SetOpcd(STP_Implied);
	cTestContext.muiInstructions += 4;

	cTestContext.SetPrint(false, false, true, false, false, false, true, false, true, false, false, false, false, true);
	TestW65C816InContext(&cTestContext, ""\
		"LDX: X.0003  PC.00:0202\n"\
		"DEX: X.0002  PC.00:0203\n"\
		"BNE: X.0002  PC.00:0202\n"\
		"DEX: X.0001  PC.00:0203\n"\
		"BNE: X.0001  PC.00:0202\n"\
		"DEX: X.0000  PC.00:0203\n"\
		"BNE: X.0000  PC.00:0205\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BranchAlways(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, BRA_ProgramCounterRelative);
	cTestContext.SetByte(0x7f);
	cTestContext.SetOpcd(0x0281, BRA_ProgramCounterRelative);
	cTestContext.SetByte(0x80);
	cTestContext.SetOpcd(0x0203, STP_Implied);

	TestW65C816BranchInContext(&cTestContext, ""\
		"BRA: A.0000  PC.00:0281\n"\
		"BRA: A.0000  PC.00:0203\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BranchLong(void)
{
	// BRL has a signed 16 bit offset and stays in the program bank.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, BRL_ProgramCounterRelativeLong);
	cTestContext.SetShort(0x7000);
	cTestContext.SetOpcd(0x7203, BRL_ProgramCounterRelativeLong);
	cTestContext.SetShort(0x8000);
	cTestContext.SetOpcd(0xf206, BRL_ProgramCounterRelativeLong);
	cTestContext.SetShort(0x1000);
	cTestContext.SetOpcd(0x0209, STP_Implied);

	TestW65C816BranchInContext(&cTestContext, ""\
		"BRL: A.0000  PC.00:7203\n"\
		"BRL: A.0000  PC.00:f206\n"\
		"BRL: A.0000  PC.00:0209\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BranchWrapsInBank(void)
{
	// Datasheet 3.4: the program bank does not change when a branch wraps.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x20000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, JML_AbsoluteLong);
	cTestContext.SetShort(0xfff0);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(0x1fff0, BRA_ProgramCounterRelative);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(0x10002, STP_Implied);

	TestW65C816BranchInContext(&cTestContext, ""\
		"JMP: A.0000  PC.01:fff0\n"\
		"BRA: A.0000  PC.01:0002\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BranchJMPAbsolute(void)
{
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, JMP_Absolute);
	cTestContext.SetShort(0x1234);
	cTestContext.SetOpcd(0x1234, STP_Implied);

	TestW65C816BranchInContext(&cTestContext, ""\
		"JMP: A.0000  PC.00:1234\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BranchJMPAbsoluteLong(void)
{
	// Datasheet 3.4: JMP al (JML) loads the program bank as well as the program counter.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x20000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, JML_AbsoluteLong);
	cTestContext.SetShort(0x1234);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(0x11234, STP_Implied);

	TestW65C816BranchInContext(&cTestContext, ""\
		"JMP: A.0000  PC.01:1234\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BranchJMPAbsoluteIndirect(void)
{
	// Datasheet 7.9: JMP (a) reads its pointer from bank 0 and stays in the program bank.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x20000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, JML_AbsoluteLong);
	cTestContext.SetShort(0x8000);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(0x18000, JMP_AbsoluteIndirect);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(0x14567, STP_Implied);

	cTestContext.SetShort(0x03000, 0x4567);
	cTestContext.SetShort(0x13000, 0x9999);

	TestW65C816BranchInContext(&cTestContext, ""\
		"JMP: A.0000  PC.01:8000\n"\
		"JMP: A.0000  PC.01:4567\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BranchJMPAbsoluteIndexedIndirect(void)
{
	// Datasheet 7.9: JMP (a,x) reads its pointer from the program bank.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x20000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, JML_AbsoluteLong);
	cTestContext.SetShort(0x8000);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(0x18000, LDX_Immediate);
	cTestContext.SetByte(0x04);
	cTestContext.SetOpcd(JMP_AbsoluteIndexedIndirectWithX);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(0x14567, STP_Implied);

	cTestContext.SetShort(0x03004, 0x9999);
	cTestContext.SetShort(0x13004, 0x4567);

	TestW65C816BranchInContext(&cTestContext, ""\
		"JMP: A.0000  PC.01:8000\n"\
		"LDX: A.0000  PC.01:8002\n"\
		"JMP: A.0000  PC.01:4567\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BranchJMLAbsoluteIndirectLong(void)
{
	// Datasheet 7.9 and table 5-7 3a: JML [a] reads a three byte pointer from bank 0.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x30000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, JML_AbsoluteLong);
	cTestContext.SetShort(0x8000);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(0x18000, JML_AbsoluteIndirectLong);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(0x24567, STP_Implied);

	cTestContext.SetShort(0x03000, 0x4567);
	cTestContext.SetByte(0x02);
	cTestContext.SetShort(0x13000, 0x9999);
	cTestContext.SetByte(0x01);

	TestW65C816BranchInContext(&cTestContext, ""\
		"JMP: A.0000  PC.01:8000\n"\
		"JMP: A.0000  PC.02:4567\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816Branch(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();


	TestW65C816BranchConditional();
	TestW65C816BranchOverflowSet();
	TestW65C816BranchBackward();
	TestW65C816BranchAlways();
	TestW65C816BranchLong();
	TestW65C816BranchWrapsInBank();
	TestW65C816BranchJMPAbsolute();
	TestW65C816BranchJMPAbsoluteLong();
	TestW65C816BranchJMPAbsoluteIndirect();
	TestW65C816BranchJMPAbsoluteIndexedIndirect();
	TestW65C816BranchJMLAbsoluteIndirectLong();


	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

