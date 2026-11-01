#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "MicroprocessorLib/65C64InstructionFactory.h"
#include "MicroprocessorLib/65C64Meta.h"
#include "TestLib/Assert.h"
#include "Test65C64Context.h"


//////////////////////////////////////////////////////////////////////////
//
// Block operations: one element of R's width at V a step.  C stays on the opcode until
// A is negative after its decrement (or an allowed stop condition holds).
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Test65C64VNZ(STest65C64* psTest)
{
	return Test65C64MPU(psTest)->GetP() & (STATUS65_OVERFLOW | STATUS65_NEGATIVE | STATUS65_ZERO);
}


//////////////////////////////////////////////////////////////////////////
//
// A is the count so it cannot be R.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64BlockDecode(void)
{
	S65C64Decoded	sDecoded;

	Decode65C64(Op65Block(K65_AND, false, false, 0, R65_A), &sDecoded);
	AssertInt(I65_Illegal, sDecoded.eInstruction);
	Decode65C64(Op65Block(K65_AND, false, false, 0, R65_Zero), &sDecoded);
	AssertInt(I65_Illegal, sDecoded.eInstruction);
	Decode65C64(Op65Block(7, false, false, 0, R65_X), &sDecoded);
	AssertInt(I65_Illegal, sDecoded.eInstruction);
	Decode65C64(Op65Block(9, false, false, 0, R65_X), &sDecoded);
	AssertInt(I65_Illegal, sDecoded.eInstruction);
	Decode65C64(Op65Block(11, false, false, 0, R65_X), &sDecoded);
	AssertInt(I65_Illegal, sDecoded.eInstruction);
	Decode65C64(Op65Block(10, false, false, 0, R65_X), &sDecoded);
	AssertInt(I65_Bcmp, sDecoded.eInstruction);
	AssertInt(MA65_Read, sDecoded.eAccess);
	Decode65C64(Op65Block(K65_READ_COPY, false, false, 0, R65_X), &sDecoded);
	AssertInt(I65_Bpcpy, sDecoded.eInstruction);
	AssertInt(MA65_ReadWrite, sDecoded.eAccess);
	Decode65C64(Op65Block(0, false, false, 0, R65_X), &sDecoded);
	AssertInt(I65_Bcpy, sDecoded.eInstruction);
	AssertInt(MA65_Write, sDecoded.eAccess);
	Decode65C64(Op65Block(K65_AND, true, true, STOP65_NOT_NEGATIVE, R65_Y), &sDecoded);
	AssertInt(I65_Band, sDecoded.eInstruction);
	AssertInt(R65_Y, sDecoded.eSource);
	AssertTrue(sDecoded.bDecrement);
	AssertTrue(sDecoded.bAllowStop);
	AssertInt(STOP65_NOT_NEGATIVE, sDecoded.uiStopStyle);
}


