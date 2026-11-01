#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "MicroprocessorLib/65C64InstructionFactory.h"
#include "MicroprocessorLib/65C64Meta.h"
#include "TestLib/Assert.h"
#include "Test65C64Context.h"


//////////////////////////////////////////////////////////////////////////
//
// Subcopy bytes (scp) and bits (scb).  Two cycles: the opcode and the index word.
//
//////////////////////////////////////////////////////////////////////////


#define TEST_SUBCOPY_A	0x1122334455667788ULL


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Test65C64NZ(STest65C64* psTest)
{
	return Test65C64MPU(psTest)->GetP() & (STATUS65_NEGATIVE | STATUS65_ZERO);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64SubcopyBytes(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65Wtth(R65_A, R65_B, 8));
	pcContext->OpQuad(Op65RI(C65_CPY, R65_A, 8), TEST_SUBCOPY_A);
	pcContext->Op(Op65Subcopy(false, R65_A, R65_X, false, 3), Op65SubcopyIndices(false, false, 2, false, 5));
	pcContext->Op(Op65Subcopy(false, R65_A, R65_X, false, 2), Op65SubcopyIndices(false, false, 0, false, 0));
	pcContext->Op(Op65RE(C65_CPY, R65_V, 6));
	pcContext->Op(Op65RE(C65_CPY, R65_Y, 2));
	pcContext->Op(Op65Subcopy(false, R65_A, R65_B, true, R65_Y), Op65SubcopyIndices(false, true, R65_V, false, 0));
	pcContext->Op(Op65RE(C65_CPY, R65_B, -1));
	pcContext->Op(Op65Subcopy(false, R65_A, R65_B, false, 4), Op65SubcopyIndices(false, false, 6, false, 0));
	pcContext->Op(Op65RR(C65_CPY, R65_B, R65_Zero));
	pcContext->Op(Op65Subcopy(false, R65_A, R65_B, false, 4), Op65SubcopyIndices(false, false, 0, false, 6));
	pcContext->Op(Op65Subcopy(false, R65_A, R65_A, false, 1), Op65SubcopyIndices(false, false, 0, false, 1));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(5, Test65C64Step(&sTest));

	//X is 8 bits wide but subcopy ignores the width.  n and z come from X's width.
	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x4455660000000000LL, pcMPU->GetX());
	AssertShortHex((uint16)STATUS65_ZERO, Test65C64NZ(&sTest));
	AssertTrue(pcContext->masCycles[1].bVPA);

	//Like tax.
	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x4455660000007788LL, pcMPU->GetX());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64NZ(&sTest));

	//Count in Y, source index in V.
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x1122LL, pcMPU->GetB());
	AssertShortHex((uint16)0, Test65C64NZ(&sTest));

	//Source bytes past byte 7 read as 0.
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex((int64)0xFFFFFFFF00001122ULL, pcMPU->GetB());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64NZ(&sTest));

	//Destination bytes past byte 7 are dropped.
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x7788000000000000LL, pcMPU->GetB());
	AssertShortHex((uint16)0, Test65C64NZ(&sTest));

	//Within one register.
	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x1122334455668888LL, pcMPU->GetA());

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64SubcopyBits(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->Op(Op65Wtth(R65_A, R65_B, 8));
	pcContext->OpQuad(Op65RI(C65_CPY, R65_A, 8), TEST_SUBCOPY_A);
	pcContext->Op(Op65Subcopy(true, R65_A, R65_X, false, 12), Op65SubcopyIndices(true, false, 4, false, 8, 12));
	pcContext->Op(Op65RE(C65_CPY, R65_B, -1));
	pcContext->Op(Op65Subcopy(true, R65_A, R65_B, false, 40), Op65SubcopyIndices(true, false, 0, false, 0, 40));
	pcContext->Op(Op65RR(C65_CPY, R65_B, R65_Zero));
	pcContext->Op(Op65Subcopy(true, R65_A, R65_B, false, 12), Op65SubcopyIndices(true, false, 4, false, 60, 12));
	pcContext->Op(Op65RE(C65_CPY, R65_Y, 16));
	pcContext->Op(Op65RE(C65_CPY, R65_W, 3));
	pcContext->Op(Op65RR(C65_CPY, R65_B, R65_Zero));
	pcContext->Op(Op65Subcopy(true, R65_A, R65_B, true, R65_Y), Op65SubcopyIndices(true, true, R65_W, false, 63));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(5, Test65C64Step(&sTest));

	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex(0x77800LL, pcMPU->GetX());
	AssertShortHex((uint16)STATUS65_ZERO, Test65C64NZ(&sTest));

	//40 bits: the count's top bits are in the index word.
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex((int64)0xFFFFFF4455667788ULL, pcMPU->GetB());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64NZ(&sTest));

	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex((int64)0x8000000000000000ULL, pcMPU->GetB());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64NZ(&sTest));

	//Count in Y, source index in W: A's bit 3 (1) goes to bit 63, the rest is dropped.
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(2, Test65C64Step(&sTest));
	AssertLongLongIntHex((int64)0x8000000000000000ULL, pcMPU->GetB());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64NZ(&sTest));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Subcopy(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	C65C64InstructionFactory::GetInstance()->Init();

	Test65C64SubcopyBytes();
	Test65C64SubcopyBits();

	C65C64InstructionFactory::GetInstance()->Kill();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

