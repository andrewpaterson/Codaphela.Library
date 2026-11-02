#include <string.h>
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "MicroprocessorLib/65C64InstructionFactory.h"
#include "MicroprocessorLib/65C64Meta.h"
#include "MicroprocessorLib/Fast65C64.h"
#include "TestLib/Assert.h"
#include "Test65C64Context.h"


//////////////////////////////////////////////////////////////////////////
//
// CFast65C64 against C65C64.  Both run the same code from the same state in their own copy of memory and
// must agree after every step: registers, P, C, the interrupt depth, waiting and stopping, the clock cycles
// taken and memory.
//
//////////////////////////////////////////////////////////////////////////


#define TEST_FAST_65C64_FUZZ_SIZE		0x1000
#define TEST_FAST_65C64_FUZZ_STEPS		3
#define TEST_FAST_65C64_MAX_FAILURES	8


//Memory for the random instructions: 4KB repeated through the whole address space, so every address is
//memory.
struct STestFast65C64FuzzMemory
{
	uint8	mauiBytes[TEST_FAST_65C64_FUZZ_SIZE];
	size	muiCycles;
};


//Two copies of a program in CTest65C64Contexts, one run by C65C64, one by CFast65C64.
struct STestFast65C64Pair
{
	CTest65C64Context*	pcSimContext;
	CTest65C64Context*	pcFastContext;
	C65C64Meta			cMeta;
	CFast65C64			cFast;
	uint64				uiFastResetCycles;
};


