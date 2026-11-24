#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "MicroprocessorLib/65C64InstructionFactory.h"
#include "MicroprocessorLib/65C64Meta.h"
#include "TestLib/Assert.h"
#include "Test65C64Context.h"


//////////////////////////////////////////////////////////////////////////
//
// Find, bit to status, status to bit and status to status.
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64AssertFind(STest65C64* psTest, uint64 uiIndex, bool bNoMore)
{
	C65C64*	pcMPU;

	pcMPU = Test65C64MPU(psTest);
	AssertSize(1, Test65C64Step(psTest));
	AssertLongLongIntHex((int64)uiIndex, pcMPU->GetX());
	AssertBool(bNoMore, pcMPU->IsFlagSet(STATUS65_OVERFLOW));
	AssertTrue(pcMPU->IsFlagSet(STATUS65_CARRY));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Find(void)
{
	STest65C64			sTest;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65Wth(R65_A, 8));
	pcContext->OpQuad(Op65RI(C65_CPY, R65_A, 8), 0x8000000000001010ULL);

	//Find set bits from bit 0.  c then moves the index on before each find.
	pcContext->Op(Op65Find(true, U65_BIT, R65_A, R65_X, false));
	pcContext->Op(Op65Find(true, U65_BIT, R65_A, R65_X, false));
	pcContext->Op(Op65Find(true, U65_BIT, R65_A, R65_X, false));
	pcContext->Op(Op65Find(true, U65_BIT, R65_A, R65_X, false));

	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65RR(C65_CPY, R65_X, R65_Zero));
	pcContext->Op(Op65Find(false, U65_BIT, R65_A, R65_X, false));
	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65RR(C65_CPY, R65_X, R65_Zero));
	pcContext->Op(Op65Find(true, U65_NYBBLE, R65_A, R65_X, false));
	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65RR(C65_CPY, R65_X, R65_Zero));
	pcContext->Op(Op65Find(true, U65_CRUMB, R65_A, R65_X, false));

	//Bytes downwards from byte 7.
	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65RE(C65_CPY, R65_X, 7));
	pcContext->Op(Op65Find(true, U65_BYTE, R65_A, R65_X, true));
	pcContext->Op(Op65Find(true, U65_BYTE, R65_A, R65_X, true));
	pcContext->Op(Op65Find(true, U65_BYTE, R65_A, R65_X, true));
	pcContext->Op(Op65Find(true, U65_BYTE, R65_A, R65_X, true));

	//A zero nybble is one with any bit clear.
	pcContext->Op(Op65Wth(R65_B, 4));
	pcContext->OpLong(Op65RI(C65_CPY, R65_B, 4), 0xFFFF0FFF);
	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65RR(C65_CPY, R65_X, R65_Zero));
	pcContext->Op(Op65Find(false, U65_NYBBLE, R65_B, R65_X, false));

	//Only the width of the scanned register is scanned.
	pcContext->Op(Op65Wth(R65_A, 1));
	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65RE(C65_CPY, R65_X, 5));
	pcContext->Op(Op65Find(true, U65_BIT, R65_A, R65_X, false));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(5, Test65C64Step(&sTest));

	Test65C64AssertFind(&sTest, 4, false);
	Test65C64AssertFind(&sTest, 12, false);
	Test65C64AssertFind(&sTest, 63, false);
	Test65C64AssertFind(&sTest, 64, true);

	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64AssertFind(&sTest, 0, false);
	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64AssertFind(&sTest, 1, false);
	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64AssertFind(&sTest, 2, false);

	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64AssertFind(&sTest, 7, false);
	Test65C64AssertFind(&sTest, 1, false);
	Test65C64AssertFind(&sTest, 0, false);
	Test65C64AssertFind(&sTest, 0xFF, true);

	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64AssertFind(&sTest, 3, false);

	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64AssertFind(&sTest, 8, true);

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// The all-bits finds: fz* stop at a unit with every bit clear, ff* at one with every
// bit set, scanning upward.  A is 0x11000022FF0F0033: bytes 33 00 0F FF 22 00 00 11.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64FindAll(void)
{
	STest65C64			sTest;
	CTest65C64Context*	pcContext;
	S65C64Decoded		sDecoded;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65Wth(R65_A, 8));
	pcContext->OpQuad(Op65RI(C65_CPY, R65_A, 8), 0x11000022FF0F0033ULL);

	//Zero bytes from byte 0, c moving the index on: 1, 5, 6, then none (8, v set).
	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65RR(C65_CPY, R65_X, R65_Zero));
	pcContext->Op(Op65FindAll(false, U65_BYTE, R65_A, R65_X));
	pcContext->Op(Op65FindAll(false, U65_BYTE, R65_A, R65_X));
	pcContext->Op(Op65FindAll(false, U65_BYTE, R65_A, R65_X));
	pcContext->Op(Op65FindAll(false, U65_BYTE, R65_A, R65_X));

	//The first full byte is byte 3 (FF); the first zero nybble is nybble 2 (byte 1), the
	//first full one nybble 4 (byte 2's F); crumb 1 (byte 0's 00) is zero, crumb 0 (11) full.
	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65RR(C65_CPY, R65_X, R65_Zero));
	pcContext->Op(Op65FindAll(true, U65_BYTE, R65_A, R65_X));
	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65RR(C65_CPY, R65_X, R65_Zero));
	pcContext->Op(Op65FindAll(false, U65_NYBBLE, R65_A, R65_X));
	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65RR(C65_CPY, R65_X, R65_Zero));
	pcContext->Op(Op65FindAll(true, U65_NYBBLE, R65_A, R65_X));
	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65RR(C65_CPY, R65_X, R65_Zero));
	pcContext->Op(Op65FindAll(false, U65_CRUMB, R65_A, R65_X));
	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65RR(C65_CPY, R65_X, R65_Zero));
	pcContext->Op(Op65FindAll(true, U65_CRUMB, R65_A, R65_X));

	//Unlike fcny (any bit clear: nybble 0, a 3), fzny wants every bit clear.
	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65RR(C65_CPY, R65_X, R65_Zero));
	pcContext->Op(Op65Find(false, U65_NYBBLE, R65_A, R65_X, false));

	//Only the width of the scanned register is scanned: one byte, 33, has no zero byte.
	pcContext->Op(Op65Wth(R65_A, 1));
	pcContext->Op(Op65Cep(STATUS65_CARRY));
	pcContext->Op(Op65RR(C65_CPY, R65_X, R65_Zero));
	pcContext->Op(Op65FindAll(false, U65_BYTE, R65_A, R65_X));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(5, Test65C64Step(&sTest));

	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64AssertFind(&sTest, 1, false);
	Test65C64AssertFind(&sTest, 5, false);
	Test65C64AssertFind(&sTest, 6, false);
	Test65C64AssertFind(&sTest, 8, true);

	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64AssertFind(&sTest, 3, false);
	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64AssertFind(&sTest, 2, false);
	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64AssertFind(&sTest, 4, false);
	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64AssertFind(&sTest, 1, false);
	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64AssertFind(&sTest, 0, false);

	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64AssertFind(&sTest, 0, false);

	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64Step(&sTest);
	Test65C64AssertFind(&sTest, 1, true);

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);

	//A unit of 0 leaves multiply and divide as they were; 1 to 3 are the finds.
	Decode65C64(Op65FindAll(false, 0, R65_Zero, R65_Zero), &sDecoded);
	AssertInt(I65_Mul, sDecoded.eInstruction);
	Decode65C64(Op65FindAll(true, 0, R65_Zero, R65_Zero), &sDecoded);
	AssertInt(I65_Div, sDecoded.eInstruction);
	Decode65C64(Op65FindAll(false, U65_CRUMB, R65_A, R65_X), &sDecoded);
	AssertInt(I65_Fzcr, sDecoded.eInstruction);
	AssertString("fzcr", Get65C64InstructionName(sDecoded.eInstruction));
	Decode65C64(Op65FindAll(true, U65_BYTE, R65_B, R65_Y), &sDecoded);
	AssertInt(I65_Ffby, sDecoded.eInstruction);
	AssertInt(R65_B, sDecoded.eSource);
	AssertInt(R65_Y, sDecoded.eCount);
	AssertString("ffby", Get65C64InstructionName(sDecoded.eInstruction));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64BitStatus(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65Wth(R65_A, 8));
	pcContext->OpQuad(Op65RI(C65_CPY, R65_A, 8), 0x8000000000001010ULL);
	pcContext->Op(Op65Bist(F65_C, R65_A, X65_INDEX_IMMEDIATE), 63 << 10);
	pcContext->Op(Op65Bist(F65_Z, R65_A, 4));
	pcContext->Op(Op65Bist(F65_V, R65_A, 3));
	pcContext->Op(Op65RE(C65_CPY, R65_X, 12));
	pcContext->Op(Op65Bist(F65_G, R65_A, X65_INDEX_REGISTER), R65_X << 13);
	pcContext->Op(Op65Stat(F65_F, F65_G));
	pcContext->Op(Op65Stat(F65_G, F65_FALSE));
	pcContext->Op(Op65Stat(F65_N, F65_TRUE));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(5, Test65C64Step(&sTest));

	AssertSize(2, Test65C64Step(&sTest));
	AssertTrue(pcMPU->IsFlagSet(STATUS65_CARRY));
	AssertSize(1, Test65C64Step(&sTest));
	AssertTrue(pcMPU->IsFlagSet(STATUS65_ZERO));
	AssertSize(1, Test65C64Step(&sTest));
	AssertFalse(pcMPU->IsFlagSet(STATUS65_OVERFLOW));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertTrue(pcMPU->IsFlagSet(STATUS65_FLAG_G));

	AssertSize(1, Test65C64Step(&sTest));
	AssertTrue(pcMPU->IsFlagSet(STATUS65_FLAG_F));
	AssertSize(1, Test65C64Step(&sTest));
	AssertFalse(pcMPU->IsFlagSet(STATUS65_FLAG_G));
	AssertSize(1, Test65C64Step(&sTest));
	AssertTrue(pcMPU->IsFlagSet(STATUS65_NEGATIVE));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// The Arithmetic Shift sheet: shr A, X; gwth Y, (A, -1); stab (A, Y, X, -1), n.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64StatusToBit(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65Wth(R65_A, 4));
	pcContext->OpLong(Op65RI(C65_CPY, R65_A, 4), 0x80001234);
	pcContext->Op(Op65RE(C65_CPY, R65_X, 4));
	pcContext->Op(Op65ShiftRegister(false, R65_A, R65_X));
	pcContext->Op(Op65Gwth(R65_A, true, R65_Y));
	pcContext->Op(Op65Stab(F65_N, R65_A, X65_INDEX_REGISTER), (R65_Y << 13) | (2 << 8) | (R65_X << 5));

	//40 bits from bit 0, then clear bit 5 (an embedded index, no repeat).
	pcContext->Op(Op65Wth(R65_B, 8));
	pcContext->Op(Op65Stab(F65_TRUE, R65_B, X65_INDEX_IMMEDIATE), (0 << 10) | (1 << 8) | (40 << 2));
	pcContext->Op(Op65Stab(F65_FALSE, R65_B, 5));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));

	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x08000123LL, pcMPU->GetA());
	AssertTrue(pcMPU->IsFlagSet(STATUS65_NEGATIVE));
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(31LL, pcMPU->GetY());
	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xF8000123LL, pcMPU->GetA());

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xFFFFFFFFFFLL, pcMPU->GetB());
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(0xFFFFFFFFDFLL, pcMPU->GetB());

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Bits(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	C65C64InstructionFactory::GetInstance()->Init();

	Test65C64Find();
	Test65C64FindAll();
	Test65C64BitStatus();
	Test65C64StatusToBit();

	C65C64InstructionFactory::GetInstance()->Kill();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

