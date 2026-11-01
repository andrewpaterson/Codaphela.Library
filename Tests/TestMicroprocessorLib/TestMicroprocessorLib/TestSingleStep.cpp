#include <stdio.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "TestLib/Assert.h"
#include "TestSingleStep.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSingleStepRecordCycle(CW65CMeta* pcMPU, CTestSingleStepContext* pcContext, uint8 uiSignals, bool bEnabled)
{
	//Records the bus cycle and acts as the memory.  The tests only enable memory when
	//bEnabled (VDA, VPA or VPB for the W65C816S).
	SSingleStepCycle*	psCycle;
	bool				bRead;

	if (pcContext->muiCycles >= SINGLE_STEP_MAX_CYCLES)
	{
		return;
	}

	psCycle = &pcContext->masCycles[pcContext->muiCycles];
	pcContext->muiCycles++;

	psCycle->uiAddress = pcMPU->GetAddress()->Get() | (pcMPU->GetBank() << 16);
	psCycle->uiSignals = uiSignals;
	psCycle->uiValid = SINGLE_STEP_ADDRESS_VALID;

	bRead = (uiSignals & SINGLE_STEP_READ) != 0;
	if (bRead)
	{
		if (bEnabled)
		{
			psCycle->uiValue = pcContext->GetByte(psCycle->uiAddress);
			psCycle->uiValid |= SINGLE_STEP_VALUE_VALID;
			pcMPU->GetData()->Set(psCycle->uiValue);
		}
		else
		{
			psCycle->uiValue = 0;
		}
	}
	else
	{
		psCycle->uiValue = pcMPU->GetData()->Get();
		psCycle->uiValid |= SINGLE_STEP_VALUE_VALID;
		if (bEnabled)
		{
			pcContext->SetByte(psCycle->uiAddress, psCycle->uiValue);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSingleStepReadState(CSingleStepReader* pcReader, SSingleStepState* psState)
{
	psState->uiPC = pcReader->ReadShort();
	psState->uiS = pcReader->ReadShort();
	psState->uiP = pcReader->ReadByte();
	psState->uiA = pcReader->ReadShort();
	psState->uiX = pcReader->ReadShort();
	psState->uiY = pcReader->ReadShort();
	psState->uiDBR = pcReader->ReadByte();
	psState->uiD = pcReader->ReadShort();
	psState->uiPBR = pcReader->ReadByte();
	psState->uiE = pcReader->ReadByte();
	psState->uiRamCount = pcReader->ReadShort();
	psState->puiRam = pcReader->GetPos();
	pcReader->Skip(psState->uiRamCount * 4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSingleStepAppendHex(CChars* psz, uint32 uiValue, int iDigits)
{
	char	sz[16];

	snprintf(sz, sizeof(sz), "%0*x", iDigits, uiValue);
	psz->Append(sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSingleStepAppendCycle(CChars* psz, SSingleStepCycle* psCycle)
{
	char	szSignals[9];

	if (psCycle->uiValid & SINGLE_STEP_ADDRESS_VALID)
	{
		TestSingleStepAppendHex(psz, psCycle->uiAddress, 6);
	}
	else
	{
		psz->Append("------");
	}
	psz->Append(" ");
	if (psCycle->uiValid & SINGLE_STEP_VALUE_VALID)
	{
		TestSingleStepAppendHex(psz, psCycle->uiValue, 2);
	}
	else
	{
		psz->Append("--");
	}

	szSignals[0] = (psCycle->uiSignals & SINGLE_STEP_VDA) ? 'd' : '-';
	szSignals[1] = (psCycle->uiSignals & SINGLE_STEP_VPA) ? 'p' : '-';
	szSignals[2] = (psCycle->uiSignals & SINGLE_STEP_VPB) ? 'v' : '-';
	szSignals[3] = (psCycle->uiSignals & SINGLE_STEP_READ) ? 'r' : 'w';
	szSignals[4] = (psCycle->uiSignals & SINGLE_STEP_E) ? 'e' : '-';
	szSignals[5] = (psCycle->uiSignals & SINGLE_STEP_M) ? 'm' : '-';
	szSignals[6] = (psCycle->uiSignals & SINGLE_STEP_X) ? 'x' : '-';
	szSignals[7] = (psCycle->uiSignals & SINGLE_STEP_MLB) ? 'l' : '-';
	szSignals[8] = '\0';
	psz->Append(" ");
	psz->Append(szSignals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool TestSingleStepCompareRegister(CChars* pszError, char* szName, uint32 uiExpected, uint32 uiActual, int iDigits)
{
	if (uiExpected == uiActual)
	{
		return true;
	}
	pszError->Append(szName);
	pszError->Append(" expected ");
	TestSingleStepAppendHex(pszError, uiExpected, iDigits);
	pszError->Append(" actual ");
	TestSingleStepAppendHex(pszError, uiActual, iDigits);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool TestSingleStepCompareCycles(CChars* pszError, SSingleStepCycle* pasExpected, size uiExpected, CTestSingleStepContext* pcContext, uint8 uiSignalMask)
{
	size				i;
	SSingleStepCycle*	psExpected;
	SSingleStepCycle*	psActual;
	bool				bSame;

	for (i = 0; i < uiExpected; i++)
	{
		psExpected = &pasExpected[i];
		if (!(psExpected->uiValid & SINGLE_STEP_ADDRESS_VALID))
		{
			//The bus is floating (after WAI or STP).
			continue;
		}

		psActual = &pcContext->masCycles[i];
		bSame = (psExpected->uiAddress == psActual->uiAddress) && ((psExpected->uiSignals & uiSignalMask) == (psActual->uiSignals & uiSignalMask));
		if (psExpected->uiValid & SINGLE_STEP_VALUE_VALID)
		{
			bSame = bSame && (psActual->uiValid & SINGLE_STEP_VALUE_VALID) && (psExpected->uiValue == psActual->uiValue);
		}

		if (!bSame)
		{
			pszError->Append("cycle ");
			pszError->Append((int)i + 1);
			pszError->Append(" expected [");
			TestSingleStepAppendCycle(pszError, psExpected);
			pszError->Append("] actual [");
			TestSingleStepAppendCycle(pszError, psActual);
			pszError->Append("]");
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool TestSingleStepCompareState(CChars* pszError, SSingleStepState* psExpected, CW65CCore* pcCPU, CTestSingleStepContext* pcContext, uint8 uiStatusMask)
{
	CSingleStepReader	cReader;
	uint16				i;
	uint32				uiAddress;
	uint8				uiValue;

	if (!TestSingleStepCompareRegister(pszError, "E", psExpected->uiE, pcCPU->IsEmulation() ? 1 : 0, 1)) return false;
	if (!TestSingleStepCompareRegister(pszError, "P", psExpected->uiP & uiStatusMask, pcCPU->GetProcessorRegisterValue() & uiStatusMask, 2)) return false;
	if (!TestSingleStepCompareRegister(pszError, "PBR", psExpected->uiPBR, pcCPU->GetProgramCounter()->GetBank(), 2)) return false;
	if (!TestSingleStepCompareRegister(pszError, "PC", psExpected->uiPC, pcCPU->GetProgramCounter()->GetOffset(), 4)) return false;
	if (!TestSingleStepCompareRegister(pszError, "A", psExpected->uiA, pcCPU->GetC(), 4)) return false;
	if (!TestSingleStepCompareRegister(pszError, "X", psExpected->uiX, pcCPU->GetX(), 4)) return false;
	if (!TestSingleStepCompareRegister(pszError, "Y", psExpected->uiY, pcCPU->GetY(), 4)) return false;
	if (!TestSingleStepCompareRegister(pszError, "S", psExpected->uiS, pcCPU->GetStackPointer16Bit(), 4)) return false;
	if (!TestSingleStepCompareRegister(pszError, "D", psExpected->uiD, pcCPU->GetDirectPage(), 4)) return false;
	if (!TestSingleStepCompareRegister(pszError, "DBR", psExpected->uiDBR, pcCPU->GetDataBank(), 2)) return false;

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
void TestSingleStepAppendTest(CChars* psz, SSingleStepState* psInitial, SSingleStepCycle* pasExpected, size uiExpected, CTestSingleStepContext* pcContext)
{
	size	i;

	psz->Append("\n  initial E.");
	TestSingleStepAppendHex(psz, psInitial->uiE, 1);
	psz->Append(" P.");
	TestSingleStepAppendHex(psz, psInitial->uiP, 2);
	psz->Append(" PC.");
	TestSingleStepAppendHex(psz, psInitial->uiPBR, 2);
	psz->Append(":");
	TestSingleStepAppendHex(psz, psInitial->uiPC, 4);
	psz->Append(" A.");
	TestSingleStepAppendHex(psz, psInitial->uiA, 4);
	psz->Append(" X.");
	TestSingleStepAppendHex(psz, psInitial->uiX, 4);
	psz->Append(" Y.");
	TestSingleStepAppendHex(psz, psInitial->uiY, 4);
	psz->Append(" S.");
	TestSingleStepAppendHex(psz, psInitial->uiS, 4);
	psz->Append(" D.");
	TestSingleStepAppendHex(psz, psInitial->uiD, 4);
	psz->Append(" DBR.");
	TestSingleStepAppendHex(psz, psInitial->uiDBR, 2);

	for (i = 0; (i < uiExpected) || (i < pcContext->muiCycles); i++)
	{
		psz->Append("\n  ");
		if (i < uiExpected)
		{
			TestSingleStepAppendCycle(psz, &pasExpected[i]);
		}
		else
		{
			psz->Append("                ");
		}
		psz->Append("  |  ");
		if (i < pcContext->muiCycles)
		{
			TestSingleStepAppendCycle(psz, &pcContext->masCycles[i]);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool TestSingleStepRunTest(CSingleStepProcessor* pcProcessor, CSingleStepReader* pcReader, CChars* pszError, bool bWaitOrStop)
{
	SSingleStepState				sInitial;
	SSingleStepState				sFinal;
	SSingleStepCycle				asExpected[SINGLE_STEP_MAX_CYCLES];
	uint16							uiCycles;
	uint16							i;
	CTestSingleStepContext			cContext;
	CW65CMeta*						pcMPU;
	CW65CCore*						pcCPU;
	CSingleStepReader				cRam;
	uint32							uiAddress;
	bool							bOpcodeFetch;
	bool							bPassed;

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

	cContext.Init();
	cRam.Init(sInitial.puiRam);
	for (i = 0; i < sInitial.uiRamCount; i++)
	{
		uiAddress = cRam.ReadAddress();
		cContext.SetByte(uiAddress, cRam.ReadByte());
	}

	pcProcessor->Init(&cContext);
	pcMPU = pcProcessor->GetMeta();
	pcMPU->GetRESB()->Set(true);

	pcCPU = pcProcessor->GetCPU();
	pcCPU->TestSetProcessorStatus(sInitial.uiP, sInitial.uiE != 0);
	pcCPU->TestSetC(sInitial.uiA);
	pcCPU->TestSetX(sInitial.uiX);
	pcCPU->TestSetY(sInitial.uiY);
	pcCPU->TestSetStackPointer(sInitial.uiS);
	pcProcessor->SetState(&sInitial);
	pcCPU->TestSetProgramCounter(sInitial.uiPBR, sInitial.uiPC);
	pcCPU->TestStartInstruction();

	//The first cycle only sets up the opcode fetch, the bus is not driven while it is high.
	pcMPU->TestTickCycle();
	cContext.muiCycles = 0;

	bOpcodeFetch = false;
	for (i = 0; i < uiCycles; i++)
	{
		bOpcodeFetch = pcMPU->TestTickCycle();
	}

	bPassed = TestSingleStepCompareCycles(pszError, asExpected, uiCycles, &cContext, pcProcessor->GetSignalMask());
	if (bPassed && !bWaitOrStop && !bOpcodeFetch)
	{
		pszError->Append("instruction is longer than ");
		pszError->Append((int)uiCycles);
		pszError->Append(" cycles");
		bPassed = false;
	}
	if (bPassed)
	{
		bPassed = TestSingleStepCompareState(pszError, &sFinal, pcCPU, &cContext, pcProcessor->GetStatusMask());
	}

	if (!bPassed)
	{
		TestSingleStepAppendTest(pszError, &sInitial, asExpected, uiCycles, &cContext);
	}

	pcProcessor->Kill();
	cContext.Kill();
	return bPassed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSingleStepFile(CSingleStepProcessor* pcProcessor, char* szDirectory, char* szName, uint8 uiOpcode)
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
	bool				bWaitOrStop;

	szFilename.Init();
	cFileUtil.CurrentDirectory(&szFilename);
	cFileUtil.AppendToPath(&szFilename, "Input");
	cFileUtil.AppendToPath(&szFilename, szDirectory);
	cFileUtil.AppendToPath(&szFilename, szName);
	szFilename.Append(".bin");

	szExpected.Init();
	szActual.Init();
	szError.Init();

	cDisk.Init(szFilename);
	cFile.Init(&cDisk);
	if (!cFile.Open(EFM_Read))
	{
		cFile.Kill();
		cDisk.Kill();
		szActual.Append(szName);
		szActual.Append(": missing ");
		szActual.Append(szFilename);
		AssertString(szName, szActual.Text());
		szActual.Kill();
		szExpected.Kill();
		szError.Kill();
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
	cReader.Skip(4);  //"S816" or "SC02"
	cReader.ReadShort();  //Version.
	uiTests = cReader.ReadShort();

	bWaitOrStop = pcProcessor->IsWaitOrStop(uiOpcode);
	uiPassed = 0;
	uiFirstFailure = 0;
	szTestError.Init();
	for (i = 0; i < uiTests; i++)
	{
		szTestError.Clear();
		if (TestSingleStepRunTest(pcProcessor, &cReader, &szTestError, bWaitOrStop))
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

	//One assert per file: "xx.n: 200/200" or the first failure.
	szExpected.Append(szName);
	szExpected.Append(": ");
	szExpected.Append((int)uiTests);
	szExpected.Append('/');
	szExpected.Append((int)uiTests);

	szActual.Append(szName);
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

