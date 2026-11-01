#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "MicroprocessorLib/65C64InstructionFactory.h"
#include "MicroprocessorLib/65C64Meta.h"
#include "TestLib/Assert.h"
#include "Test65C64Context.h"


//////////////////////////////////////////////////////////////////////////
//
// Partial multiply (a byte of B a step) and divide (a nybble of Y a step).  Each step
// is one cycle.  C stays on the opcode until the last step so the instruction repeats.
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Test65C64VZ(STest65C64* psTest)
{
	return Test65C64MPU(psTest)->GetP() & (STATUS65_OVERFLOW | STATUS65_ZERO);
}


//////////////////////////////////////////////////////////////////////////
//
// The Multiplication sheet's example: $6A8 x $7C = $33960.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64MultiplySheetExample(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	uint32				uiMul;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65Wtth(R65_A, R65_B, 2));
	pcContext->Op(Op65Wth(R65_Y, 4));
	pcContext->Op(Op65RI(C65_CPY, R65_A, 2), 0x06A8);
	pcContext->Op(Op65RI(C65_CPY, R65_B, 1), 0x7C);
	pcContext->Op(Op65RE(C65_CPY, R65_Y, -1));
	pcContext->Op(Op65RR(C65_CPY, R65_W, R65_Zero));
	uiMul = pcContext->Here();
	pcContext->Op(Op65Mul());
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));

	//The first step clears Y.
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x33960LL, pcMPU->GetY());
	AssertIntHex(2, pcMPU->GetW());
	AssertIntHex(uiMul, pcMPU->GetProgramCounter());
	AssertShortHex((uint16)0, Test65C64VZ(&sTest));

	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x33960LL, pcMPU->GetY());
	AssertIntHex(4, pcMPU->GetW());
	AssertIntHex(uiMul + 2, pcMPU->GetProgramCounter());
	AssertShortHex((uint16)STATUS65_OVERFLOW, Test65C64VZ(&sTest));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// A 64 bit multiply interrupted after its third step continues where it left off.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64MultiplyInterrupted(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	uint32				uiMul;
	size				i;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetVector(VECTOR_65C64_LEVEL, 0x1200);
	pcContext->Op(Op65Wtth(R65_A, R65_B, 8));
	pcContext->Op(Op65Wth(R65_Y, 8));
	pcContext->OpQuad(Op65RI(C65_CPY, R65_A, 8), 0x123456789ULL);
	pcContext->OpQuad(Op65RI(C65_CPY, R65_B, 8), 0xFEDCBA987ULL);
	pcContext->Op(Op65RR(C65_CPY, R65_W, R65_Zero));
	pcContext->Op(Op65Cep(STATUS65_LEVEL_DISABLE));
	uiMul = pcContext->Here();
	pcContext->Op(Op65Mul());
	pcContext->Op(Op65Stp());
	pcContext->Org(0x1200);
	pcContext->Op(Op65Return(R65_S, true, true, true));

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(5, Test65C64Step(&sTest));
	AssertSize(5, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertIntHex(uiMul, pcMPU->GetProgramCounter());

	sTest.cMeta.GetLIRQB()->Set(false);
	AssertSize(6, Test65C64Step(&sTest));
	sTest.cMeta.GetLIRQB()->Set(true);
	AssertIntHex(0x1200, pcMPU->GetProgramCounter());
	AssertIntHex(uiMul, pcContext->GetLong(0x1FFFC));
	AssertLongLongIntHex(0xE7B8EB84960A3FLL, pcMPU->GetY());
	AssertIntHex(6, pcMPU->GetW());

	AssertSize(4, Test65C64Step(&sTest));
	AssertIntHex(uiMul, pcMPU->GetProgramCounter());

	for (i = 0; i < 4; i++)
	{
		AssertSize(1, Test65C64Step(&sTest));
		AssertIntHex(uiMul, pcMPU->GetProgramCounter());
		AssertShortHex((uint16)0, Test65C64VZ(&sTest));
	}
	AssertSize(1, Test65C64Step(&sTest));
	AssertIntHex(uiMul + 2, pcMPU->GetProgramCounter());
	AssertIntHex(16, pcMPU->GetW());
	AssertLongLongIntHex(0x1FA00ACC59960A3FLL, pcMPU->GetY());
	AssertShortHex((uint16)STATUS65_OVERFLOW, Test65C64VZ(&sTest));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// Divides the 8 bit uiDividend by uiDivisor: 2 steps.  B is the remainder, W counts.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Divide8(uint8 uiDividend, uint8 uiDivisor, uint8 uiQuotient, uint8 uiRemainder)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	uint32				uiDiv;
	uint16				uiFlags;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65RI(C65_CPY, R65_Y, 1), uiDividend);
	pcContext->Op(Op65RI(C65_CPY, R65_A, 1), uiDivisor);
	pcContext->Op(Op65RR(C65_CPY, R65_W, R65_Zero));
	pcContext->Op(Op65RR(C65_CPY, R65_B, R65_Zero));
	uiDiv = pcContext->Here();
	pcContext->Op(Op65Div());
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));

	AssertSize(1, Test65C64Step(&sTest));
	AssertIntHex(uiDiv, pcMPU->GetProgramCounter());
	AssertIntHex(1, pcMPU->GetW());
	AssertFalse(pcMPU->IsFlagSet(STATUS65_OVERFLOW));

	AssertSize(1, Test65C64Step(&sTest));
	AssertIntHex(uiDiv + 2, pcMPU->GetProgramCounter());
	AssertIntHex(2, pcMPU->GetW());
	AssertLongLongIntHex((int64)uiQuotient, pcMPU->GetY());
	AssertLongLongIntHex((int64)uiRemainder, pcMPU->GetB());

	uiFlags = STATUS65_OVERFLOW;
	if (uiQuotient == 0)
	{
		uiFlags |= STATUS65_ZERO;
	}
	if (uiRemainder != 0)
	{
		uiFlags |= STATUS65_CARRY;
	}
	AssertShortHex(uiFlags, (uint16)(pcMPU->GetP() & (STATUS65_OVERFLOW | STATUS65_ZERO | STATUS65_CARRY)));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Divide64(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	uint32				uiDiv;
	size				i;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65Wth(R65_Y, 8));
	pcContext->Op(Op65Wth(R65_A, 4));
	pcContext->Op(Op65Wth(R65_B, 4));
	pcContext->OpQuad(Op65RI(C65_CPY, R65_Y, 8), 0xFEDCBA9876543210ULL);
	pcContext->OpLong(Op65RI(C65_CPY, R65_A, 4), 0x12345);
	pcContext->Op(Op65RR(C65_CPY, R65_W, R65_Zero));
	pcContext->Op(Op65RR(C65_CPY, R65_B, R65_Zero));
	uiDiv = pcContext->Here();
	pcContext->Op(Op65Div());
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(5, Test65C64Step(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));

	for (i = 0; i < 15; i++)
	{
		AssertSize(1, Test65C64Step(&sTest));
		AssertIntHex(uiDiv, pcMPU->GetProgramCounter());
	}
	AssertSize(1, Test65C64Step(&sTest));
	AssertIntHex(uiDiv + 2, pcMPU->GetProgramCounter());
	AssertLongLongIntHex(0xE0004FA01C4DLL, pcMPU->GetY());
	AssertLongLongIntHex(0x10A4FLL, pcMPU->GetB());
	AssertIntHex(16, pcMPU->GetW());

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// Divides uiDividend by uiDivisor, Y and A uiYBytes wide.  The remainder is truncated
// to B's width (uiBBytes) every step.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64DivideWidth(uint8 uiYBytes, uint8 uiBBytes, uint64 uiDividend, uint64 uiDivisor, uint64 uiQuotient, uint64 uiRemainder)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	size				i;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65Wth(R65_Y, uiYBytes));
	pcContext->Op(Op65Wth(R65_A, uiYBytes));
	pcContext->Op(Op65Wth(R65_B, uiBBytes));
	pcContext->OpQuad(Op65RI(C65_CPY, R65_Y, 8), uiDividend);
	pcContext->OpQuad(Op65RI(C65_CPY, R65_A, 8), uiDivisor);
	pcContext->Op(Op65RR(C65_CPY, R65_W, R65_Zero));
	pcContext->Op(Op65RR(C65_CPY, R65_B, R65_Zero));
	pcContext->Op(Op65Div());
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	for (i = 0; i < 7; i++)
	{
		Test65C64Step(&sTest);
	}
	for (i = 0; i < (size)(2 * uiYBytes); i++)
	{
		AssertSize(1, Test65C64Step(&sTest));
	}
	AssertTrue(pcMPU->IsFlagSet(STATUS65_OVERFLOW));
	AssertLongLongIntHex((int64)uiQuotient, pcMPU->GetY() & Mask65(uiYBytes));
	AssertLongLongIntHex((int64)uiRemainder, pcMPU->GetB() & Mask65(uiBBytes));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// A B that is not 0 at the start is the top half of the dividend: (B 2^64 + Y) / A, the
