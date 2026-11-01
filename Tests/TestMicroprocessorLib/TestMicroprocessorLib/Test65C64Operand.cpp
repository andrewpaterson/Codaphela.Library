#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "MicroprocessorLib/65C64InstructionFactory.h"
#include "MicroprocessorLib/65C64Meta.h"
#include "TestLib/Assert.h"
#include "Test65C64Context.h"


//////////////////////////////////////////////////////////////////////////
//
// Negate, absolute, push operand and pull into address.
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64NegateAbsolute(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65Wtth(R65_A, R65_B, 2));
	pcContext->Op(Op65RE(C65_CPY, R65_X, 5));
	pcContext->Op(Op65Neg(R65_A, R65_X));
	pcContext->Op(Op65Abs(R65_B, R65_A));
	pcContext->Op(Op65Abs(R65_Y, R65_X));
	pcContext->Op(Op65RE(C65_CPY, R65_X, -5));
	pcContext->Op(Op65Neg(R65_A, R65_X));
	pcContext->Op(Op65Neg(R65_A, R65_Zero));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));

	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xFFFBLL, pcMPU->GetA());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64Flags(&sTest) & (STATUS65_NEGATIVE | STATUS65_ZERO));

	//n says the source was negative.
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(5LL, pcMPU->GetB());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64Flags(&sTest) & (STATUS65_NEGATIVE | STATUS65_ZERO));
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(5LL, pcMPU->GetY());
	AssertShortHex((uint16)0, Test65C64Flags(&sTest) & (STATUS65_NEGATIVE | STATUS65_ZERO));

	//The 8 bit source is sign extended to A's 16 bits.
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(5LL, pcMPU->GetA());
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0LL, pcMPU->GetA());
	AssertShortHex((uint16)STATUS65_ZERO, Test65C64Flags(&sTest) & (STATUS65_NEGATIVE | STATUS65_ZERO));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64PushPullOperand(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetLong(0x2000, 0x11223344);
	pcContext->SetLong(0x3100, 0x3200);
	pcContext->Op(Op65PushOperand(R65_S, OPERAND_MODE_IMMEDIATE, 2), 0x1234);
	pcContext->OpLong(Op65PushOperand(R65_S, OPERAND_MODE_ABSOLUTE, R65_Zero), 0x2000);
	pcContext->OpLong(Op65PullOperand(R65_S, OPERAND_MODE_ABSOLUTE, R65_Zero), 0x3000);
	pcContext->OpLong(Op65PullOperand(R65_S, OPERAND_MODE_ABSOLUTE_INDIRECT, R65_Zero), 0x3100);
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	//An immediate is pushed at its own width.
	AssertSize(3, Test65C64Step(&sTest));
	AssertIntHex(0x0001FFFD, pcMPU->GetS());
	AssertShortHex((uint16)0x1234, pcContext->GetWord(0x1FFFE));

	//The 32 bit value at an address: read it, then push it.
	AssertSize(7, Test65C64Step(&sTest));
	AssertIntHex(0x0001FFF9, pcMPU->GetS());
	AssertIntHex(0x11223344, pcContext->GetLong(0x1FFFA));
	AssertTrue(pcContext->masCycles[3].bRead);
	AssertIntHex(0x2000, pcContext->masCycles[3].uiAddress);
	AssertFalse(pcContext->masCycles[5].bRead);
	AssertIntHex(0x1FFFA, pcContext->masCycles[5].uiAddress);

	//Pull 32 bits, then write them to the address.
	AssertSize(7, Test65C64Step(&sTest));
	AssertIntHex(0x0001FFFD, pcMPU->GetS());
	AssertIntHex(0x11223344, pcContext->GetLong(0x3000));
	AssertIntHex(0x1FFFA, pcContext->masCycles[3].uiAddress);
	AssertFalse(pcContext->masCycles[5].bRead);
	AssertIntHex(0x3000, pcContext->masCycles[5].uiAddress);

	//Indirect: the pointer, the pull, then the write.
	AssertSize(9, Test65C64Step(&sTest));
	AssertIntHex(0x00020001, pcMPU->GetS());
	AssertIntHex(0x00001234, pcContext->GetLong(0x3200));
	AssertIntHex(0x3100, pcContext->masCycles[3].uiAddress);
	AssertIntHex(0x1FFFE, pcContext->masCycles[5].uiAddress);
	AssertIntHex(0x3200, pcContext->masCycles[7].uiAddress);

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Operand(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	C65C64InstructionFactory::GetInstance()->Init();

	Test65C64NegateAbsolute();
	Test65C64PushPullOperand();

	C65C64InstructionFactory::GetInstance()->Kill();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