static size	gsTestFast65C64Failures;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint64 TestFast65C64Random(uint64* puiSeed)
{
	//xorshift64*.
	*puiSeed ^= *puiSeed >> 12;
	*puiSeed ^= *puiSeed << 25;
	*puiSeed ^= *puiSeed >> 27;
	return *puiSeed * 0x2545F4914F6CDD1DULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Small addresses, any value, small signed values or edge values.
//
//////////////////////////////////////////////////////////////////////////
static uint64 TestFast65C64RandomRegister(uint64* puiSeed)
{
	static uint64	auiEdges[8] = { 0, 0xFFFFFFFFFFFFFFFFULL, 0x8000000000000000ULL, 0x7FFFFFFFFFFFFFFFULL, 0x80, 0xFF, 0x8000, 0xFFFFFFFFULL };
	uint64			uiRandom;

	uiRandom = TestFast65C64Random(puiSeed);
	switch (uiRandom & 3)
	{
	case 0:
		return (uiRandom >> 8) & (TEST_FAST_65C64_FUZZ_SIZE - 1);
	case 1:
		return uiRandom;
	case 2:
		return (uint64)((int64)((uiRandom >> 8) & 31) - 16);
	default:
		return auiEdges[(uiRandom >> 8) & 7];
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C64FuzzTickHigh(C65C64Meta* pcMPU, void* pvMemory)
{
	STestFast65C64FuzzMemory*	psMemory;
	uint32						uiAddress;
	uint16						uiData;

	psMemory = (STestFast65C64FuzzMemory*)pvMemory;
	if (!pcMPU->GetRWB()->IsHigh() && !pcMPU->GetRWB()->IsLow())
	{
		return;
	}

	psMemory->muiCycles++;
	if (!pcMPU->GetVPA()->IsHigh() && !pcMPU->GetVDA()->IsHigh() && pcMPU->GetVPB()->IsHigh())
	{
		return;
	}

	uiAddress = pcMPU->GetAddress()->Get();
	if (pcMPU->GetRWB()->IsHigh())
	{
		uiData = (uint16)(psMemory->mauiBytes[uiAddress & (TEST_FAST_65C64_FUZZ_SIZE - 1)] | (psMemory->mauiBytes[(uiAddress + 1) & (TEST_FAST_65C64_FUZZ_SIZE - 1)] << 8));
		pcMPU->GetData()->Set(uiData);
	}
	else
	{
		uiData = pcMPU->GetData()->Get();
		if (pcMPU->GetLowerByte()->IsHigh())
		{
			psMemory->mauiBytes[uiAddress & (TEST_FAST_65C64_FUZZ_SIZE - 1)] = (uint8)uiData;
		}
		if (pcMPU->GetUpperByte()->IsHigh())
		{
			psMemory->mauiBytes[(uiAddress + 1) & (TEST_FAST_65C64_FUZZ_SIZE - 1)] = (uint8)(uiData >> 8);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C64FuzzTickLow(C65C64Meta* pcMPU, void* pvMemory)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint16 TestFast65C64FuzzReadWord(void* pvMemory, uint32 uiAddress)
{
	STestFast65C64FuzzMemory*	psMemory;

	psMemory = (STestFast65C64FuzzMemory*)pvMemory;
	return (uint16)(psMemory->mauiBytes[uiAddress & (TEST_FAST_65C64_FUZZ_SIZE - 1)] | (psMemory->mauiBytes[(uiAddress + 1) & (TEST_FAST_65C64_FUZZ_SIZE - 1)] << 8));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C64FuzzWriteByte(void* pvMemory, uint32 uiAddress, uint8 uiValue)
{
	STestFast65C64FuzzMemory*	psMemory;

	psMemory = (STestFast65C64FuzzMemory*)pvMemory;
	psMemory->mauiBytes[uiAddress & (TEST_FAST_65C64_FUZZ_SIZE - 1)] = uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint16 TestFast65C64ContextReadWord(void* pvContext, uint32 uiAddress)
{
	return ((CTest65C64Context*)pvContext)->GetWord(uiAddress);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C64ContextWriteByte(void* pvContext, uint32 uiAddress, uint8 uiValue)
{
	((CTest65C64Context*)pvContext)->SetByte(uiAddress, uiValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C64AppendValue(CChars* psz, const char* szName, uint64 uiValue)
{
	psz->Append(" ");
	psz->Append(szName);
	psz->Append(" 0x");
	psz->AppendHexHiLo(&uiValue, 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C64AppendState(CChars* psz, C65C64* pcSim)
{
	TestFast65C64AppendValue(psz, "A", pcSim->GetA());
	TestFast65C64AppendValue(psz, "B", pcSim->GetB());
	TestFast65C64AppendValue(psz, "X", pcSim->GetX());
	TestFast65C64AppendValue(psz, "Y", pcSim->GetY());
	TestFast65C64AppendValue(psz, "S", pcSim->GetS());
	TestFast65C64AppendValue(psz, "V", pcSim->GetV());
	TestFast65C64AppendValue(psz, "W", pcSim->GetW());
	TestFast65C64AppendValue(psz, "C", pcSim->GetProgramCounter());
	TestFast65C64AppendValue(psz, "P", pcSim->GetP());
}


//////////////////////////////////////////////////////////////////////////
//
// The first few differences fail the test.
//
//////////////////////////////////////////////////////////////////////////
static bool TestFast65C64Failed(CChars* psz)
{
	if (psz->Empty())
	{
		return false;
	}

	gsTestFast65C64Failures++;
	if (gsTestFast65C64Failures <= TEST_FAST_65C64_MAX_FAILURES)
	{
		AssertString("", psz->Text());
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Every opcode, with random registers, P, operand words and memory, for a few steps (the instructions after
// it are random too).  With bFlat CFast65C64 reads and writes the first 4KB directly, otherwise it goes
// through the callbacks for everything.
//
//////////////////////////////////////////////////////////////////////////
void TestFast65C64Fuzz(bool bFlat, uint64 uiSeed)
{
	STestFast65C64FuzzMemory*	psSimMemory;
	STestFast65C64FuzzMemory*	psFastMemory;
	C65C64Meta					cMeta;
	CFast65C64					cFast;
	C65C64*						pcSim;
	uint32						uiOpcode;
	uint32						uiCode;
	uint64						auiRegisters[8];
	uint16						uiP;
	size						i;
	size						uiStep;
	bool						bSim;
	bool						bFast;
	CChars						sz;
	CChars						szWhere;
	CChars						szBefore;
	uint16						uiExecuted;
	uint64						uiSimCycles;
	uint64						uiFastCycles;
	size						uiSteps;

	psSimMemory = (STestFast65C64FuzzMemory*)malloc(sizeof(STestFast65C64FuzzMemory));
	psFastMemory = (STestFast65C64FuzzMemory*)malloc(sizeof(STestFast65C64FuzzMemory));
	psSimMemory->muiCycles = 0;
	psFastMemory->muiCycles = 0;

	cMeta.Init(TestFast65C64FuzzTickHigh, TestFast65C64FuzzTickLow, psSimMemory);
	pcSim = cMeta.GetMPU();
	if (bFlat)
	{
		cFast.Init(TestFast65C64FuzzReadWord, TestFast65C64FuzzWriteByte, psFastMemory, psFastMemory->mauiBytes, TEST_FAST_65C64_FUZZ_SIZE);
	}
	else
	{
		cFast.Init(TestFast65C64FuzzReadWord, TestFast65C64FuzzWriteByte, psFastMemory, NULL, 0);
	}

	uiSteps = 0;
	for (uiOpcode = 0; uiOpcode < 0x10000; uiOpcode++)
	{
		for (i = 0; i < TEST_FAST_65C64_FUZZ_SIZE; i += 8)
		{
			*((uint64*)&psSimMemory->mauiBytes[i]) = TestFast65C64Random(&uiSeed);
		}
		uiCode = (uint32)(TestFast65C64Random(&uiSeed) % 0xF00) & 0xFFE;
		psSimMemory->mauiBytes[uiCode] = (uint8)uiOpcode;
		psSimMemory->mauiBytes[uiCode + 1] = (uint8)(uiOpcode >> 8);

		//The reset vector (FFFF:FFFC) is at 0FFC.
		psSimMemory->mauiBytes[0xFFC] = (uint8)uiCode;
		psSimMemory->mauiBytes[0xFFD] = (uint8)(uiCode >> 8);
		psSimMemory->mauiBytes[0xFFE] = 0;
		psSimMemory->mauiBytes[0xFFF] = 0;
		memcpy(psFastMemory->mauiBytes, psSimMemory->mauiBytes, TEST_FAST_65C64_FUZZ_SIZE);

		cMeta.GetRESB()->Set(false);
		cMeta.TickInstruction();
		cFast.Reset();

		for (i = R65_A; i <= R65_Y; i++)
		{
			auiRegisters[i] = TestFast65C64RandomRegister(&uiSeed);
			pcSim->TestSetRegister((E65Register)i, auiRegisters[i]);
			cFast.TestSetRegister((E65Register)i, auiRegisters[i]);
		}
		uiP = (uint16)TestFast65C64Random(&uiSeed);
		pcSim->TestSetP(uiP);
		cFast.TestSetP(uiP);

		sz.Init();
		cFast.AppendDifferences(pcSim, &sz);
		if (TestFast65C64Failed(&sz))
		{
			sz.Kill();
			break;
		}
		sz.Kill();

		for (uiStep = 0; uiStep < TEST_FAST_65C64_FUZZ_STEPS; uiStep++)
		{
			psSimMemory->muiCycles = 0;
			uiExecuted = TestFast65C64FuzzReadWord(psSimMemory, pcSim->GetProgramCounter() & 0xFFFFFFFE);
			szBefore.Init(" before:");
			TestFast65C64AppendState(&szBefore, pcSim);
			uiFastCycles = cFast.NumCycles();
			bSim = cMeta.TickInstruction();
			bFast = cFast.Step();
			uiSimCycles = psSimMemory->muiCycles;
			uiFastCycles = cFast.NumCycles() - uiFastCycles;
			uiSteps++;

			sz.Init();
			cFast.AppendDifferences(pcSim, &sz);
			cFast.AppendDifference(&sz, "Running", bSim, bFast);
			cFast.AppendDifference(&sz, "Cycles", uiSimCycles, uiFastCycles);
			if (memcmp(psSimMemory->mauiBytes, psFastMemory->mauiBytes, TEST_FAST_65C64_FUZZ_SIZE) != 0)
			{
				sz.Append(" Memory differs");
			}
			if (!sz.Empty())
			{
				szWhere.Init("Opcode 0x");
				szWhere.AppendHexHiLo(&uiOpcode, 2);
				szWhere.Append(" step ");
				szWhere.Append((int)uiStep);
				szWhere.Append(" ran 0x");
				szWhere.AppendHexHiLo(&uiExecuted, 2);
				szWhere.Append(" sim ran 0x");
				szWhere.AppendHexHiLo(&pcSim->GetDecoded()->uiOpcode, 2);
				sz.Insert(0, &szWhere);
				sz.Append(szBefore);
				szWhere.Kill();
			}
			if (TestFast65C64Failed(&sz))
			{
				uiStep = TEST_FAST_65C64_FUZZ_STEPS;
			}
			sz.Kill();
			szBefore.Kill();

			if (!bSim || !bFast)
			{
				break;
			}
		}
	}
	AssertSize(0, gsTestFast65C64Failures);
	AssertTrue(uiSteps > 0x10000);

	cFast.Kill();
	cMeta.Kill();
	free(psFastMemory);
	free(psSimMemory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C64Begin(STestFast65C64Pair* psPair, uint32 uiResetAddress)
{
	psPair->pcSimContext = (CTest65C64Context*)malloc(sizeof(CTest65C64Context));
	psPair->pcSimContext->Init(uiResetAddress);
	psPair->pcFastContext = (CTest65C64Context*)malloc(sizeof(CTest65C64Context));
}


//////////////////////////////////////////////////////////////////////////
//
// Copies the program (written to pcSimContext) for CFast65C64 and runs reset on both.
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C64Reset(STestFast65C64Pair* psPair)
{
	memcpy(psPair->pcFastContext, psPair->pcSimContext, sizeof(CTest65C64Context));
	psPair->cMeta.Init(Test65C64TickHigh, Test65C64TickLow, psPair->pcSimContext);
	psPair->cFast.Init(TestFast65C64ContextReadWord, TestFast65C64ContextWriteByte, psPair->pcFastContext, psPair->pcFastContext->mauiLow, TEST_65C64_LOW_SIZE);

	psPair->cMeta.TickInstruction();
	psPair->cFast.Reset();
	psPair->pcSimContext->muiTotalCycles = 0;
	psPair->uiFastResetCycles = psPair->cFast.NumCycles();
}


//////////////////////////////////////////////////////////////////////////
//
// One step on both.  Returns false if either stopped or they differ.
//
//////////////////////////////////////////////////////////////////////////
static bool TestFast65C64Step(STestFast65C64Pair* psPair)
{
	bool	bSim;
	bool	bFast;
	CChars	sz;
	bool	bFailed;

	bSim = psPair->cMeta.TickInstruction();
	bFast = psPair->cFast.Step();

	sz.Init();
	psPair->cFast.AppendDifferences(psPair->cMeta.GetMPU(), &sz);
	psPair->cFast.AppendDifference(&sz, "Running", bSim, bFast);
	psPair->cFast.AppendDifference(&sz, "Cycles", psPair->pcSimContext->muiTotalCycles, psPair->cFast.NumCycles() - psPair->uiFastResetCycles);
	psPair->cFast.AppendDifference(&sz, "BadAccesses", psPair->pcSimContext->muiBadAccesses, psPair->pcFastContext->muiBadAccesses);
	if ((memcmp(psPair->pcSimContext->mauiLow, psPair->pcFastContext->mauiLow, TEST_65C64_LOW_SIZE) != 0) ||
		(memcmp(psPair->pcSimContext->mauiHigh, psPair->pcFastContext->mauiHigh, TEST_65C64_HIGH_SIZE) != 0))
	{
		sz.Append(" Memory differs");
	}
	bFailed = TestFast65C64Failed(&sz);
	sz.Kill();
	return bSim && bFast && !bFailed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C64End(STestFast65C64Pair* psPair)
{
	psPair->cFast.Kill();
	psPair->cMeta.Kill();
	free(psPair->pcFastContext);
	free(psPair->pcSimContext);
}


//////////////////////////////////////////////////////////////////////////
//
// A program at 0x1000 using edge and level interrupts, brk, rti and stp waiting for an interrupt.
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C64InterruptProgram(CTest65C64Context* pcContext)
{
	pcContext->SetVector(VECTOR_65C64_EDGE_0, 0x1200);
	pcContext->SetVector(VECTOR_65C64_EDGE_0 - 2, 0x1240);
	pcContext->SetVector(VECTOR_65C64_LEVEL, 0x1300);
	pcContext->SetVector(5, 0x1400);

	pcContext->Op(Op65Cep(STATUS65_EDGE_DISABLE | STATUS65_LEVEL_DISABLE));
	pcContext->Op(Op65RE(C65_CPY, R65_A, 1));
	pcContext->Op(Op65RE(C65_CPY, R65_A, 2));
	pcContext->Op(Op65Brk(5));
	pcContext->Op(Op65RE(C65_CPY, R65_B, 3));
	pcContext->Op(Op65StpWake(WAKE65_EDGE_0));
	pcContext->Op(Op65RE(C65_ADD, R65_X, 4));
	pcContext->Op(Op65Sep(STATUS65_EDGE_DISABLE));
	pcContext->Op(Op65StpWake(WAKE65_EDGE_0 << 2));
	pcContext->Op(Op65RE(C65_ADD, R65_X, 5));
	pcContext->Op(Op65Cep(STATUS65_EDGE_DISABLE));
	pcContext->Op(Op65StpWake(WAKE65_LEVEL));
	pcContext->Op(Op65RE(C65_CPY, R65_Y, 5));
	pcContext->Op(Op65RE(C65_ADD, R65_Y, 1));
	pcContext->Op(Op65RE(C65_ADD, R65_Y, 1));
	pcContext->Op(Op65Stp());

	pcContext->Org(0x1200);
	pcContext->Op(Op65RE(C65_ADD, R65_X, 1));
	pcContext->Op(Op65Return(R65_S, true, true, true));
	pcContext->Org(0x1240);
	pcContext->Op(Op65RE(C65_ADD, R65_X, 2));
	pcContext->Op(Op65Return(R65_S, true, true, true));
	pcContext->Org(0x1300);
	pcContext->Op(Op65RE(C65_ADD, R65_Y, 8));
	pcContext->Op(Op65Return(R65_S, true, true, true));
	pcContext->Org(0x1400);
	pcContext->Op(Op65RE(C65_ADD, R65_B, 1));
	pcContext->Op(Op65Return(R65_S, true, true, true));
}


//////////////////////////////////////////////////////////////////////////
//
// Edge and level interrupts, brk, rti and stp waiting for an interrupt, with the inputs changed between steps.
//
//////////////////////////////////////////////////////////////////////////
void TestFast65C64Interrupts(void)
{
	STestFast65C64Pair	sPair;
	size				uiStep;
	bool				bRunning;

	TestFast65C64Begin(&sPair, 0x1000);
	TestFast65C64InterruptProgram(sPair.pcSimContext);
	TestFast65C64Reset(&sPair);

	bRunning = true;
	for (uiStep = 0; (uiStep < 100) && bRunning; uiStep++)
	{
		//Edge 0 falls at step 2 (taken at once), and again at 14 to wake the first stp.  Edge 2 falls while
		//edges are disabled and wakes the second stp.  The level interrupt is low for steps 30 to 33.
		if ((uiStep == 2) || (uiStep == 14))
		{
			sPair.cMeta.GetEIRQB(0)->Set(false);
			sPair.cFast.SetEdgeInterrupt(0, true);
		}
		if ((uiStep == 4) || (uiStep == 16))
		{
			sPair.cMeta.GetEIRQB(0)->Set(true);
			sPair.cFast.SetEdgeInterrupt(0, false);
		}
		if (uiStep == 22)
		{
			sPair.cMeta.GetEIRQB(2)->Set(false);
			sPair.cFast.SetEdgeInterrupt(2, true);
		}
		if (uiStep == 30)
		{
			sPair.cMeta.GetLIRQB()->Set(false);
			sPair.cFast.SetLevelInterrupt(true);
		}
		if (uiStep == 33)
		{
			sPair.cMeta.GetLIRQB()->Set(true);
			sPair.cFast.SetLevelInterrupt(false);
		}
		bRunning = TestFast65C64Step(&sPair);
	}

	AssertSize(0, gsTestFast65C64Failures);
	AssertTrue(sPair.cFast.IsStopped());
	AssertLongLongIntHex(2, sPair.cFast.GetA());
	AssertLongLongIntHex(3, sPair.cFast.GetB());
	AssertLongLongIntHex(1 + 1 + 4 + 5 + 2, sPair.cFast.GetX());
	AssertLongLongIntHex(5 + 1 + 1, sPair.cFast.GetY());

	TestFast65C64End(&sPair);
}


//////////////////////////////////////////////////////////////////////////
//
// pcRun (after a Run) and pcStep (after as many Steps) are the same.
//
//////////////////////////////////////////////////////////////////////////
static bool TestFast65C64RunSame(CFast65C64* pcStep, CFast65C64* pcRun, CTest65C64Context* pcStepContext, CTest65C64Context* pcRunContext, size uiStepped, size uiRun, size uiChunk)
{
	CChars	sz;
	CChars	szWhere;
	bool	bFailed;

	sz.Init();
	pcRun->AppendDifference(&sz, "Steps", uiStepped, uiRun);
	pcRun->AppendDifference(&sz, "A", pcStep->GetA(), pcRun->GetA());
	pcRun->AppendDifference(&sz, "B", pcStep->GetB(), pcRun->GetB());
	pcRun->AppendDifference(&sz, "X", pcStep->GetX(), pcRun->GetX());
	pcRun->AppendDifference(&sz, "Y", pcStep->GetY(), pcRun->GetY());
	pcRun->AppendDifference(&sz, "S", pcStep->GetS(), pcRun->GetS());
	pcRun->AppendDifference(&sz, "V", pcStep->GetV(), pcRun->GetV());
	pcRun->AppendDifference(&sz, "W", pcStep->GetW(), pcRun->GetW());
	pcRun->AppendDifference(&sz, "C", pcStep->GetProgramCounter(), pcRun->GetProgramCounter());
	pcRun->AppendDifference(&sz, "P", pcStep->GetP(), pcRun->GetP());
	pcRun->AppendDifference(&sz, "Depth", pcStep->GetInterruptDepth(), pcRun->GetInterruptDepth());
	pcRun->AppendDifference(&sz, "Stopped", pcStep->IsStopped(), pcRun->IsStopped());
	pcRun->AppendDifference(&sz, "Waiting", pcStep->IsWaiting(), pcRun->IsWaiting());
	pcRun->AppendDifference(&sz, "PendingEdges", pcStep->GetPendingEdges(), pcRun->GetPendingEdges());
	pcRun->AppendDifference(&sz, "Cycles", pcStep->NumCycles(), pcRun->NumCycles());
	pcRun->AppendDifference(&sz, "BadAccesses", pcStepContext->muiBadAccesses, pcRunContext->muiBadAccesses);
	if ((memcmp(pcStepContext->mauiLow, pcRunContext->mauiLow, TEST_65C64_LOW_SIZE) != 0) ||
		(memcmp(pcStepContext->mauiHigh, pcRunContext->mauiHigh, TEST_65C64_HIGH_SIZE) != 0))
	{
		sz.Append(" Memory differs");
	}
	if (!sz.Empty())
	{
		szWhere.Init("Run chunk ");
		szWhere.Append((int)uiChunk);
		szWhere.Append(" (\"simulator\" is Step)");
		sz.Insert(0, &szWhere);
		szWhere.Kill();
	}
	bFailed = TestFast65C64Failed(&sz);
	sz.Kill();
	return !bFailed;
}


//////////////////////////////////////////////////////////////////////////
//
// The interrupt inputs before Run uiChunk: each falls again and again, so whichever stp the program is
// waiting in is woken.
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C64RunInputs(CFast65C64* pcFast, size uiChunk)
{
	pcFast->SetEdgeInterrupt(0, (uiChunk % 6) == 2);
	pcFast->SetEdgeInterrupt(2, (uiChunk % 7) == 4);
	pcFast->SetLevelInterrupt(((uiChunk % 9) == 5) || ((uiChunk % 9) == 6));
}


//////////////////////////////////////////////////////////////////////////
//
// Run does what as many Steps do: edge and level interrupts, brk, rti and stp waiting for an interrupt, with
// the inputs changed between Runs of different lengths.
//
//////////////////////////////////////////////////////////////////////////
void TestFast65C64Run(void)
{
	CTest65C64Context*	pcStepContext;
	CTest65C64Context*	pcRunContext;
	CFast65C64			cStep;
	CFast65C64			cRun;
	size				uiChunk;
	size				uiLength;
	size				uiStepped;
	size				uiRun;
	static size			auiLengths[] = { 1, 2, 3, 1, 4, 2, 7, 1, 5, 3 };

	pcStepContext = (CTest65C64Context*)malloc(sizeof(CTest65C64Context));
	pcStepContext->Init(0x1000);
	TestFast65C64InterruptProgram(pcStepContext);
	pcRunContext = (CTest65C64Context*)malloc(sizeof(CTest65C64Context));
	memcpy(pcRunContext, pcStepContext, sizeof(CTest65C64Context));

	cStep.Init(TestFast65C64ContextReadWord, TestFast65C64ContextWriteByte, pcStepContext, pcStepContext->mauiLow, TEST_65C64_LOW_SIZE);
	cRun.Init(TestFast65C64ContextReadWord, TestFast65C64ContextWriteByte, pcRunContext, pcRunContext->mauiLow, TEST_65C64_LOW_SIZE);
	cStep.Reset();
	cRun.Reset();

	for (uiChunk = 0; (uiChunk < 60) && !cStep.IsStopped(); uiChunk++)
	{
		TestFast65C64RunInputs(&cStep, uiChunk);
		TestFast65C64RunInputs(&cRun, uiChunk);

		uiLength = auiLengths[uiChunk % (sizeof(auiLengths) / sizeof(size))];
		for (uiStepped = 0; uiStepped < uiLength; )
		{
			uiStepped++;
			if (!cStep.Step())
			{
				break;
			}
		}
		uiRun = cRun.Run(uiLength);

		if (!TestFast65C64RunSame(&cStep, &cRun, pcStepContext, pcRunContext, uiStepped, uiRun, uiChunk))
		{
			break;
		}
	}
	AssertTrue(cRun.IsStopped());
	AssertSize(1, cRun.Run(10));

	cRun.Kill();
	cStep.Kill();
	free(pcRunContext);
	free(pcStepContext);
}


//A device at TEST_FAST_65C64_DEVICE (a hole): writing a non-zero byte to it pulls edge interrupt 0 low, from
//inside the instruction.
#define TEST_FAST_65C64_DEVICE	0x3000
struct STestFast65C64Device
{
	CTest65C64Context*	pcContext;
	CFast65C64*			pcFast;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint16 TestFast65C64DeviceReadWord(void* pvDevice, uint32 uiAddress)
{
	STestFast65C64Device*	psDevice;

	psDevice = (STestFast65C64Device*)pvDevice;
	if ((uiAddress >= TEST_FAST_65C64_DEVICE) && (uiAddress < TEST_FAST_65C64_DEVICE + 0x10))
	{
		return 0;
	}
	return psDevice->pcContext->GetWord(uiAddress);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C64DeviceWriteByte(void* pvDevice, uint32 uiAddress, uint8 uiValue)
{
	STestFast65C64Device*	psDevice;

	psDevice = (STestFast65C64Device*)pvDevice;
	if ((uiAddress >= TEST_FAST_65C64_DEVICE) && (uiAddress < TEST_FAST_65C64_DEVICE + 0x10))
	{
		if (uiValue != 0)
		{
			psDevice->pcFast->SetEdgeInterrupt(0, true);
		}
		return;
	}
	psDevice->pcContext->SetByte(uiAddress, uiValue);
}


//////////////////////////////////////////////////////////////////////////
//
// Run does what as many Steps do when an instruction's own write pulls an interrupt input low: the interrupt
// is taken straight after that instruction.
//
//////////////////////////////////////////////////////////////////////////
void TestFast65C64RunDevice(void)
{
	CTest65C64Context*		pcStepContext;
	CTest65C64Context*		pcRunContext;
	STestFast65C64Device	sStepDevice;
	STestFast65C64Device	sRunDevice;
	CFast65C64				cStep;
	CFast65C64				cRun;
	size					uiChunk;
	size					uiLength;
	size					uiStepped;
	size					uiRun;
	static size				auiLengths[] = { 3, 2, 4, 1, 5 };

	pcStepContext = (CTest65C64Context*)malloc(sizeof(CTest65C64Context));
	pcStepContext->Init(0x1000);
	pcStepContext->SetVector(VECTOR_65C64_EDGE_0, 0x1200);
	pcStepContext->Op(Op65Cep(STATUS65_EDGE_DISABLE | STATUS65_LEVEL_DISABLE));
	pcStepContext->Op(Op65RE(C65_CPY, R65_A, 1));
	pcStepContext->OpLong(Op65M(C65_CPY, true, R65_A, OPERAND_MODE_ABSOLUTE, R65_Zero), TEST_FAST_65C64_DEVICE);
	pcStepContext->Op(Op65RE(C65_ADD, R65_Y, 4));
	pcStepContext->Op(Op65RE(C65_ADD, R65_Y, 4));
	pcStepContext->Op(Op65RE(C65_ADD, R65_Y, 4));
	pcStepContext->Op(Op65Stp());
	pcStepContext->Org(0x1200);
	pcStepContext->Op(Op65RE(C65_ADD, R65_X, 1));
	pcStepContext->Op(Op65Return(R65_S, true, true, true));
	pcRunContext = (CTest65C64Context*)malloc(sizeof(CTest65C64Context));
	memcpy(pcRunContext, pcStepContext, sizeof(CTest65C64Context));

	sStepDevice.pcContext = pcStepContext;
	sStepDevice.pcFast = &cStep;
	sRunDevice.pcContext = pcRunContext;
	sRunDevice.pcFast = &cRun;
	cStep.Init(TestFast65C64DeviceReadWord, TestFast65C64DeviceWriteByte, &sStepDevice, pcStepContext->mauiLow, TEST_65C64_LOW_SIZE);
	cRun.Init(TestFast65C64DeviceReadWord, TestFast65C64DeviceWriteByte, &sRunDevice, pcRunContext->mauiLow, TEST_65C64_LOW_SIZE);
	cStep.SetHole(TEST_FAST_65C64_DEVICE, TEST_FAST_65C64_DEVICE + 0x10);
	cRun.SetHole(TEST_FAST_65C64_DEVICE, TEST_FAST_65C64_DEVICE + 0x10);
	cStep.Reset();
	cRun.Reset();

	for (uiChunk = 0; (uiChunk < 20) && !cStep.IsStopped(); uiChunk++)
	{
		uiLength = auiLengths[uiChunk % (sizeof(auiLengths) / sizeof(size))];
		for (uiStepped = 0; uiStepped < uiLength; )
		{
			uiStepped++;
			if (!cStep.Step())
			{
				break;
			}
		}
		uiRun = cRun.Run(uiLength);

		if (!TestFast65C64RunSame(&cStep, &cRun, pcStepContext, pcRunContext, uiStepped, uiRun, uiChunk))
		{
			break;
		}
	}
	AssertTrue(cRun.IsStopped());
	AssertLongLongIntHex(1, cRun.GetX());
	AssertLongLongIntHex(12, cRun.GetY());

	cRun.Kill();
	cStep.Kill();
	free(pcRunContext);
	free(pcStepContext);
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer at an odd address takes three aligned words.  A stack at an odd and an even address.
//
//////////////////////////////////////////////////////////////////////////
void TestFast65C64Unaligned(void)
{
	STestFast65C64Pair	sPair;
	CTest65C64Context*	pcContext;
	size				uiStep;

	TestFast65C64Begin(&sPair, 0x1000);
	pcContext = sPair.pcSimContext;
	pcContext->SetLong(0x2001, 0x00003001);
	pcContext->SetQuad(0x3001, 0x1122334455667788ULL);

	//wth A, 64; cpy A, [0000:2001] (indirect) then a push and pull of A on an even S.
	pcContext->Op(Op65Wth(R65_A, 8));
	pcContext->OpLong(Op65M(C65_CPY, false, R65_A, OPERAND_MODE_ABSOLUTE_INDIRECT, R65_Zero), 0x2001);
	pcContext->Op(Op65RE(C65_SUB, R65_S, 1));
	pcContext->Op(Op65Push(R65_S, M65_A));
	pcContext->Op(Op65RE(C65_CPY, R65_A, 3));
	pcContext->Op(Op65Pull(R65_S, M65_A));
	pcContext->Op(Op65Stp());

	TestFast65C64Reset(&sPair);
	for (uiStep = 0; uiStep < 20; uiStep++)
	{
		if (!TestFast65C64Step(&sPair))
		{
			break;
		}
	}

	AssertSize(0, gsTestFast65C64Failures);
	AssertTrue(sPair.cFast.IsStopped());
	AssertLongLongIntHex(0x1122334455667788LL, sPair.cFast.GetA());
	AssertLongLongIntHex(0x1122334455667788LL, sPair.cMeta.GetMPU()->GetA());

	TestFast65C64End(&sPair);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFast65C64(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	C65C64InstructionFactory::GetInstance()->Init();
	gsTestFast65C64Failures = 0;

	TestFast65C64Unaligned();
	TestFast65C64Interrupts();
	TestFast65C64Run();
	TestFast65C64RunDevice();
	TestFast65C64Fuzz(true, 0x6502C64ULL);
	TestFast65C64Fuzz(false, 0x65C816ULL);

	C65C64InstructionFactory::GetInstance()->Kill();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