//////////////////////////////////////////////////////////////////////////
//
// Four bytes ANDed with $0F.  A level interrupt between steps returns to the opcode.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64BlockAnd(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	uint32				uiBand;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetLong(0x2000, 0x78563412);
	pcContext->SetVector(VECTOR_65C64_LEVEL, 0x1200);
	pcContext->Op(Op65RI(C65_CPY, R65_B, 1), 0x0F);
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2000);
	pcContext->Op(Op65RE(C65_CPY, R65_A, 3));
	pcContext->Op(Op65Cep(STATUS65_LEVEL_DISABLE));
	uiBand = pcContext->Here();
	pcContext->Op(Op65Block(K65_AND, false, false, 0, R65_B));
	pcContext->Op(Op65Stp());
	pcContext->Org(0x1200);
	pcContext->Op(Op65Return(R65_S, true, true, true));

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));

	AssertSize(3, Test65C64Step(&sTest));
	AssertIntHex(uiBand, pcMPU->GetProgramCounter());
	AssertByteHex((uint8)0x02, pcContext->GetByte(0x2000));
	AssertIntHex(0x2001, pcMPU->GetV());
	AssertLongLongIntHex(2LL, pcMPU->GetA());
	AssertShortHex((uint16)0, Test65C64VNZ(&sTest));
	AssertTrue(pcContext->masCycles[1].bRead);
	AssertIntHex(0x2000, pcContext->masCycles[1].uiAddress);
	AssertTrue(pcContext->masCycles[1].bLower);
	AssertFalse(pcContext->masCycles[1].bUpper);
	AssertFalse(pcContext->masCycles[2].bRead);
	AssertIntHex(0x2000, pcContext->masCycles[2].uiAddress);

	//Interrupted after the second element; rti goes back to the opcode.
	sTest.cMeta.GetLIRQB()->Set(false);
	AssertSize(3 + 5, Test65C64Step(&sTest));
	sTest.cMeta.GetLIRQB()->Set(true);
	AssertIntHex(0x1200, pcMPU->GetProgramCounter());
	AssertIntHex(uiBand, pcContext->GetLong(0x1FFFC));
	AssertByteHex((uint8)0x04, pcContext->GetByte(0x2001));
	AssertSize(4, Test65C64Step(&sTest));
	AssertIntHex(uiBand, pcMPU->GetProgramCounter());

	AssertSize(3, Test65C64Step(&sTest));
	AssertIntHex(uiBand, pcMPU->GetProgramCounter());
	AssertSize(3, Test65C64Step(&sTest));
	AssertIntHex(uiBand + 2, pcMPU->GetProgramCounter());
	AssertIntHex(0x08060402, pcContext->GetLong(0x2000));
	AssertIntHex(0x2004, pcMPU->GetV());
	AssertLongLongIntHex(0xFFLL, pcMPU->GetA());
	AssertShortHex((uint16)STATUS65_OVERFLOW, Test65C64VNZ(&sTest));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// Stop on zero: the first element ANDs to zero.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64BlockAndStop(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	uint32				uiBand;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetLong(0x2100, 0x40032010);
	pcContext->Op(Op65RI(C65_CPY, R65_B, 1), 0x0F);
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2100);
	pcContext->Op(Op65RE(C65_CPY, R65_A, 3));
	uiBand = pcContext->Here();
	pcContext->Op(Op65Block(K65_AND, false, true, STOP65_ZERO, R65_B));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));

	AssertSize(3, Test65C64Step(&sTest));
	AssertIntHex(uiBand + 2, pcMPU->GetProgramCounter());
	AssertIntHex(0x40032000, pcContext->GetLong(0x2100));
	AssertIntHex(0x2101, pcMPU->GetV());
	AssertLongLongIntHex(2LL, pcMPU->GetA());
	AssertShortHex((uint16)STATUS65_ZERO, Test65C64VNZ(&sTest));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// 16 bit elements at an odd address, V decrementing.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64BlockAndDecrement(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	uint32				uiBand;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetWord(0x3005, 0x1234);
	pcContext->SetWord(0x3003, 0xABCD);
	pcContext->Op(Op65Wth(R65_B, 2));
	pcContext->Op(Op65RI(C65_CPY, R65_B, 2), 0xFF00);
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x3005);
	pcContext->Op(Op65RE(C65_CPY, R65_A, 1));
	uiBand = pcContext->Here();
	pcContext->Op(Op65Block(K65_AND, true, false, 0, R65_B));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));

	AssertSize(5, Test65C64Step(&sTest));
	AssertIntHex(uiBand, pcMPU->GetProgramCounter());
	AssertShortHex((uint16)0x1200, pcContext->GetWord(0x3005));
	AssertIntHex(0x3003, pcMPU->GetV());
	AssertIntHex(0x3004, pcContext->masCycles[1].uiAddress);
	AssertIntHex(0x3006, pcContext->masCycles[2].uiAddress);

	AssertSize(5, Test65C64Step(&sTest));
	AssertIntHex(uiBand + 2, pcMPU->GetProgramCounter());
	AssertShortHex((uint16)0xAB00, pcContext->GetWord(0x3003));
	AssertShortHex((uint16)0x1200, pcContext->GetWord(0x3005));
	AssertIntHex(0x3001, pcMPU->GetV());
	AssertShortHex((uint16)(STATUS65_OVERFLOW | STATUS65_NEGATIVE), Test65C64VNZ(&sTest));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// Block copy only writes: three 16 bit copies of X from an odd address.  n and z are
