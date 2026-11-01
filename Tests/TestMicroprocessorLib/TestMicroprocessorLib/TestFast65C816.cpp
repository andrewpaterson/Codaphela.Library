#include <stdio.h>
#include <string.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "MicroprocessorLib/W65C816InstructionFactory.h"
#include "MicroprocessorLib/W65C816InstructionCodes.h"
#include "MicroprocessorLib/W65C816Meta.h"
#include "MicroprocessorLib/Fast65C816.h"
#include "TestLib/Assert.h"
#include "TestSingleStep.h"


//////////////////////////////////////////////////////////////////////////
//
// CFast65C816 against the SingleStepTests (every access with VDA, VPA or VPB, the cycle count and the
// final state) and against CW65C816 (random instructions and interrupts run in lockstep: registers, the
// cycles taken and memory after every step).
//
//////////////////////////////////////////////////////////////////////////


#define TEST_FAST_65C816_FUZZ_SIZE		0x10000
#define TEST_FAST_65C816_FUZZ_TRIALS	48
#define TEST_FAST_65C816_FUZZ_STEPS		3
#define TEST_FAST_65C816_MAX_FAILURES	8
#define TEST_FAST_65C816_MAX_ACCESSES	SINGLE_STEP_MAX_CYCLES


struct STestFast65C816Access
{
	uint32	uiAddress;
	uint8	uiValue;
	bool	bRead;
};


//The single step memory, and the accesses CFast65C816 made.
class CTestFast65C816StepContext : public CTestSingleStepContext
{
public:
	STestFast65C816Access	masAccesses[TEST_FAST_65C816_MAX_ACCESSES];
	size					muiAccesses;
};


//64KB repeated through the whole address space, so every address is memory.
struct STestFast65C816FuzzMemory
{
	uint8	mauiBytes[TEST_FAST_65C816_FUZZ_SIZE];
	size	muiCycles;
};


static size	gsTestFast65C816Failures;


