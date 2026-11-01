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
void TestW65C816BlockMoveSetPrint(CTestW65C816Context* pcContext)
{
	pcContext->SetPrint(false, false, true, false, false, true, true, true, true, false, false, true, false, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BlockMoveNative(CTestW65C816Context* pcContext, uint16 uiCount, uint16 uiSource, uint16 uiDestination)
{
	pcContext->SetOpcd(0x0200, CLC_Implied);
	pcContext->SetOpcd(XCE_Implied);
	pcContext->SetOpcd(REP_Immediate);
	pcContext->SetByte(0x30);
	pcContext->SetOpcd(LDA_Immediate);
	pcContext->SetShort(uiCount);
	pcContext->SetOpcd(LDX_Immediate);
	pcContext->SetShort(uiSource);
	pcContext->SetOpcd(LDY_Immediate);
	pcContext->SetShort(uiDestination);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BlockMoveMVN(void)
{
	// Datasheet 3.5.9: MVN moves C+1 bytes from srcbnk:X to dstbnk:Y, incrementing X
	// and Y.  The instruction repeats itself once per byte (seven cycles each) and
	// leaves C at FFFF.  Datasheet 7.18: DBR becomes the destination bank.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x30000, 0xea, 0x0200);

	TestW65C816BlockMoveNative(&cTestContext, 0x0002, 0x1000, 0x2000);
	cTestContext.SetOpcd(MVN_BlockMove);
	cTestContext.SetByte(0x02);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(STP_Implied);
	cTestContext.muiInstructions += 2;

	cTestContext.SetByte(0x11000, 0x11);
	cTestContext.SetByte(0x22);
	cTestContext.SetByte(0x33);
	cTestContext.SetByte(0x44);

	TestW65C816BlockMoveSetPrint(&cTestContext);
	TestW65C816RunInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000  PC.00:0201  DB.00\n"\
		"XCE: A.0000  X.0000  Y.0000  PC.00:0202  DB.00\n"\
		"REP: A.0000  X.0000  Y.0000  PC.00:0204  DB.00\n"\
		"LDA: A.0002  X.0000  Y.0000  PC.00:0207  DB.00\n"\
		"LDX: A.0002  X.1000  Y.0000  PC.00:020a  DB.00\n"\
		"LDY: A.0002  X.1000  Y.2000  PC.00:020d  DB.00\n"\
		"MVN: A.0001  X.1001  Y.2001  PC.00:020d  DB.02\n"\
		"MVN: A.0000  X.1002  Y.2002  PC.00:020d  DB.02\n"\
		"MVN: A.ffff  X.1003  Y.2003  PC.00:0210  DB.02\n");

	AssertByteHex((uint8)0x11, cTestContext.GetByte(0x22000));
	AssertByteHex((uint8)0x22, cTestContext.GetByte(0x22001));
	AssertByteHex((uint8)0x33, cTestContext.GetByte(0x22002));
	AssertByteHex((uint8)0xea, cTestContext.GetByte(0x22003));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BlockMoveMVP(void)
{
	// MVP moves from the end of the block down, decrementing X and Y.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x30000, 0xea, 0x0200);

	TestW65C816BlockMoveNative(&cTestContext, 0x0002, 0x1002, 0x2002);
	cTestContext.SetOpcd(MVP_BlockMove);
	cTestContext.SetByte(0x02);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(STP_Implied);
	cTestContext.muiInstructions += 2;

	cTestContext.SetByte(0x10fff, 0x00);
	cTestContext.SetByte(0x11);
	cTestContext.SetByte(0x22);
	cTestContext.SetByte(0x33);

	TestW65C816BlockMoveSetPrint(&cTestContext);
	TestW65C816RunInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000  PC.00:0201  DB.00\n"\
		"XCE: A.0000  X.0000  Y.0000  PC.00:0202  DB.00\n"\
		"REP: A.0000  X.0000  Y.0000  PC.00:0204  DB.00\n"\
		"LDA: A.0002  X.0000  Y.0000  PC.00:0207  DB.00\n"\
		"LDX: A.0002  X.1002  Y.0000  PC.00:020a  DB.00\n"\
		"LDY: A.0002  X.1002  Y.2002  PC.00:020d  DB.00\n"\
		"MVP: A.0001  X.1001  Y.2001  PC.00:020d  DB.02\n"\
		"MVP: A.0000  X.1000  Y.2000  PC.00:020d  DB.02\n"\
		"MVP: A.ffff  X.0fff  Y.1fff  PC.00:0210  DB.02\n");

	AssertByteHex((uint8)0xea, cTestContext.GetByte(0x21fff));
	AssertByteHex((uint8)0x11, cTestContext.GetByte(0x22000));
	AssertByteHex((uint8)0x22, cTestContext.GetByte(0x22001));
	AssertByteHex((uint8)0x33, cTestContext.GetByte(0x22002));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BlockMoveSameBank(void)
{
	// An overlapping move up within one bank with MVP.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	TestW65C816BlockMoveNative(&cTestContext, 0x0003, 0x3003, 0x3005);
	cTestContext.SetOpcd(MVP_BlockMove);
	cTestContext.SetByte(0x00);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(STP_Implied);
	cTestContext.muiInstructions += 3;

	cTestContext.SetByte(0x3000, 0x01);
	cTestContext.SetByte(0x02);
	cTestContext.SetByte(0x03);
	cTestContext.SetByte(0x04);

	cTestContext.SetPrint(false, false, true, false, false, true, true, true, false, false, false, false, false, true);
	TestW65C816RunInContext(&cTestContext, ""\
		"CLC: A.0000  X.0000  Y.0000\n"\
		"XCE: A.0000  X.0000  Y.0000\n"\
		"REP: A.0000  X.0000  Y.0000\n"\
		"LDA: A.0003  X.0000  Y.0000\n"\
		"LDX: A.0003  X.3003  Y.0000\n"\
		"LDY: A.0003  X.3003  Y.3005\n"\
		"MVP: A.0002  X.3002  Y.3004\n"\
		"MVP: A.0001  X.3001  Y.3003\n"\
		"MVP: A.0000  X.3000  Y.3002\n"\
		"MVP: A.ffff  X.2fff  Y.3001\n");

	AssertByteHex((uint8)0x01, cTestContext.GetByte(0x3000));
	AssertByteHex((uint8)0x02, cTestContext.GetByte(0x3001));
	AssertByteHex((uint8)0x01, cTestContext.GetByte(0x3002));
	AssertByteHex((uint8)0x02, cTestContext.GetByte(0x3003));
	AssertByteHex((uint8)0x03, cTestContext.GetByte(0x3004));
	AssertByteHex((uint8)0x04, cTestContext.GetByte(0x3005));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BlockMoveEmulation(void)
{
	// Datasheet 7.8.3.2: in emulation mode X and Y are 8 bit so the block move
	// addresses stay in 0000 to 00FF of each bank.  C is still a 16 bit count.
	CTestW65C816Context		cTestContext;

	cTestContext.Init(0x10000, 0xea, 0x0200);

	cTestContext.SetOpcd(0x0200, LDA_Immediate);
	cTestContext.SetByte(0x01);
	cTestContext.SetOpcd(LDX_Immediate);
	cTestContext.SetByte(0xff);
	cTestContext.SetOpcd(LDY_Immediate);
	cTestContext.SetByte(0x40);
	cTestContext.SetOpcd(MVN_BlockMove);
	cTestContext.SetByte(0x00);
	cTestContext.SetByte(0x00);
	cTestContext.SetOpcd(STP_Implied);
	cTestContext.muiInstructions += 1;

	cTestContext.SetByte(0x00ff, 0x11);
	cTestContext.SetByte(0x0000, 0x22);
	cTestContext.SetByte(0x0100, 0x33);

	cTestContext.SetPrint(false, false, true, false, false, true, true, true, false, false, false, false, false, true);
	TestW65C816RunInContext(&cTestContext, ""\
		"LDA: A.0001  X.0000  Y.0000\n"\
		"LDX: A.0001  X.00ff  Y.0000\n"\
		"LDY: A.0001  X.00ff  Y.0040\n"\
		"MVN: A.0000  X.0000  Y.0041\n"\
		"MVN: A.00ff  X.0001  Y.0042\n");

	AssertByteHex((uint8)0x11, cTestContext.GetByte(0x0040));
	AssertByteHex((uint8)0x22, cTestContext.GetByte(0x0041));
	AssertByteHex((uint8)0xea, cTestContext.GetByte(0x0042));

	cTestContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816BlockMove(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();


	TestW65C816BlockMoveMVN();
	TestW65C816BlockMoveMVP();
	TestW65C816BlockMoveSameBank();
	TestW65C816BlockMoveEmulation();


	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

