#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "MicroprocessorLib/65C64InstructionFactory.h"
#include "MicroprocessorLib/65C64Meta.h"
#include "TestLib/Assert.h"
#include "Test65C64Context.h"


//////////////////////////////////////////////////////////////////////////
//
// 65C64 interrupts.  The interrupt inputs are changed between instructions.  An
// interrupt is taken at the end of the instruction during which it is seen, so a
// step that ends in an interrupt also runs the interrupt's cycles: the pushes of C
// and P and the two vector reads.
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64InterruptLevel(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;
	STest65C64Cycle*	psCycles;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetVector(VECTOR_65C64_LEVEL, 0x1200);
	pcContext->Op(Op65Cep(STATUS65_LEVEL_DISABLE));
	pcContext->Op(Op65RE(C65_CPY, R65_A, 1));
	pcContext->Op(Op65RE(C65_CPY, R65_A, 2));
	pcContext->Op(Op65Stp());
	pcContext->Org(0x1200);
	pcContext->Op(Op65RE(C65_CPY, R65_B, 5));
	pcContext->Op(Op65Return(R65_S, true, true, true));

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	AssertSize(1, Test65C64Step(&sTest));
	AssertShortHex((uint16)STATUS65_EDGE_DISABLE, pcMPU->GetP());

	sTest.cMeta.GetLIRQB()->Set(false);
	AssertSize(6, Test65C64Step(&sTest));
	psCycles = pcContext->masCycles;
	AssertLongLongIntHex(1LL, pcMPU->GetA());
	AssertIntHex(0x1FFFC, psCycles[1].uiAddress);
	AssertFalse(psCycles[1].bRead);
	AssertTrue(psCycles[1].bVDA);
	AssertFalse(psCycles[1].bOPV);
	AssertIntHex(0x1FFFE, psCycles[2].uiAddress);
	AssertIntHex(0x1FFFA, psCycles[3].uiAddress);
	AssertIntHex(0xFFFFFFE8, psCycles[4].uiAddress);
	AssertFalse(psCycles[4].bVPB);
	AssertTrue(psCycles[4].bRead);
	AssertIntHex(0xFFFFFFEA, psCycles[5].uiAddress);
	AssertFalse(psCycles[5].bVPB);
	AssertIntHex(0x1200, pcMPU->GetProgramCounter());
	AssertIntHex(0x0001FFF9, pcMPU->GetS());
	AssertIntHex(0x1004, pcContext->GetLong(0x1FFFC));
	AssertShortHex((uint16)STATUS65_EDGE_DISABLE, pcContext->GetWord(0x1FFFA));
	AssertShortHex((uint16)(STATUS65_EDGE_DISABLE | STATUS65_LEVEL_DISABLE), pcMPU->GetP());
	AssertInt(2, pcMPU->GetInterruptDepth());

	//l is set, the level interrupt still being low does not interrupt the handler.
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(5LL, pcMPU->GetB());
	AssertIntHex(0x1202, pcMPU->GetProgramCounter());

	sTest.cMeta.GetLIRQB()->Set(true);
	AssertSize(4, Test65C64Step(&sTest));
	AssertIntHex(0x1004, pcMPU->GetProgramCounter());
	AssertShortHex((uint16)STATUS65_EDGE_DISABLE, pcMPU->GetP());
	AssertIntHex(0x0001FFFF, pcMPU->GetS());
	AssertInt(1, pcMPU->GetInterruptDepth());

	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(2LL, pcMPU->GetA());
	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64InterruptEdge(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetVector(VECTOR_65C64_EDGE_0, 0x1300);
	pcContext->SetVector(VECTOR_65C64_EDGE_0 - 2, 0x1400);
	pcContext->SetVector(VECTOR_65C64_LEVEL, 0x1200);
	pcContext->Op(Op65RE(C65_CPY, R65_A, 1));
	pcContext->Op(Op65RE(C65_CPY, R65_A, 2));
	pcContext->Op(Op65Cep(STATUS65_EDGE_DISABLE | STATUS65_LEVEL_DISABLE));
	pcContext->Op(Op65RE(C65_CPY, R65_A, 3));
	pcContext->Op(Op65Stp());
	pcContext->Org(0x1200);
	pcContext->Op(Op65Return(R65_S, true, true, true));
	pcContext->Org(0x1300);
	pcContext->Op(Op65Return(R65_S, true, true, true));
	pcContext->Org(0x1400);
	pcContext->Op(Op65Return(R65_S, true, true, true));

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	//Edge interrupts are latched while e is set.  Only the falling edge matters.
	sTest.cMeta.GetEIRQB(2)->Set(false);
	AssertSize(1, Test65C64Step(&sTest));
	sTest.cMeta.GetEIRQB(2)->Set(true);
	AssertInt(0x04, pcMPU->GetPendingEdges());

	sTest.cMeta.GetLIRQB()->Set(false);
	sTest.cMeta.GetEIRQB(0)->Set(false);
	AssertSize(1, Test65C64Step(&sTest));
	sTest.cMeta.GetEIRQB(0)->Set(true);
	AssertInt(0x05, pcMPU->GetPendingEdges());
	AssertLongLongIntHex(2LL, pcMPU->GetA());

	//Edge interrupt 0 is taken first.  It disables edge and level interrupts.
	AssertSize(6, Test65C64Step(&sTest));
	AssertIntHex(0x1300, pcMPU->GetProgramCounter());
	AssertIntHex(0xFFFFFFF8, pcContext->masCycles[4].uiAddress);
	AssertIntHex(0x1006, pcContext->GetLong(0x1FFFC));
	AssertShortHex((uint16)0x0000, pcContext->GetWord(0x1FFFA));
	AssertShortHex((uint16)(STATUS65_EDGE_DISABLE | STATUS65_LEVEL_DISABLE), pcMPU->GetP());
	AssertInt(0x04, pcMPU->GetPendingEdges());
	AssertInt(2, pcMPU->GetInterruptDepth());

	//rti enables them again; edge interrupt 2 is next, then the level interrupt.
	AssertSize(9, Test65C64Step(&sTest));
	AssertIntHex(0x1400, pcMPU->GetProgramCounter());
	AssertIntHex(0xFFFFFFF0, pcContext->masCycles[7].uiAddress);
	AssertIntHex(0x1006, pcContext->GetLong(0x1FFFC));
	AssertInt(0x00, pcMPU->GetPendingEdges());
	AssertInt(2, pcMPU->GetInterruptDepth());

	AssertSize(9, Test65C64Step(&sTest));
	AssertIntHex(0x1200, pcMPU->GetProgramCounter());
	AssertIntHex(0xFFFFFFE8, pcContext->masCycles[7].uiAddress);
	AssertShortHex((uint16)STATUS65_LEVEL_DISABLE, pcMPU->GetP());
	sTest.cMeta.GetLIRQB()->Set(true);

	AssertSize(4, Test65C64Step(&sTest));
	AssertIntHex(0x1006, pcMPU->GetProgramCounter());
	AssertShortHex((uint16)0x0000, pcMPU->GetP());
	AssertInt(1, pcMPU->GetInterruptDepth());
	AssertIntHex(0x0001FFFF, pcMPU->GetS());

	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(3LL, pcMPU->GetA());
	AssertFalse(sTest.cMeta.TickInstruction());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64InterruptWake(void)
{
	STest65C64			sTest;
	C65C64*				pcMPU;
	CTest65C64Context*	pcContext;

	Test65C64Begin(&sTest, 0x1000);
	pcContext = sTest.pcContext;
	pcContext->SetVector(VECTOR_65C64_EDGE_0 - 1, 0x1300);
	pcContext->SetVector(VECTOR_65C64_EDGE_0 - 2, 0x1400);
	pcContext->Op(Op65StpWake(WAKE65_LEVEL));
	pcContext->Op(Op65RE(C65_CPY, R65_A, 1));
	pcContext->Op(Op65Cep(STATUS65_EDGE_DISABLE | STATUS65_LEVEL_DISABLE));
	pcContext->Op(Op65StpWake(WAKE65_EDGE_0 << 1));
	pcContext->Op(Op65RE(C65_CPY, R65_A, 2));
	pcContext->Op(Op65Stp());
	pcContext->Org(0x1300);
	pcContext->Op(Op65RE(C65_CPY, R65_B, 7));
	pcContext->Op(Op65Return(R65_S, true, true, true));
	pcContext->Org(0x1400);
	pcContext->Op(Op65Return(R65_S, true, true, true));

	Test65C64Reset(&sTest);
	pcMPU = Test65C64MPU(&sTest);

	//stp waits for the level interrupt.  l is set so it carries on after stp.
	AssertSize(1, Test65C64Step(&sTest));
	AssertTrue(pcMPU->IsWaiting());
	AssertFalse(pcMPU->IsStopped());
	AssertSize(1, Test65C64Step(&sTest));
	AssertTrue(pcMPU->IsWaiting());
	AssertFalse(pcContext->masCycles[0].bVPA);
	AssertFalse(pcContext->masCycles[0].bVDA);

	sTest.cMeta.GetLIRQB()->Set(false);
	AssertSize(1, Test65C64Step(&sTest));
	sTest.cMeta.GetLIRQB()->Set(true);
	AssertFalse(pcMPU->IsWaiting());
	AssertInt(1, pcMPU->GetInterruptDepth());
	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(1LL, pcMPU->GetA());

	//stp waits for edge interrupt 1.  Edge interrupt 2 does not wake it but stays pending.
	AssertSize(1, Test65C64Step(&sTest));
	AssertSize(1, Test65C64Step(&sTest));
	AssertTrue(pcMPU->IsWaiting());
	sTest.cMeta.GetEIRQB(2)->Set(false);
	AssertSize(1, Test65C64Step(&sTest));
	sTest.cMeta.GetEIRQB(2)->Set(true);
	AssertTrue(pcMPU->IsWaiting());
	AssertInt(0x04, pcMPU->GetPendingEdges());

	sTest.cMeta.GetEIRQB(1)->Set(false);
	AssertSize(6, Test65C64Step(&sTest));
	sTest.cMeta.GetEIRQB(1)->Set(true);
	AssertFalse(pcMPU->IsWaiting());
	AssertIntHex(0x1300, pcMPU->GetProgramCounter());
	AssertIntHex(0xFFFFFFF4, pcContext->masCycles[4].uiAddress);
	AssertIntHex(0x1008, pcContext->GetLong(0x1FFFC));
	AssertInt(0x04, pcMPU->GetPendingEdges());

	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(7LL, pcMPU->GetB());
	AssertSize(9, Test65C64Step(&sTest));
	AssertIntHex(0x1400, pcMPU->GetProgramCounter());
	AssertIntHex(0x1008, pcContext->GetLong(0x1FFFC));
	AssertSize(4, Test65C64Step(&sTest));
	AssertIntHex(0x1008, pcMPU->GetProgramCounter());
	AssertInt(1, pcMPU->GetInterruptDepth());

	AssertSize(1, Test65C64Step(&sTest));
	AssertLongLongIntHex(2LL, pcMPU->GetA());

	//stp without wake bits only restarts on reset.
	AssertFalse(sTest.cMeta.TickInstruction());
	AssertTrue(pcMPU->IsStopped());
	sTest.cMeta.GetEIRQB(1)->Set(false);
	AssertFalse(sTest.cMeta.TickInstruction());
	sTest.cMeta.GetEIRQB(1)->Set(true);
	AssertTrue(pcMPU->IsStopped());
	AssertIntHex(0x100C, pcMPU->GetProgramCounter());
	Test65C64End(&sTest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Test65C64Interrupt(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	C65C64InstructionFactory::GetInstance()->Init();

	Test65C64InterruptLevel();
	Test65C64InterruptEdge();
	Test65C64InterruptWake();

	C65C64InstructionFactory::GetInstance()->Kill();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

