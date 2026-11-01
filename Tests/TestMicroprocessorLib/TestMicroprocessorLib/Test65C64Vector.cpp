#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "MicroprocessorLib/65C64InstructionFactory.h"
#include "MicroprocessorLib/65C64Meta.h"
#include "TestLib/Assert.h"
#include "Test65C64Context.h"


//////////////////////////////////////////////////////////////////////////
//
// Vector operations: [V] op [W] -> [V], one element a step.  [W] is read, then [V], then
// [V] is written.  C stays on the opcode until A is negative after its decrement (or an
// allowed stop condition holds).
//
//////////////////////////////////////////////////////////////////////////


#define V65_CPY		0
#define V65_NOT		1
#define V65_TST		3
#define V65_ADD		6
#define V65_SUB		8
#define V65_CMP		10
#define V65_PCPY	12


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 Test65C64VectorFlags(STest65C64* psTest)
{
	return Test65C64MPU(psTest)->GetP() & (STATUS65_OVERFLOW | STATUS65_NEGATIVE | STATUS65_ZERO);
}


//////////////////////////////////////////////////////////////////////////
//
// Sets V, W and A (all with 16 bit immediates).
//
//////////////////////////////////////////////////////////////////////////
void Test65C64VectorSetup(CTest65C64Context* pcContext, uint16 uiV, uint16 uiW, uint16 uiA)
{
	pcContext->Op(Op65RI(C65_CPY, R65_V, 2), uiV);
	pcContext->Op(Op65RI(C65_CPY, R65_W, 2), uiW);
	pcContext->Op(Op65RI(C65_CPY, R65_A, 1), uiA);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64VectorStepSetup(STest65C64* psTest)
{
	AssertSize(2, Test65C64Step(psTest));
	AssertSize(2, Test65C64Step(psTest));
	AssertSize(2, Test65C64Step(psTest));
}


//////////////////////////////////////////////////////////////////////////
//
// The signed add, subtract and compare (7, 9 and 11) were dropped.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64VectorDecode(void)
{
	S65C64Decoded	sDecoded;

	Decode65C64(Op65Vector(V65_ADD, false, false, 0, false, WIDTH65_8), &sDecoded);
	AssertInt(I65_Vadd, sDecoded.eInstruction);
	Decode65C64(Op65Vector(7, false, false, 0, false, WIDTH65_8), &sDecoded);
	AssertInt(I65_Illegal, sDecoded.eInstruction);
	Decode65C64(Op65Vector(V65_SUB, false, false, 0, false, WIDTH65_8), &sDecoded);
	AssertInt(I65_Vsub, sDecoded.eInstruction);
	Decode65C64(Op65Vector(9, false, false, 0, false, WIDTH65_8), &sDecoded);
	AssertInt(I65_Illegal, sDecoded.eInstruction);
	Decode65C64(Op65Vector(V65_CMP, false, false, 0, false, WIDTH65_8), &sDecoded);
	AssertInt(I65_Vcmp, sDecoded.eInstruction);
	AssertInt(MA65_Read, sDecoded.eAccess);
	Decode65C64(Op65Vector(11, false, false, 0, false, WIDTH65_8), &sDecoded);
	AssertInt(I65_Illegal, sDecoded.eInstruction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64VectorCopyAddSubtract(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetLong(0x2000, 0x44332211);
	pcContext->SetLong(0x2200, 0x00302010);
	pcContext->SetLong(0x2300, 0x00FF0201);
	pcContext->SetWord(0x2210, 0x0005);
	pcContext->SetWord(0x2310, 0x0103);

	//Two 16 bit copies.
	Test65C64VectorSetup(pcContext, 0x2100, 0x2000, 1);
	pcContext->Op(Op65Vector(V65_CPY, false, false, 0, false, WIDTH65_16));
	//Three 8 bit adds.
	Test65C64VectorSetup(pcContext, 0x2200, 0x2300, 2);
	pcContext->Op(Op65Vector(V65_ADD, false, false, 0, false, WIDTH65_8));
	//Two 8 bit subtracts: [V] - [W].
	Test65C64VectorSetup(pcContext, 0x2210, 0x2310, 1);
	pcContext->Op(Op65Vector(V65_SUB, false, false, 0, false, WIDTH65_8));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	Test65C64VectorStepSetup(&sTest);
	AssertSize(3, Test65C64Step(&sTest));
	AssertTrue(pcContext->masCycles[1].bRead);
	AssertIntHex(0x2000, pcContext->masCycles[1].uiAddress);
	AssertFalse(pcContext->masCycles[2].bRead);
	AssertIntHex(0x2100, pcContext->masCycles[2].uiAddress);
	AssertSize(3, Test65C64Step(&sTest));
	AssertIntHex(0x44332211, pcContext->GetLong(0x2100));
	AssertIntHex(0x2104, pcMPU->GetV());
	AssertIntHex(0x2004, pcMPU->GetW());
	AssertShortHex((uint16)STATUS65_OVERFLOW, Test65C64VectorFlags(&sTest));

	Test65C64VectorStepSetup(&sTest);
	AssertSize(4, Test65C64Step(&sTest));
	AssertIntHex(0x2300, pcContext->masCycles[1].uiAddress);
	AssertIntHex(0x2200, pcContext->masCycles[2].uiAddress);
	AssertIntHex(0x2200, pcContext->masCycles[3].uiAddress);
	AssertTrue(pcContext->masCycles[2].bRead);
	AssertFalse(pcContext->masCycles[3].bRead);
	AssertSize(4, Test65C64Step(&sTest));
	AssertSize(4, Test65C64Step(&sTest));
	AssertIntHex(0x002F2211, pcContext->GetLong(0x2200));

	Test65C64VectorStepSetup(&sTest);
	AssertSize(4, Test65C64Step(&sTest));
	AssertSize(4, Test65C64Step(&sTest));
	AssertShortHex((uint16)0xFF02, pcContext->GetWord(0x2210));
	AssertShortHex((uint16)(STATUS65_OVERFLOW | STATUS65_NEGATIVE), Test65C64VectorFlags(&sTest));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// Compare stops at the first difference and writes nothing.  Precopy stops at the
// zero in V (after writing over it).  Test writes nothing.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64VectorCompareStop(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	uint32				uiNext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetLong(0x2400, 0x78434241);
	pcContext->SetLong(0x2500, 0x78444241);
	pcContext->SetLong(0x2600, 0x7A007978);
	pcContext->SetLong(0x2700, 0x64636261);

	Test65C64VectorSetup(pcContext, 0x2400, 0x2500, 3);
	uiNext = pcContext->Here() + 2;
	pcContext->Op(Op65Vector(V65_CMP, false, true, STOP65_NOT_ZERO, false, WIDTH65_8));
	Test65C64VectorSetup(pcContext, 0x2600, 0x2700, 3);
	pcContext->Op(Op65Vector(V65_PCPY, false, true, STOP65_ZERO, false, WIDTH65_8));
	Test65C64VectorSetup(pcContext, 0x2400, 0x2500, 0);
	pcContext->Op(Op65Vector(V65_TST, false, false, 0, false, WIDTH65_8));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	Test65C64VectorStepSetup(&sTest);
	AssertSize(3, Test65C64Step(&sTest));
	AssertShortHex((uint16)STATUS65_ZERO, Test65C64VectorFlags(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertSize(3, Test65C64Step(&sTest));
	AssertIntHex(uiNext, pcMPU->GetProgramCounter());
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64VectorFlags(&sTest));
	AssertIntHex(0x78434241, pcContext->GetLong(0x2400));
	AssertIntHex(0x2403, pcMPU->GetV());
	AssertLongLongIntHex(0LL, pcMPU->GetA());

	Test65C64VectorStepSetup(&sTest);
	AssertSize(4, Test65C64Step(&sTest));
	AssertSize(4, Test65C64Step(&sTest));
	AssertSize(4, Test65C64Step(&sTest));
	AssertShortHex((uint16)STATUS65_ZERO, Test65C64VectorFlags(&sTest));
	AssertIntHex(0x7A636261, pcContext->GetLong(0x2600));

	Test65C64VectorStepSetup(&sTest);
	AssertSize(3, Test65C64Step(&sTest));
	AssertShortHex((uint16)STATUS65_OVERFLOW, Test65C64VectorFlags(&sTest));
	AssertIntHex(0x78434241, pcContext->GetLong(0x2400));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
// 64 bit invert with W going down and V going up.
//
//////////////////////////////////////////////////////////////////////////
void Test65C64VectorInvert(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetQuad(0x2808, 0x00FF00FF00FF00FFULL);
	pcContext->SetQuad(0x2800, 0);
	Test65C64VectorSetup(pcContext, 0x2900, 0x2808, 1);
	pcContext->Op(Op65Vector(V65_NOT, false, false, 0, true, WIDTH65_64));
	pcContext->Op(Op65Stp());

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	Test65C64VectorStepSetup(&sTest);
	AssertSize(9, Test65C64Step(&sTest));
	AssertShortHex((uint16)STATUS65_NEGATIVE, Test65C64VectorFlags(&sTest));
	AssertSize(9, Test65C64Step(&sTest));
	AssertLongLongIntHex((int64)0xFF00FF00FF00FF00ULL, (int64)pcContext->GetQuad(0x2900));
	AssertLongLongIntHex((int64)0xFFFFFFFFFFFFFFFFULL, (int64)pcContext->GetQuad(0x2908));
	AssertIntHex(0x27F8, pcMPU->GetW());
	AssertIntHex(0x2910, pcMPU->GetV());
	AssertShortHex((uint16)(STATUS65_OVERFLOW | STATUS65_NEGATIVE), Test65C64VectorFlags(&sTest));

	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Vector(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	C65C64InstructionFactory::GetInstance()->Init();

	Test65C64VectorDecode();
	Test65C64VectorCopyAddSubtract();
	Test65C64VectorCompareStop();
	Test65C64VectorInvert();

	C65C64InstructionFactory::GetInstance()->Kill();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