// not changed.  Its stop bits are ignored: with stop on not zero and z clear it still
// copies all four elements.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64BlockCopyRegister(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	uint32				uiBcpy;
	size				i;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetByte(0x2000, 0x55);
	pcContext->SetByte(0x2007, 0x55);
	pcContext->Op(Op65Wth(R65_X, 2));
	pcContext->Op(Op65RI(C65_CPY, R65_X, 2), 0xBEEF);
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2001);
	pcContext->Op(Op65RE(C65_CPY, R65_A, 2));
	pcContext->Op(Op65RR(C65_CPY, R65_Y, R65_Zero));
	uiBcpy = pcContext->Here();
	pcContext->Op(Op65Block(0, false, false, 0, R65_X));
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2100);
	pcContext->Op(Op65RE(C65_CPY, R65_A, 3));
	pcContext->Op(Op65Block(0, false, true, STOP65_NOT_ZERO, R65_X));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertTrue(pcMPU->IsFlagSet(STATUS65_ZERO));

	for (i = 0; i < 3; i++)
	{
		AssertSize(3, Test65C64Step(&sTest));
		AssertFalse(pcContext->masCycles[1].bRead);
		AssertFalse(pcContext->masCycles[2].bRead);
		AssertTrue(pcMPU->IsFlagSet(STATUS65_ZERO));
		AssertFalse(pcMPU->IsFlagSet(STATUS65_NEGATIVE));
	}
	AssertIntHex(uiBcpy + 2, pcMPU->GetProgramCounter());
	AssertIntHex(0x2007, pcMPU->GetV());
	AssertLongLongIntHex(0xFFLL, pcMPU->GetA());
	AssertTrue(pcMPU->IsFlagSet(STATUS65_OVERFLOW));
	AssertByteHex((uint8)0x55, pcContext->GetByte(0x2000));
	AssertIntHex(0xBEEFBEEF, pcContext->GetLong(0x2001));
	AssertShortHex((uint16)0xBEEF, pcContext->GetWord(0x2005));
	AssertByteHex((uint8)0x55, pcContext->GetByte(0x2007));

	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertFalse(pcMPU->IsFlagSet(STATUS65_ZERO));
	for (i = 0; i < 4; i++)
	{
		AssertSize(2, Test65C64Step(&sTest));
	}
	AssertIntHex(0xBEEFBEEF, pcContext->GetLong(0x2100));
	AssertIntHex(0xBEEFBEEF, pcContext->GetLong(0x2104));
	AssertIntHex(0x2108, pcMPU->GetV());
	AssertLongLongIntHex(0xFFLL, pcMPU->GetA());
	AssertTrue(pcMPU->IsFlagSet(STATUS65_OVERFLOW));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// Block precopy reads [V] first and sets n and z from it, then writes R.  Stop on zero