// quotient in Y and the remainder in B, as long as B is below A (the quotient fits).
//
//////////////////////////////////////////////////////////////////////////
void Test65C64DivideHighB(uint64 uiHigh, uint64 uiLow, uint64 uiDivisor, uint64 uiQuotient, uint64 uiRemainder)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	size				i;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65Wth(R65_Y, 8));
	pcContext->Op(Op65Wth(R65_A, 8));
	pcContext->Op(Op65Wth(R65_B, 8));
	pcContext->OpQuad(Op65RI(C65_CPY, R65_Y, 8), uiLow);
	pcContext->OpQuad(Op65RI(C65_CPY, R65_A, 8), uiDivisor);
	pcContext->Op(Op65RR(C65_CPY, R65_W, R65_Zero));
	pcContext->OpQuad(Op65RI(C65_CPY, R65_B, 8), uiHigh);
	pcContext->Op(Op65Div());
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	for (i = 0; i < 7; i++)
	{
		Test65C64Step(&sTest);
	}
	for (i = 0; i < 16; i++)
	{
		AssertSize(1, Test65C64Step(&sTest));
	}
	AssertTrue(pcMPU->IsFlagSet(STATUS65_OVERFLOW));
	AssertLongLongIntHex((int64)uiQuotient, pcMPU->GetY());
	AssertLongLongIntHex((int64)uiRemainder, pcMPU->GetB());

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64MultiplyDivide(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	C65C64InstructionFactory::GetInstance()->Init();

	Test65C64MultiplySheetExample();
	Test65C64MultiplyInterrupted();
	Test65C64Divide8(20, 3, 6, 2);
	Test65C64Divide8(16, 4, 4, 0);
	Test65C64Divide8(2, 5, 0, 2);
	Test65C64Divide8(0xFF, 0x10, 0x0F, 0x0F);
	Test65C64Divide8(0x5A, 0, 0xFF, 0x5A);
	Test65C64Divide64();

	//$FFFF / $2AB is $5F remainder $28A, but an 8 bit B truncates the remainder every step.
	Test65C64DivideWidth(2, 2, 0xFFFF, 0x2AB, 0x5F, 0x28A);
	Test65C64DivideWidth(2, 1, 0xFFFF, 0x2AB, 0x53, 0x8E);

	//A 64 bit remainder shifted 4 bits needs 68 bits.
	Test65C64DivideWidth(8, 8, 0xFFFFFFFFFFFFFFFFULL, 0xF000000000000001ULL, 1, 0x0FFFFFFFFFFFFFFEULL);

	//128 by 64 bits, with the dividend's top half in B.
	Test65C64DivideHighB(0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL, 0x0FEDCBA987654321ULL, 0x1249249249249247ULL, 0x0EB5B8284F51C1E9ULL);
	Test65C64DivideHighB(0x000FFFFFFFFFFFFEULL, 0x8000000000000000ULL, 0x0010000000000001ULL, 0xFFFFFFFFFFFFD800ULL, 0x2800ULL);
	Test65C64DivideHighB(0x000FFFFFFFFFFFFFULL, 0, 0x0010000000000000ULL, 0xFFFFFFFFFFFFF000ULL, 0);

	C65C64InstructionFactory::GetInstance()->Kill();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

