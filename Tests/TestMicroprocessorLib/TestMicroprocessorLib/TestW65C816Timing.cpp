#include <string.h>
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
void TestW65C816TimingAppendCycles(CChars* pszCycles, char* szMnemonic, int iCycles)
{
	if (pszCycles->Length() > 0)
	{
		pszCycles->Append(' ');
	}
	pszCycles->Append(szMnemonic);
	pszCycles->Append(':');
	pszCycles->Append(iCycles);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingCycleCounts(CTestW65C816Context* pcContext, char* szExpected)
{
	// Runs the program and reduces the cycle trace to "MNE:cycles" for each
	// instruction.  Each instruction starts with an opcode fetch line "OPC: (1)".
	char*	szSequence;
	char*	szLine;
	char*	szEnd;
	CChars	szCycles;
	char	szMnemonic[4];
	int		iCycles;

	pcContext->SetPrint(false, false, true, true, false, false, false, false, false, false, false, false, false, false);
	TestW65C816RunInContext(pcContext, NULL);

	szCycles.Init();
	szSequence = pcContext->SequenceText();
	iCycles = 0;
	szMnemonic[0] = '\0';
	szLine = szSequence;
	while (*szLine)
	{
		szEnd = strchr(szLine, '\n');
		if (szEnd == NULL)
		{
			break;
		}

		if (strncmp(szLine, "OPC:", 4) == 0)
		{
			if ((iCycles > 0) && (szMnemonic[0] != '\0'))
			{
				TestW65C816TimingAppendCycles(&szCycles, szMnemonic, iCycles);
			}
			iCycles = 1;
			szMnemonic[0] = '\0';
		}
		else
		{
			if (szMnemonic[0] == '\0')
			{
				memcpy(szMnemonic, szLine, 3);
				szMnemonic[3] = '\0';
			}
			iCycles++;
		}
		szLine = szEnd + 1;
	}
	if ((iCycles > 0) && (szMnemonic[0] != '\0'))
	{
		TestW65C816TimingAppendCycles(&szCycles, szMnemonic, iCycles);
	}

	AssertString(szExpected, szCycles.Text());

	szCycles.Kill();
	pcContext->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingSignalsInContext(CTestW65C816Context* pcContext, char* szInstruction, char* szExpected)
{
	pcContext->SetPrint(false, false, true, true, false, false, false, false, false, false, false, false, false, false);
	pcContext->SetSignalPrint(true, false, true, true, false, true, true, true, false, false, false, true);
	TestW65C816RunInContext(pcContext, NULL);

	pcContext->StripToInstruction(szInstruction);
	AssertString(szExpected, pcContext->SequenceText());

	pcContext->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingImpliedAndImmediate(void)
{
	// Table 5-7 18, 19a, 19b and note 1: implied is 2 cycles, XBA 3, immediate 2 plus
	// 1 for 16 bit data, REP and SEP always 3.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(XBA_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x1234);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetShort(0x1234);
	cTestContext.SetOpcd(SEP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(NOP_Implied);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816TimingCycleCounts(&cTestContext, "CLC:2 XCE:2 LDA:2 XBA:3 REP:3 LDA:3 LDX:3 SEP:3 NOP:2");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingAbsolute(void)
{
	// Table 5-7 1a, 1d: absolute is 4 cycles plus 1 for 16 bit data.  Read-modify-write
	// is 6 plus 2 for 16 bit data.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDA_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(INC_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(LDA_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(STA_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(INC_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816TimingCycleCounts(&cTestContext, "LDA:4 STA:4 INC:6 CLC:2 XCE:2 REP:3 LDA:5 STA:5 INC:8");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingAbsoluteIndexed(void)
{
	// Table 5-7 6a, 7 and note 4: absolute indexed adds a cycle when the index crosses
	// a page, when writing, or when X=0.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDX_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(LDA_AbsoluteIndexedWithX);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(LDA_AbsoluteIndexedWithX);
	cTestContext.SetShort(0x30ff);
	cTestContext.SetOpcd(STA_AbsoluteIndexedWithX);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(LDY_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(LDA_AbsoluteIndexedWithY);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(LDA_AbsoluteIndexedWithY);
	cTestContext.SetShort(0x30ff);
	cTestContext.SetOpcd(CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(LDA_AbsoluteIndexedWithX);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816TimingCycleCounts(&cTestContext, "LDX:2 LDA:4 LDA:5 STA:5 LDY:2 LDA:4 LDA:5 CLC:2 XCE:2 REP:3 LDA:5");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingDirect(void)
{
	// Table 5-7 10a and note 2: direct is 3 cycles plus 1 when DL is not zero.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDA_DirectPage);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x20);
	cTestContext.SetOpcd(LDA_Immediate);
	cTestContext.SetShort(0x0001);
	cTestContext.SetOpcd(TCD_Implied);
	cTestContext.SetOpcd(LDA_DirectPage);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816TimingCycleCounts(&cTestContext, "LDA:3 CLC:2 XCE:2 REP:3 LDA:3 TCD:2 LDA:5");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingBranchEmulation(void)
{
	// Table 5-7 20 and notes 5, 6: a branch is 2 cycles, plus 1 when taken, plus 1
	// more when taken across a page in emulation mode.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(BCS_ProgramCounterRelative);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(BCC_ProgramCounterRelative);
	cTestContext.SetByte(0x7f);
	cTestContext.SetOpcd(0x0284, BCC_ProgramCounterRelative);
	cTestContext.SetByte(0x7f);
	cTestContext.SetOpcd(0x0305, STP_Implied);

	TestW65C816TimingCycleCounts(&cTestContext, "CLC:2 BCS:2 BCC:3 BCC:4");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingBranchNative(void)
{
	// Table 5-7 note 6: no page crossing penalty in native mode.  BRL is 4 cycles.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(BRA_ProgramCounterRelative);
	cTestContext.SetByte(0x7f);
	cTestContext.SetOpcd(0x0283, BRA_ProgramCounterRelative);
	cTestContext.SetByte(0x7f);
	cTestContext.SetOpcd(0x0304, BRL_ProgramCounterRelativeLong);
	cTestContext.SetShort(0x1000);
	cTestContext.SetOpcd(0x1307, STP_Implied);

	TestW65C816TimingCycleCounts(&cTestContext, "CLC:2 XCE:2 BRA:3 BRA:3 BRL:4");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingStack(void)
{
	// Table 5-7 22b to 22f: push 3 (+1 for 16 bit), pull 4 (+1 for 16 bit), PEA 5,
	// PEI 6, PER 6, PHD 4, PLD 5.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, PHA_StackImplied);
	cTestContext.SetOpcd(PLA_Stack);
	cTestContext.SetOpcd(PHP_StackImplied);
	cTestContext.SetOpcd(PLP_StackImplied);
	cTestContext.SetOpcd(CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(REP_Immediate);
	cTestContext.SetByte(0x30);
	cTestContext.SetOpcd(PHA_StackImplied);
	cTestContext.SetOpcd(PLA_Stack);
	cTestContext.SetOpcd(PHX_StackImplied);
	cTestContext.SetOpcd(PLX_StackImplied);
	cTestContext.SetOpcd(PHD_StackImplied);
	cTestContext.SetOpcd(PLD_StackImplied);
	cTestContext.SetOpcd(PEI_StackDirectPageIndirect);
	cTestContext.SetByte(0x10);
	cTestContext.SetOpcd(PER_StackProgramCounterRelativeLong);
	cTestContext.SetShort(0x0000);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816TimingCycleCounts(&cTestContext, "PHA:3 PLA:4 PHP:3 PLP:4 CLC:2 XCE:2 REP:3 PHA:4 PLA:5 PHX:4 PLX:5 PHD:4 PLD:5 PEI:6 PER:6");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingSubroutines(void)
{
	// Table 5-7 1c, 2b, 4c, 22h, 22i: JSR 6, JSR (a,x) 8, JSL 8, RTS 6, RTL 6.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, JSL_AbsoluteLong);
	cTestContext.SetShort(0x0310);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(JSR_Absolute);
	cTestContext.SetShort(0x0300);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(JSR_AbsoluteIndexedIndirectWithX);
	cTestContext.SetShort(0x0400);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0x0400, 0x0300);
	cTestContext.SetOpcd(0x0300, RTS_StackImplied);
	cTestContext.SetOpcd(0x0310, RTL_StackImplied);
	cTestContext.muiInstructions++;

	TestW65C816TimingCycleCounts(&cTestContext, "JSL:8 RTL:6 JSR:6 RTS:6 LDX:2 JSR:8 RTS:6");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingJumps(void)
{
	// Table 5-7 1b, 2a, 3a, 3b, 4b: JMP a 3, JMP (a,x) 6, JML [a] 6, JMP (a) 5, JMP al 4.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, JMP_Absolute);
	cTestContext.SetShort(0x0300);
	cTestContext.SetOpcd(0x0300, LDX_Immediate);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(JMP_AbsoluteIndexedIndirectWithX);
	cTestContext.SetShort(0x0400);
	cTestContext.SetOpcd(0x0310, JML_AbsoluteIndirectLong);
	cTestContext.SetShort(0x0402);
	cTestContext.SetOpcd(0x0320, JMP_AbsoluteIndirect);
	cTestContext.SetShort(0x0405);
	cTestContext.SetOpcd(0x0330, JML_AbsoluteLong);
	cTestContext.SetShort(0x0340);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(0x0340, STP_Implied);

	cTestContext.SetShort(0x0400, 0x0310);
	cTestContext.SetShort(0x0402, 0x0320);
	cTestContext.SetByte(0x00);
	cTestContext.SetShort(0x0405, 0x0330);

	TestW65C816TimingCycleCounts(&cTestContext, "JMP:3 LDX:2 JMP:6 JMP:6 JMP:5 JMP:4");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingInterruptsNative(void)
{
	// Table 5-7 22g, 22j: BRK and COP are 8 cycles in native mode, RTI is 7.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, CLC_Implied);
	cTestContext.SetOpcd(XCE_Implied);
	cTestContext.SetOpcd(BRK_Interrupt);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(COP_Interrupt);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0xffe6, 0x0300);
	cTestContext.SetShort(0xffe4, 0x0300);
	cTestContext.SetOpcd(0x0300, RTI_StackImplied);
	cTestContext.muiInstructions++;

	TestW65C816TimingCycleCounts(&cTestContext, "CLC:2 XCE:2 BRK:8 RTI:7 COP:8 RTI:7");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingInterruptsEmulation(void)
{
	// Table 5-7 note 7: BRK is 7 cycles in emulation mode as PBR is not pushed.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, BRK_Interrupt);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0xfffe, 0x0300);
	cTestContext.SetOpcd(0x0300, STP_Implied);
	cTestContext.muiInstructions--;

	TestW65C816TimingCycleCounts(&cTestContext, "BRK:7");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingBlockMove(void)
{
	// Table 5-7 9a: a block move is 7 cycles per byte.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(MVN_BlockMove);
	cTestContext.SetByte(0x00);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(STP_Implied);
	cTestContext.muiInstructions++;

	TestW65C816TimingCycleCounts(&cTestContext, "LDA:2 MVN:7 MVN:7");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingSignalsImmediate(void)
{
	// Table 5-7 18 and 2.26: the opcode fetch has VDA and VPA high, the operand fetch
	// has only VPA high.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816TimingSignalsInContext(&cTestContext, "LDA", ""\
		"OPC: (1)    Addr.00:0200  RWB1 VPA1 VDA1 VPB1 MLB1\n"\
		"LDA: (2)    Addr.00:0201  RWB1 VPA1 VDA0 VPB1 MLB1\n"\
		"OPC: (1)    Addr.00:0202  RWB1 VPA1 VDA1 VPB1 MLB1\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingSignalsAbsolute(void)
{
	// Table 5-7 1a: the address operand bytes are program fetches (VPA), the data
	// access is a data fetch (VDA) at DBR,AA.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, STA_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816TimingSignalsInContext(&cTestContext, "STA", ""\
		"OPC: (1)    Addr.00:0200  RWB1 VPA1 VDA1 VPB1 MLB1\n"\
		"STA: (2)    Addr.00:0201  RWB1 VPA1 VDA0 VPB1 MLB1\n"\
		"STA: (3)    Addr.00:0202  RWB1 VPA1 VDA0 VPB1 MLB1\n"\
		"STA: (4)    Addr.00:3000  RWB0 VPA0 VDA1 VPB1 MLB1\n"\
		"OPC: (1)    Addr.00:0203  RWB1 VPA1 VDA1 VPB1 MLB1\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingSignalsReadModifyWrite(void)
{
	// Table 5-7 1d and notes 3, 17: MLB is low for the read, modify and write cycles.
	// In emulation mode RWB is low for the modify cycle as well as the write.  The
	// cycle numbers are the rows of table 5-7, 4a and 6a are skipped for 8 bit data.
	// Table 5-7 puts the modify cycle at AA+1 but with 8 bit data it is at AA (as in
	// the SingleStepTests).
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, INC_Absolute);
	cTestContext.SetShort(0x3000);
	cTestContext.SetOpcd(STP_Implied);

	TestW65C816TimingSignalsInContext(&cTestContext, "INC", ""\
		"OPC: (1)    Addr.00:0200  RWB1 VPA1 VDA1 VPB1 MLB1\n"\
		"INC: (2)    Addr.00:0201  RWB1 VPA1 VDA0 VPB1 MLB1\n"\
		"INC: (3)    Addr.00:0202  RWB1 VPA1 VDA0 VPB1 MLB1\n"\
		"INC: (4)    Addr.00:3000  RWB1 VPA0 VDA1 VPB1 MLB0\n"\
		"INC: (6)    Addr.00:3000  RWB0 VPA0 VDA0 VPB1 MLB0\n"\
		"INC: (8)    Addr.00:3000  RWB0 VPA0 VDA1 VPB1 MLB0\n"\
		"OPC: (1)    Addr.00:0203  RWB1 VPA1 VDA1 VPB1 MLB1\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816TimingSignalsBRK(void)
{
	// Table 5-7 22j and 2.28: VPB is low for the two vector reads.  In emulation mode
	// PBR is not pushed so row 3 is skipped.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, BRK_Interrupt);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(STP_Implied);

	cTestContext.SetShort(0xfffe, 0x0300);
	cTestContext.SetOpcd(0x0300, STP_Implied);
	cTestContext.muiInstructions--;

	TestW65C816TimingSignalsInContext(&cTestContext, "BRK", ""\
		"OPC: (1)    Addr.00:0200  RWB1 VPA1 VDA1 VPB1 MLB1\n"\
		"BRK: (2)    Addr.00:0201  RWB1 VPA1 VDA0 VPB1 MLB1\n"\
		"BRK: (4)    Addr.00:01fc  RWB0 VPA0 VDA1 VPB1 MLB1\n"\
		"BRK: (5)    Addr.00:01fb  RWB0 VPA0 VDA1 VPB1 MLB1\n"\
		"BRK: (6)    Addr.00:01fa  RWB0 VPA0 VDA1 VPB1 MLB1\n"\
		"BRK: (7)    Addr.00:fffe  RWB1 VPA0 VDA1 VPB0 MLB1\n"\
		"BRK: (8)    Addr.00:ffff  RWB1 VPA0 VDA1 VPB0 MLB1\n"\
		"OPC: (1)    Addr.00:0300  RWB1 VPA1 VDA1 VPB1 MLB1\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816Timing(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();


	TestW65C816TimingImpliedAndImmediate();
	TestW65C816TimingAbsolute();
	TestW65C816TimingAbsoluteIndexed();
	TestW65C816TimingDirect();
	TestW65C816TimingBranchEmulation();
	TestW65C816TimingBranchNative();
	TestW65C816TimingStack();
	TestW65C816TimingSubroutines();
	TestW65C816TimingJumps();
	TestW65C816TimingInterruptsNative();
	TestW65C816TimingInterruptsEmulation();
	TestW65C816TimingBlockMove();
	TestW65C816TimingSignalsImmediate();
	TestW65C816TimingSignalsAbsolute();
	TestW65C816TimingSignalsReadModifyWrite();
	TestW65C816TimingSignalsBRK();


	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