// ends the copy at the first zero read.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64BlockPrecopy(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	uint32				uiBpcpy;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetLong(0x2200, 0x09008705);
	pcContext->Op(Op65RI(C65_CPY, R65_X, 1), 0xEE);
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2200);
	pcContext->Op(Op65RE(C65_CPY, R65_A, 3));
	uiBpcpy = pcContext->Here();
	pcContext->Op(Op65Block(K65_READ_COPY, false, true, STOP65_ZERO, R65_X));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));

	AssertSize(3, Test65C64Step(&sTest));
	AssertTrue(pcContext->masCycles[1].bRead);
	AssertFalse(pcContext->masCycles[2].bRead);
	AssertIntHex(uiBpcpy, pcMPU->GetProgramCounter());
	AssertShortHex((uint16)0, Test65C64VNZ(&sTest));

	AssertSize(3, Test65C64Step(&sTest));
	AssertIntHex(uiBpcpy, pcMPU->GetProgramCounter());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64VNZ(&sTest));

	AssertSize(3, Test65C64Step(&sTest));
	AssertIntHex(uiBpcpy + 2, pcMPU->GetProgramCounter());
	AssertShortHex((uint16)STATUS65_ZERO, Test65C64VNZ(&sTest));
	AssertIntHex(0x09EEEEEE, pcContext->GetLong(0x2200));
	AssertIntHex(0x2203, pcMPU->GetV());
	AssertLongLongIntHex(0LL, pcMPU->GetA());

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// bpand, bpor and bpxor with R = X = $3C.  n and z come from the value read, the
// result of the operation is written.  bpxor stops on the first negative value read.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64BlockPreOperations(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	uint32				uiNext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetLong(0x2300, 0x00F0800F);
	pcContext->SetWord(0x2310, 0x1100);
	pcContext->SetLong(0x2320, 0x0002FF01);
	pcContext->Op(Op65RI(C65_CPY, R65_X, 1), 0x3C);
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2300);
	pcContext->Op(Op65RE(C65_CPY, R65_A, 2));
	pcContext->Op(Op65Block(13, false, false, 0, R65_X));
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2310);
	pcContext->Op(Op65RE(C65_CPY, R65_A, 1));
	pcContext->Op(Op65Block(14, false, false, 0, R65_X));
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2320);
	pcContext->Op(Op65RE(C65_CPY, R65_A, 2));
	uiNext = pcContext->Here() + 2;
	pcContext->Op(Op65Block(15, false, true, STOP65_NEGATIVE, R65_X));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));

	//bpand.
	AssertSize(3, Test65C64Step(&sTest));
	AssertShortHex((uint16)0, Test65C64VNZ(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64VNZ(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertShortHex((uint16)(STATUS65_OVERFLOW | STATUS65_NEGATIVE), Test65C64VNZ(&sTest));
	AssertIntHex(0x0030000C, pcContext->GetLong(0x2300));

	//bpor.
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertShortHex((uint16)STATUS65_ZERO, Test65C64VNZ(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertShortHex((uint16)STATUS65_OVERFLOW, Test65C64VNZ(&sTest));
	AssertShortHex((uint16)0x3D3C, pcContext->GetWord(0x2310));

	//bpxor stops after reading $FF.
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertIntHex(uiNext, pcMPU->GetProgramCounter());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64VNZ(&sTest));
	AssertIntHex(0x0002C33D, pcContext->GetLong(0x2320));
	AssertLongLongIntHex(0LL, pcMPU->GetA());

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// Block test, or, xor, add, subtract and compare, each over two elements (compare over
// four, stopping at the first equal one).  Test and compare do not write.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64BlockOperations(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	uint32				uiNext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetWord(0x2400, 0x01F0);
	pcContext->SetWord(0x2410, 0xF010);
	pcContext->SetWord(0x2420, 0xFF0F);
	pcContext->SetLong(0x2430, 0x1234FFFF);
	pcContext->SetWord(0x2440, 0x0510);
	pcContext->SetLong(0x2450, 0x030F0201);
	pcContext->Op(Op65RI(C65_CPY, R65_X, 1), 0x0F);
	pcContext->Op(Op65Wth(R65_B, 2));
	pcContext->Op(Op65RE(C65_CPY, R65_B, 1));

	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2400);
	pcContext->Op(Op65RE(C65_CPY, R65_A, 1));
	pcContext->Op(Op65Block(3, false, false, 0, R65_X));
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2410);
	pcContext->Op(Op65RE(C65_CPY, R65_A, 1));
	pcContext->Op(Op65Block(4, false, false, 0, R65_X));
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2420);
	pcContext->Op(Op65RE(C65_CPY, R65_A, 1));
	pcContext->Op(Op65Block(5, false, false, 0, R65_X));
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2430);
	pcContext->Op(Op65RE(C65_CPY, R65_A, 1));
	pcContext->Op(Op65Block(6, false, false, 0, R65_B));
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2440);
	pcContext->Op(Op65RE(C65_CPY, R65_A, 1));
	pcContext->Op(Op65Block(8, false, false, 0, R65_X));
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), 0x2450);
	pcContext->Op(Op65RE(C65_CPY, R65_A, 3));
	uiNext = pcContext->Here() + 2;
	pcContext->Op(Op65Block(10, false, true, STOP65_ZERO, R65_X));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));

	//btst: $F0 and $0F is 0, $01 and $0F is not.  No writes.
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertTrue(pcContext->masCycles[1].bRead);
	AssertShortHex((uint16)STATUS65_ZERO, Test65C64VNZ(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertShortHex((uint16)STATUS65_OVERFLOW, Test65C64VNZ(&sTest));
	AssertShortHex((uint16)0x01F0, pcContext->GetWord(0x2400));

	//bor.
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertShortHex((uint16)(STATUS65_OVERFLOW | STATUS65_NEGATIVE), Test65C64VNZ(&sTest));
	AssertShortHex((uint16)0xFF1F, pcContext->GetWord(0x2410));

	//bxor.
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertShortHex((uint16)STATUS65_ZERO, Test65C64VNZ(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertShortHex((uint16)0xF000, pcContext->GetWord(0x2420));

	//badd, 16 bit elements (B is 16 bits).
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertShortHex((uint16)STATUS65_ZERO, Test65C64VNZ(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertIntHex(0x12350000, pcContext->GetLong(0x2430));
	AssertIntHex(0x2434, pcMPU->GetV());

	//bsub: [V] - R.
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertShortHex((uint16)(STATUS65_OVERFLOW | STATUS65_NEGATIVE), Test65C64VNZ(&sTest));
	AssertShortHex((uint16)0xF601, pcContext->GetWord(0x2440));

	//bcmp stops at the element equal to R and writes nothing.
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertIntHex(uiNext, pcMPU->GetProgramCounter());
	AssertShortHex((uint16)STATUS65_ZERO, Test65C64VNZ(&sTest));
	AssertIntHex(0x030F0201, pcContext->GetLong(0x2450));
	AssertIntHex(0x2453, pcMPU->GetV());
	AssertLongLongIntHex(0LL, pcMPU->GetA());

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// The two programs on the spreadsheet's Example Block Move sheet, as written there.
// W, V and A are pulled from an aligned stack.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64BlockMoveSheet(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	size				i;
	size				uiCycles;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	for (i = 0; i < 16; i++)
	{
		pcContext->SetByte((uint32)(0x4000 + i), (uint8)(0x30 + i));
		pcContext->SetByte((uint32)(0x5000 + i), (uint8)(0x60 + i));
	}

	//Example 1: V is the source, A the destination, W the byte count (from offset W down to 8).
	pcContext->SetLong(0x3000, 16);
	pcContext->SetLong(0x3004, 0x4000 - 8);
	pcContext->SetLong(0x3008, 0x4800 - 8);
	//Example 2: W is the source, V the destination, A + 1 is the number of 64 bit values.
	pcContext->SetLong(0x3010, 0x5000);
	pcContext->SetLong(0x3014, 0x5800);
	pcContext->SetLong(0x3018, 1);

	pcContext->Op(Op65RI(C65_CPY, R65_S, 2), 0x2FFF);
	pcContext->Op(Op65Wth(R65_A, 4));
	pcContext->Op(Op65Wth(R65_B, 8));
	pcContext->Op(Op65Pull(R65_S, M65_A | M65_V | M65_W));
	pcContext->Op(Op65M2(C65_CPY, false, R65_B, OPERAND_MODE_REGISTER_REGISTER, R65_V, R65_W));
	pcContext->Op(Op65M2(C65_CPY, true, R65_B, OPERAND_MODE_REGISTER_REGISTER, R65_A, R65_W));
	pcContext->Op(Op65RE(C65_SUB, R65_W, 8));
	pcContext->Op(Op65Branch(B65_CLEAR, F65_Z, 0, -8));

	pcContext->Op(Op65RI(C65_CPY, R65_S, 2), 0x300F);
	pcContext->Op(Op65Wtth(R65_X, R65_Y, 4));
	pcContext->Op(Op65Wth(R65_A, 4));
	pcContext->Op(Op65Pull(R65_S, M65_A | M65_V | M65_W));
	pcContext->Op(Op65Vector(0, false, false, 0, false, WIDTH65_64));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(7, Test65C64Step(&sTest));
	AssertIntHex(16, pcMPU->GetW());
	AssertIntHex(0x4000 - 8, pcMPU->GetV());
	AssertLongLongIntHex(0x4800 - 8, pcMPU->GetA());
	for (i = 0; i < 2; i++)
	{
		AssertSize(5, Test65C64Step(&sTest));
		AssertSize(5, Test65C64Step(&sTest));
		AssertSize(1, Test65C64Step(&sTest));
		AssertSize(1, Test65C64Step(&sTest));
	}
	for (i = 0; i < 16; i++)
	{
		AssertByteHex((uint8)(0x30 + i), pcContext->GetByte((uint32)(0x4800 + i)));
	}

	AssertSize(2, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(7, Test65C64Step(&sTest));
	uiCycles = Test65C64Step(&sTest);
	uiCycles += Test65C64Step(&sTest);
	AssertSize(18, uiCycles);
	for (i = 0; i < 16; i++)
	{
		AssertByteHex((uint8)(0x60 + i), pcContext->GetByte((uint32)(0x5800 + i)));
	}
	AssertTrue(pcMPU->IsFlagSet(STATUS65_OVERFLOW));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Block(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	C65C64InstructionFactory::GetInstance()->Init();

	Test65C64BlockDecode();
	Test65C64BlockAnd();
	Test65C64BlockAndStop();
	Test65C64BlockAndDecrement();
	Test65C64BlockCopyRegister();
	Test65C64BlockPrecopy();
	Test65C64BlockPreOperations();
	Test65C64BlockOperations();
	Test65C64BlockMoveSheet();

	C65C64InstructionFactory::GetInstance()->Kill();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