//////////////////////////////////////////////////////////////////////////
//
// The first few differences fail the test.
//
//////////////////////////////////////////////////////////////////////////
static bool TestFast65C816Failed(CChars* psz)
{
	if (psz->Empty())
	{
		return false;
	}

	gsTestFast65C816Failures++;
	if (gsTestFast65C816Failures <= TEST_FAST_65C816_MAX_FAILURES)
	{
		AssertString("", psz->Text());
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint8 TestFast65C816StepRead(void* pvContext, uint32 uiAddress)
{
	CTestFast65C816StepContext*		pcContext;
	STestFast65C816Access*			psAccess;
	uint8							uiValue;

	pcContext = (CTestFast65C816StepContext*)pvContext;
	uiValue = pcContext->GetByte(uiAddress);
	if (pcContext->muiAccesses < TEST_FAST_65C816_MAX_ACCESSES)
	{
		psAccess = &pcContext->masAccesses[pcContext->muiAccesses];
		psAccess->uiAddress = uiAddress;
		psAccess->uiValue = uiValue;
		psAccess->bRead = true;
	}
	pcContext->muiAccesses++;
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C816StepWrite(void* pvContext, uint32 uiAddress, uint8 uiValue)
{
	CTestFast65C816StepContext*		pcContext;
	STestFast65C816Access*			psAccess;

	pcContext = (CTestFast65C816StepContext*)pvContext;
	pcContext->SetByte(uiAddress, uiValue);
	if (pcContext->muiAccesses < TEST_FAST_65C816_MAX_ACCESSES)
	{
		psAccess = &pcContext->masAccesses[pcContext->muiAccesses];
		psAccess->uiAddress = uiAddress;
		psAccess->uiValue = uiValue;
		psAccess->bRead = false;
	}
	pcContext->muiAccesses++;
}


//////////////////////////////////////////////////////////////////////////
//
// The expected cycles with VDA, VPA or VPB against the accesses made.
//
//////////////////////////////////////////////////////////////////////////
static bool TestFast65C816StepCompareAccesses(CChars* pszError, SSingleStepCycle* pasExpected, size uiExpected, CTestFast65C816StepContext* pcContext)
{
	size					i;
	size					uiAccess;
	SSingleStepCycle*		psExpected;
	STestFast65C816Access*	psActual;
	bool					bRead;

	uiAccess = 0;
	for (i = 0; i < uiExpected; i++)
	{
		psExpected = &pasExpected[i];
		if (!(psExpected->uiValid & SINGLE_STEP_ADDRESS_VALID) || !(psExpected->uiSignals & (SINGLE_STEP_VDA | SINGLE_STEP_VPA | SINGLE_STEP_VPB)))
		{
			continue;
		}

		bRead = (psExpected->uiSignals & SINGLE_STEP_READ) != 0;
		if (uiAccess >= pcContext->muiAccesses)
		{
			pszError->Append("cycle ");
			pszError->Append((int)i + 1);
			pszError->Append(" missing access ");
			TestSingleStepAppendHex(pszError, psExpected->uiAddress, 6);
			pszError->Append(bRead ? " read" : " write");
			return false;
		}

		psActual = &pcContext->masAccesses[uiAccess];
		if ((psActual->uiAddress != psExpected->uiAddress) || (psActual->bRead != bRead) || ((psExpected->uiValid & SINGLE_STEP_VALUE_VALID) && (psActual->uiValue != psExpected->uiValue)))
		{
			pszError->Append("cycle ");
			pszError->Append((int)i + 1);
			pszError->Append(" expected ");
			TestSingleStepAppendHex(pszError, psExpected->uiAddress, 6);
			pszError->Append(bRead ? " read " : " write ");
			TestSingleStepAppendHex(pszError, psExpected->uiValue, 2);
			pszError->Append(" actual ");
			TestSingleStepAppendHex(pszError, psActual->uiAddress, 6);
			pszError->Append(psActual->bRead ? " read " : " write ");
			TestSingleStepAppendHex(pszError, psActual->uiValue, 2);
			return false;
		}
		uiAccess++;
	}

	if (uiAccess != pcContext->muiAccesses)
	{
		pszError->Append("extra access ");
		TestSingleStepAppendHex(pszError, pcContext->masAccesses[uiAccess].uiAddress, 6);
		pszError->Append(pcContext->masAccesses[uiAccess].bRead ? " read" : " write");
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool TestFast65C816StepCompareState(CChars* pszError, SSingleStepState* psExpected, CFast65C816* pcFast, CTestFast65C816StepContext* pcContext)
{
	CSingleStepReader	cReader;
	uint16				i;
	uint32				uiAddress;
	uint8				uiValue;

	if (!TestSingleStepCompareRegister(pszError, "E", psExpected->uiE, pcFast->IsEmulation() ? 1 : 0, 1)) return false;
	if (!TestSingleStepCompareRegister(pszError, "P", psExpected->uiP, pcFast->GetP(), 2)) return false;
	if (!TestSingleStepCompareRegister(pszError, "PBR", psExpected->uiPBR, pcFast->GetProgramBank(), 2)) return false;
	if (!TestSingleStepCompareRegister(pszError, "PC", psExpected->uiPC, pcFast->GetProgramCounter(), 4)) return false;
	if (!TestSingleStepCompareRegister(pszError, "A", psExpected->uiA, pcFast->GetC(), 4)) return false;
	if (!TestSingleStepCompareRegister(pszError, "X", psExpected->uiX, pcFast->GetX(), 4)) return false;
	if (!TestSingleStepCompareRegister(pszError, "Y", psExpected->uiY, pcFast->GetY(), 4)) return false;
	if (!TestSingleStepCompareRegister(pszError, "S", psExpected->uiS, pcFast->GetS(), 4)) return false;
	if (!TestSingleStepCompareRegister(pszError, "D", psExpected->uiD, pcFast->GetD(), 4)) return false;
	if (!TestSingleStepCompareRegister(pszError, "DBR", psExpected->uiDBR, pcFast->GetDataBank(), 2)) return false;

	cReader.Init(psExpected->puiRam);
	for (i = 0; i < psExpected->uiRamCount; i++)
	{
		uiAddress = cReader.ReadAddress();
		uiValue = cReader.ReadByte();
		if (pcContext->GetByte(uiAddress) != uiValue)
		{
			pszError->Append("ram ");
			TestSingleStepAppendHex(pszError, uiAddress, 6);
			pszError->Append(" expected ");
			TestSingleStepAppendHex(pszError, uiValue, 2);
			pszError->Append(" actual ");
			TestSingleStepAppendHex(pszError, pcContext->GetByte(uiAddress), 2);
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool TestFast65C816StepRunTest(CSingleStepReader* pcReader, CChars* pszError, uint8 uiOpcode, CTestFast65C816StepContext* pcContext)
{
	SSingleStepState	sInitial;
	SSingleStepState	sFinal;
	SSingleStepCycle	asExpected[SINGLE_STEP_MAX_CYCLES];
	uint16				uiCycles;
	uint16				i;
	CFast65C816			cFast;
	CSingleStepReader	cRam;
	uint32				uiAddress;
	uint64				uiFastCycles;
	bool				bPassed;
	bool				bWaitOrStop;

	bWaitOrStop = (uiOpcode == WAI_Implied) || (uiOpcode == STP_Implied);
	TestSingleStepReadState(pcReader, &sInitial);
	TestSingleStepReadState(pcReader, &sFinal);
	uiCycles = pcReader->ReadShort();
	for (i = 0; i < uiCycles; i++)
	{
		if (i < SINGLE_STEP_MAX_CYCLES)
		{
			asExpected[i].uiAddress = pcReader->ReadAddress();
			asExpected[i].uiValue = pcReader->ReadByte();
			asExpected[i].uiSignals = pcReader->ReadByte();
			asExpected[i].uiValid = pcReader->ReadByte();
		}
		else
		{
			pcReader->Skip(6);
		}
	}
	if (uiCycles > SINGLE_STEP_MAX_CYCLES)
	{
		pszError->Append("too many cycles");
		return false;
	}

	pcContext->Init();
	pcContext->muiAccesses = 0;
	cRam.Init(sInitial.puiRam);
	for (i = 0; i < sInitial.uiRamCount; i++)
	{
		uiAddress = cRam.ReadAddress();
		pcContext->SetByte(uiAddress, cRam.ReadByte());
	}

	cFast.Init(TestFast65C816StepRead, TestFast65C816StepWrite, pcContext, NULL, 0);
	cFast.TestSetP(sInitial.uiP, sInitial.uiE != 0);
	cFast.TestSetC(sInitial.uiA);
	cFast.TestSetX(sInitial.uiX);
	cFast.TestSetY(sInitial.uiY);
	cFast.TestSetS(sInitial.uiS);
	cFast.TestSetD(sInitial.uiD);
	cFast.TestSetDataBank(sInitial.uiDBR);
	cFast.TestSetProgramCounter(sInitial.uiPBR, sInitial.uiPC);

	//A block move is a Step for each byte, the tests run it until C reaches FFFF.
	uiFastCycles = cFast.NumCycles();
	do
	{
		cFast.Step();
	}
	while (((uiOpcode == MVN_BlockMove) || (uiOpcode == MVP_BlockMove)) && (cFast.GetProgramCounter() == sInitial.uiPC) && (cFast.NumCycles() - uiFastCycles < uiCycles));
	uiFastCycles = cFast.NumCycles() - uiFastCycles;

	bPassed = TestFast65C816StepCompareAccesses(pszError, asExpected, uiCycles, pcContext);
	if (bPassed && !bWaitOrStop && (uiFastCycles != uiCycles))
	{
		pszError->Append("cycles expected ");
		pszError->Append((int)uiCycles);
		pszError->Append(" actual ");
		pszError->Append((int)uiFastCycles);
		bPassed = false;
	}
	if (bPassed)
	{
		bPassed = TestFast65C816StepCompareState(pszError, &sFinal, &cFast, pcContext);
	}

	if (!bPassed)
	{
		pszError->Append("\n  initial E.");
		TestSingleStepAppendHex(pszError, sInitial.uiE, 1);
		pszError->Append(" P.");
		TestSingleStepAppendHex(pszError, sInitial.uiP, 2);
		pszError->Append(" PC.");
		TestSingleStepAppendHex(pszError, sInitial.uiPBR, 2);
		pszError->Append(":");
		TestSingleStepAppendHex(pszError, sInitial.uiPC, 4);
		pszError->Append(" S.");
		TestSingleStepAppendHex(pszError, sInitial.uiS, 4);
		pszError->Append(" D.");
		TestSingleStepAppendHex(pszError, sInitial.uiD, 4);
		pszError->Append(" X.");
		TestSingleStepAppendHex(pszError, sInitial.uiX, 4);
		pszError->Append(" Y.");
		TestSingleStepAppendHex(pszError, sInitial.uiY, 4);
	}

	cFast.Kill();
	pcContext->Kill();
	return bPassed;
}


//////////////////////////////////////////////////////////////////////////
//
// One assert per file: "xx.n: 200/200" or the first failure.
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C816StepFile(char* szName, uint8 uiOpcode, CTestFast65C816StepContext* pcContext)
{
	CChars				szFilename;
	CChars				szExpected;
	CChars				szActual;
	CChars				szError;
	CChars				szTestError;
	CFileUtil			cFileUtil;
	CDiskFile			cDisk;
	CFileBasic			cFile;
	uint8*				puiData;
	size				uiSize;
	CSingleStepReader	cReader;
	uint16				uiTests;
	uint16				i;
	uint16				uiPassed;
	uint16				uiFirstFailure;

	szFilename.Init();
	cFileUtil.CurrentDirectory(&szFilename);
	cFileUtil.AppendToPath(&szFilename, "Input");
	cFileUtil.AppendToPath(&szFilename, "SingleStep");
	cFileUtil.AppendToPath(&szFilename, szName);
	szFilename.Append(".bin");

	cDisk.Init(szFilename);
	cFile.Init(&cDisk);
	if (!cFile.Open(EFM_Read))
	{
		cFile.Kill();
		cDisk.Kill();
		szActual.Init(szName);
		szActual.Append(": missing ");
		szActual.Append(szFilename);
		AssertString(szName, szActual.Text());
		szActual.Kill();
		szFilename.Kill();
		return;
	}

	uiSize = (size)cDisk.Size();
	puiData = (uint8*)malloc(uiSize);
	cFile.Read(puiData, uiSize, 1);
	cFile.Close();
	cFile.Kill();
	cDisk.Kill();

	cReader.Init(puiData);
	cReader.Skip(4);
	cReader.ReadShort();
	uiTests = cReader.ReadShort();

	uiPassed = 0;
	uiFirstFailure = 0;
	szError.Init();
	szTestError.Init();
	for (i = 0; i < uiTests; i++)
	{
		szTestError.Clear();
		if (TestFast65C816StepRunTest(&cReader, &szTestError, uiOpcode, pcContext))
		{
			uiPassed++;
		}
		else if (uiFirstFailure == 0)
		{
			uiFirstFailure = i + 1;
			szError.Append(szTestError);
		}
	}
	szTestError.Kill();
	free(puiData);

	szExpected.Init(szName);
	szExpected.Append(": ");
	szExpected.Append((int)uiTests);
	szExpected.Append('/');
	szExpected.Append((int)uiTests);

	szActual.Init(szName);
	szActual.Append(": ");
	szActual.Append((int)uiPassed);
	szActual.Append('/');
	szActual.Append((int)uiTests);
	if (uiPassed != uiTests)
	{
		szActual.Append(" first failure test ");
		szActual.Append((int)uiFirstFailure);
		szActual.Append(": ");
		szActual.Append(szError);
	}
	AssertString(szExpected.Text(), szActual.Text());

	szError.Kill();
	szActual.Kill();
	szExpected.Kill();
	szFilename.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFast65C816SingleStep(void)
{
	int							iOpcode;
	char						szName[8];
	CTestFast65C816StepContext*	pcContext;

	pcContext = (CTestFast65C816StepContext*)malloc(sizeof(CTestFast65C816StepContext));
	for (iOpcode = 0; iOpcode < 256; iOpcode++)
	{
		snprintf(szName, sizeof(szName), "%02x.n", iOpcode);
		TestFast65C816StepFile(szName, (uint8)iOpcode, pcContext);
		snprintf(szName, sizeof(szName), "%02x.e", iOpcode);
		TestFast65C816StepFile(szName, (uint8)iOpcode, pcContext);
	}
	free(pcContext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint64 TestFast65C816Random(uint64* puiSeed)
{
	//xorshift64*.
	*puiSeed ^= *puiSeed >> 12;
	*puiSeed ^= *puiSeed << 25;
	*puiSeed ^= *puiSeed >> 27;
	return *puiSeed * 0x2545F4914F6CDD1DULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Any value, small values or edge values.
//
//////////////////////////////////////////////////////////////////////////
static uint16 TestFast65C816RandomRegister(uint64* puiSeed)
{
	static uint16	auiEdges[8] = { 0, 0xFFFF, 0x8000, 0x7FFF, 0x80, 0xFF, 0x7F, 0x100 };
	uint64			uiRandom;

	uiRandom = TestFast65C816Random(puiSeed);
	switch (uiRandom & 3)
	{
	case 0:
	case 1:
		return (uint16)(uiRandom >> 16);
	case 2:
		return (uint16)((uiRandom >> 16) & 0x1F);
	default:
		return auiEdges[(uiRandom >> 16) & 7];
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The simulator's memory.  Every cycle is counted, memory only answers VDA, VPA or VPB.
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C816FuzzTickHigh(CW65C816Meta* pcMPU, void* pvMemory)
{
	STestFast65C816FuzzMemory*	psMemory;
	uint32						uiAddress;

	psMemory = (STestFast65C816FuzzMemory*)pvMemory;
	psMemory->muiCycles++;
	if (!pcMPU->GetVPA()->IsHigh() && !pcMPU->GetVDA()->IsHigh() && !pcMPU->GetVPB()->IsLow())
	{
		return;
	}

	uiAddress = pcMPU->GetAddress()->Get() & (TEST_FAST_65C816_FUZZ_SIZE - 1);
	if (pcMPU->GetRWB()->IsHigh())
	{
		pcMPU->GetData()->Set(psMemory->mauiBytes[uiAddress]);
	}
	else
	{
		psMemory->mauiBytes[uiAddress] = (uint8)pcMPU->GetData()->Get();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C816FuzzTickLow(CW65C816Meta* pcMPU, void* pvMemory)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint8 TestFast65C816FuzzRead(void* pvMemory, uint32 uiAddress)
{
	return ((STestFast65C816FuzzMemory*)pvMemory)->mauiBytes[uiAddress & (TEST_FAST_65C816_FUZZ_SIZE - 1)];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C816FuzzWrite(void* pvMemory, uint32 uiAddress, uint8 uiValue)
{
	((STestFast65C816FuzzMemory*)pvMemory)->mauiBytes[uiAddress & (TEST_FAST_65C816_FUZZ_SIZE - 1)] = uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C816AppendState(CChars* psz, CW65C816* pcSim)
{
	uint64	uiValue;

	psz->Append(" C 0x");
	uiValue = pcSim->GetC();
	psz->AppendHexHiLo(&uiValue, 2);
	psz->Append(" X 0x");
	uiValue = pcSim->GetX();
	psz->AppendHexHiLo(&uiValue, 2);
	psz->Append(" Y 0x");
	uiValue = pcSim->GetY();
	psz->AppendHexHiLo(&uiValue, 2);
	psz->Append(" S 0x");
	uiValue = pcSim->GetStackPointer16Bit();
	psz->AppendHexHiLo(&uiValue, 2);
	psz->Append(" D 0x");
	uiValue = pcSim->GetDirectPage();
	psz->AppendHexHiLo(&uiValue, 2);
	psz->Append(" DBR 0x");
	uiValue = pcSim->GetDataBank();
	psz->AppendHexHiLo(&uiValue, 1);
	psz->Append(" PC 0x");
	uiValue = pcSim->GetProgramCounter()->GetBank();
	psz->AppendHexHiLo(&uiValue, 1);
	psz->Append(":");
	uiValue = pcSim->GetProgramCounter()->GetOffset();
	psz->AppendHexHiLo(&uiValue, 2);
	psz->Append(" P 0x");
	uiValue = pcSim->GetProcessorRegisterValue();
	psz->AppendHexHiLo(&uiValue, 1);
	psz->Append(pcSim->IsEmulation() ? " E" : " N");
}


//////////////////////////////////////////////////////////////////////////
//
// Both start an instruction at the same random state.
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C816SetState(CW65C816Meta* pcMeta, CFast65C816* pcFast, uint64* puiSeed, bool bEmulation, uint8 uiProgramBank, uint16 uiCode)
{
	CW65C816*	pcSim;
	uint8		uiP;
	uint16		uiC;
	uint16		uiX;
	uint16		uiY;
	uint16		uiS;
	uint16		uiD;
	uint8		uiDataBank;
	uint64		uiRandom;

	pcSim = pcMeta->GetMPU();
	uiRandom = TestFast65C816Random(puiSeed);
	uiP = (uint8)uiRandom;
	uiDataBank = (uint8)(uiRandom >> 8);
	uiC = TestFast65C816RandomRegister(puiSeed);
	uiX = TestFast65C816RandomRegister(puiSeed);
	uiY = TestFast65C816RandomRegister(puiSeed);
	uiS = TestFast65C816RandomRegister(puiSeed);
	uiD = TestFast65C816RandomRegister(puiSeed);
	if ((uiRandom >> 16) & 1)
	{
		uiD &= 0xFF00;
	}
	if (bEmulation)
	{
		uiS = (uint16)(0x100 | (uiS & 0xFF));
		uiP |= STATUS_ACCUMULATOR_WIDTH | STATUS_INDEX_WIDTH;
	}
	if (uiP & STATUS_INDEX_WIDTH)
	{
		uiX &= 0xFF;
		uiY &= 0xFF;
	}

	pcSim->TestSetProcessorStatus(uiP, bEmulation);
	pcSim->TestSetC(uiC);
	pcSim->TestSetX(uiX);
	pcSim->TestSetY(uiY);
	pcSim->TestSetStackPointer(uiS);
	pcSim->TestSetDirectPage(uiD);
	pcSim->TestSetDataBank(uiDataBank);
	pcSim->TestSetProgramCounter(uiProgramBank, uiCode);
	pcSim->TestStartInstruction();
	pcMeta->TestTickCycle();

	pcFast->TestSetP(uiP, bEmulation);
	pcFast->TestSetC(uiC);
	pcFast->TestSetX(uiX);
	pcFast->TestSetY(uiY);
	pcFast->TestSetS(uiS);
	pcFast->TestSetD(uiD);
	pcFast->TestSetDataBank(uiDataBank);
	pcFast->TestSetProgramCounter(uiProgramBank, uiCode);
}


//////////////////////////////////////////////////////////////////////////
//
// Every opcode in native and emulation mode from random states, for a few steps (the instructions after
// it are random too).  With bFlat CFast65C816 reads and writes bank 0 directly, otherwise it goes through
// the callbacks for everything.
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C816Fuzz(bool bFlat, uint64 uiSeed)
{
	STestFast65C816FuzzMemory*	psSimMemory;
	STestFast65C816FuzzMemory*	psFastMemory;
	CW65C816Meta				cMeta;
	CFast65C816					cFast;
	CW65C816*					pcSim;
	uint32						uiOpcode;
	uint32						uiTrial;
	uint16						uiCode;
	uint8						uiProgramBank;
	size						i;
	size						uiStep;
	bool						bEmulation;
	bool						bSim;
	bool						bFast;
	CChars						sz;
	CChars						szWhere;
	CChars						szBefore;
	uint8						uiExecuted;
	uint64						uiFastCycles;
	size						uiSteps;

	psSimMemory = (STestFast65C816FuzzMemory*)malloc(sizeof(STestFast65C816FuzzMemory));
	psFastMemory = (STestFast65C816FuzzMemory*)malloc(sizeof(STestFast65C816FuzzMemory));

	uiSteps = 0;
	for (uiOpcode = 0; uiOpcode < 0x200; uiOpcode++)
	{
		bEmulation = (uiOpcode & 0x100) != 0;
		for (uiTrial = 0; uiTrial < TEST_FAST_65C816_FUZZ_TRIALS; uiTrial++)
		{
			for (i = 0; i < TEST_FAST_65C816_FUZZ_SIZE; i += 8)
			{
				*((uint64*)&psSimMemory->mauiBytes[i]) = TestFast65C816Random(&uiSeed);
			}
			uiCode = (uint16)TestFast65C816Random(&uiSeed);
			uiProgramBank = (uiTrial & 1) ? (uint8)TestFast65C816Random(&uiSeed) : 0;
			psSimMemory->mauiBytes[uiCode] = (uint8)uiOpcode;
			memcpy(psFastMemory->mauiBytes, psSimMemory->mauiBytes, TEST_FAST_65C816_FUZZ_SIZE);

			//New processors for each trial, the simulator starts between instructions.
			if (bFlat)
			{
				cFast.Init(TestFast65C816FuzzRead, TestFast65C816FuzzWrite, psFastMemory, psFastMemory->mauiBytes, TEST_FAST_65C816_FUZZ_SIZE);
			}
			else
			{
				cFast.Init(TestFast65C816FuzzRead, TestFast65C816FuzzWrite, psFastMemory, NULL, 0);
			}
			cMeta.Init(TestFast65C816FuzzTickHigh, TestFast65C816FuzzTickLow, psSimMemory);
			cMeta.GetRESB()->Set(true);
			pcSim = cMeta.GetMPU();
			TestFast65C816SetState(&cMeta, &cFast, &uiSeed, bEmulation, uiProgramBank, uiCode);

			for (uiStep = 0; uiStep < TEST_FAST_65C816_FUZZ_STEPS; uiStep++)
			{
				psSimMemory->muiCycles = 0;
				uiExecuted = psSimMemory->mauiBytes[pcSim->GetProgramCounter()->GetOffset()];
				szBefore.Init(" before:");
				TestFast65C816AppendState(&szBefore, pcSim);
				uiFastCycles = cFast.NumCycles();
				bSim = cMeta.TickInstruction();
				bFast = cFast.Step();
				uiFastCycles = cFast.NumCycles() - uiFastCycles;
				uiSteps++;

				sz.Init();
				cFast.AppendDifferences(pcSim, &sz);
				cFast.AppendDifference(&sz, "Running", bSim, bFast);
				cFast.AppendDifference(&sz, "Cycles", psSimMemory->muiCycles, uiFastCycles);
				if (memcmp(psSimMemory->mauiBytes, psFastMemory->mauiBytes, TEST_FAST_65C816_FUZZ_SIZE) != 0)
				{
					sz.Append(" Memory differs");
				}
				if (!sz.Empty())
				{
					szWhere.Init("Opcode 0x");
					szWhere.AppendHexHiLo(&uiOpcode, 2);
					szWhere.Append(" trial ");
					szWhere.Append((int)uiTrial);
					szWhere.Append(" step ");
					szWhere.Append((int)uiStep);
					szWhere.Append(" ran 0x");
					szWhere.AppendHexHiLo(&uiExecuted, 1);
					sz.Insert(0, &szWhere);
					sz.Append(szBefore);
					szWhere.Kill();
				}
				if (TestFast65C816Failed(&sz))
				{
					uiStep = TEST_FAST_65C816_FUZZ_STEPS;
				}
				sz.Kill();
				szBefore.Kill();

				if (!bSim || !bFast || cFast.IsWaiting())
				{
					break;
				}
			}
			cMeta.Kill();
			cFast.Kill();
		}
	}
	AssertSize(0, gsTestFast65C816Failures);
	AssertTrue(uiSteps > 0x200 * TEST_FAST_65C816_FUZZ_TRIALS);

	free(psFastMemory);
	free(psSimMemory);
}


//////////////////////////////////////////////////////////////////////////
//
// One step on both, with IRQB and NMIB as given before it.  Returns false if they differ.
//
//////////////////////////////////////////////////////////////////////////
static bool TestFast65C816InterruptStep(CW65C816Meta* pcMeta, CFast65C816* pcFast, STestFast65C816FuzzMemory* psSimMemory, STestFast65C816FuzzMemory* psFastMemory, bool bIrqLow, bool bNmiLow, size uiStep)
{
	bool	bSim;
	bool	bFast;
	uint64	uiFastCycles;
	CChars	sz;
	CChars	szWhere;
	bool	bFailed;

	pcMeta->GetIRQB()->Set(!bIrqLow);
	pcMeta->GetNMIB()->Set(!bNmiLow);
	pcFast->SetIRQ(bIrqLow);
	pcFast->SetNMI(bNmiLow);

	psSimMemory->muiCycles = 0;
	uiFastCycles = pcFast->NumCycles();
	bSim = pcMeta->TickInstruction();
	bFast = pcFast->Step();
	uiFastCycles = pcFast->NumCycles() - uiFastCycles;

	sz.Init();
	pcFast->AppendDifferences(pcMeta->GetMPU(), &sz);
	pcFast->AppendDifference(&sz, "Running", bSim, bFast);
	pcFast->AppendDifference(&sz, "Cycles", psSimMemory->muiCycles, uiFastCycles);
	if (memcmp(psSimMemory->mauiBytes, psFastMemory->mauiBytes, TEST_FAST_65C816_FUZZ_SIZE) != 0)
	{
		sz.Append(" Memory differs");
	}
	if (!sz.Empty())
	{
		szWhere.Init("Interrupt step ");
		szWhere.Append((int)uiStep);
		sz.Insert(0, &szWhere);
		szWhere.Kill();
	}
	bFailed = TestFast65C816Failed(&sz);
	sz.Kill();
	return !bFailed;
}


//////////////////////////////////////////////////////////////////////////
//
// IRQ, NMI, BRK, COP, RTI and WAI in native and emulation mode, with the inputs changed between steps.
//
//////////////////////////////////////////////////////////////////////////
static void TestFast65C816Interrupts(bool bEmulation)
{
	STestFast65C816FuzzMemory*	psSimMemory;
	STestFast65C816FuzzMemory*	psFastMemory;
	CW65C816Meta				cMeta;
	CFast65C816					cFast;
	uint8*						puiCode;
	uint8*						puiHandler;
	size						uiStep;
	size						i;
	bool						bIrq;
	bool						bNmi;
	static uint8				auiProgram[] = {
		0x58,				//CLI
		0xEA,				//NOP
		0xEA,				//NOP
		0xEA,				//NOP
		0x00, 0x11,			//BRK
		0x02, 0x22,			//COP
		0xCB,				//WAI
		0xEA,				//NOP
		0x78,				//SEI
		0xCB,				//WAI
		0xEA,				//NOP
		0x58,				//CLI
		0xEA,				//NOP
		0xEA,				//NOP
		0xEA,				//NOP
		0x80, 0xED };		//BRA to the first NOP
	static uint8				auiHandler[] = {
		0xE8,				//INX
		0x48,				//PHA
		0x68,				//PLA
		0x40 };				//RTI

	psSimMemory = (STestFast65C816FuzzMemory*)malloc(sizeof(STestFast65C816FuzzMemory));
	psFastMemory = (STestFast65C816FuzzMemory*)malloc(sizeof(STestFast65C816FuzzMemory));
	memset(psSimMemory->mauiBytes, 0xEA, TEST_FAST_65C816_FUZZ_SIZE);
	puiCode = &psSimMemory->mauiBytes[0x1000];
	puiHandler = &psSimMemory->mauiBytes[0x2000];
	memcpy(puiCode, auiProgram, sizeof(auiProgram));
	memcpy(puiHandler, auiHandler, sizeof(auiHandler));
	for (i = 0xFFE0; i < 0x10000; i += 2)
	{
		psSimMemory->mauiBytes[i] = 0x00;
		psSimMemory->mauiBytes[i + 1] = 0x20;
	}
	memcpy(psFastMemory->mauiBytes, psSimMemory->mauiBytes, TEST_FAST_65C816_FUZZ_SIZE);

	cMeta.Init(TestFast65C816FuzzTickHigh, TestFast65C816FuzzTickLow, psSimMemory);
	cMeta.GetRESB()->Set(true);
	cFast.Init(TestFast65C816FuzzRead, TestFast65C816FuzzWrite, psFastMemory, psFastMemory->mauiBytes, TEST_FAST_65C816_FUZZ_SIZE);

	cMeta.GetMPU()->TestSetProcessorStatus(STATUS_INTERRUPT_DISABLE | STATUS_ACCUMULATOR_WIDTH | STATUS_INDEX_WIDTH, bEmulation);
	cMeta.GetMPU()->TestSetStackPointer(0x1F0);
	cMeta.GetMPU()->TestSetProgramCounter(0, 0x1000);
	cMeta.GetMPU()->TestStartInstruction();
	cMeta.TestTickCycle();
	cFast.TestSetP(STATUS_INTERRUPT_DISABLE | STATUS_ACCUMULATOR_WIDTH | STATUS_INDEX_WIDTH, bEmulation);
	cFast.TestSetS(0x1F0);
	cFast.TestSetProgramCounter(0, 0x1000);

	//IRQ held low over the NOPs, released by the handler's PLA; NMI pulsed; WAI woken by IRQ with I clear
	//and with I set.  NMI held low is taken once (steps 50 to 61), wakes WAI (80) and then leaves the next WAI
	//waiting until it falls again (110).
	for (uiStep = 0; uiStep < 128; uiStep++)
	{
		bIrq = ((uiStep >= 2) && (uiStep < 4)) || ((uiStep >= 14) && (uiStep < 16)) || ((uiStep >= 24) && (uiStep < 26)) || ((uiStep >= 40) && (uiStep < 41));
		bNmi = (uiStep == 8) || (uiStep == 30) || ((uiStep >= 50) && (uiStep < 62)) || ((uiStep >= 80) && (uiStep < 100)) || ((uiStep >= 110) && (uiStep < 113));
		if (!TestFast65C816InterruptStep(&cMeta, &cFast, psSimMemory, psFastMemory, bIrq, bNmi, uiStep))
		{
			break;
		}
	}

	cFast.Kill();
	cMeta.Kill();
	free(psFastMemory);
	free(psSimMemory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFast65C816(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	CW65C816InstructionFactory::GetInstance()->Init();
	gsTestFast65C816Failures = 0;

	TestFast65C816SingleStep();
	TestFast65C816Fuzz(true, 0x65C816ULL);
	TestFast65C816Fuzz(false, 0x816816ULL);
	TestFast65C816Interrupts(false);
	TestFast65C816Interrupts(true);

	CW65C816InstructionFactory::GetInstance()->Kill();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

