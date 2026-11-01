#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "MicroprocessorLib/W65C816InstructionFactory.h"
#include "TestLib/Assert.h"
#include "TestW65C816Context.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestW65C816Context::Init(uint32 uiMemorySize, uint8 uiFill)
{
	mauiMemory = (uint8*)malloc(uiMemorySize);
	mszSequence.Init();
	muiMemorySize = uiMemorySize;
	muiFill = uiFill;
	memset(mauiMemory, uiFill, uiMemorySize);

	mbPrintReset = false;
	mbPrintMnemonic = false;
	mbPrintCycle = false;
	mbPrintOperation = false;
	mbPrintA = false;
	mbPrintX = false;
	mbPrintY = false;
	mbPrintPC = false;
	mbPrintS = false;
	mbPrintDP = false;
	mbPrintDB = false;
	mbPrintP = false;
	muiStartOfPreviousLine = 0;
	muiStartOfPrevPrevLine = 0;
	muiLastAddress = 0;
	muiInstructions = 0;
	muiMaxInstructions = 0;

	mbPrintAddressBus = false;
	mbPrintDataBus = false;
	mbPrintBank = false;
	mbPrintVPB = false;
	mbPrintRDY = false;
	mbPrintMLB = false;
	mbPrintVPA = false;
	mbPrintVDA = false;
	mbPrintWM = false;
	mbPrintWX = false;
	mbPrintE = false;
	mbPrintRWB = false;

	mbAddressOperation = false;
	mbDataOperation = false;
	mbDescription = false;

	mbPrintSet = false;
	mbEnablePrint = true;
	mbDebugSpew = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestW65C816Context::Init(uint32 uiMemorySize, uint8 uiFill, uint16 uiResetAddress)
{
	Init(uiMemorySize, uiFill);

	SetByte(0xfffc, GetLowByte(uiResetAddress));
	SetByte(0xfffd, GetHighByte(uiResetAddress));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestW65C816Context::Init(char* szMemoryFileame)
{
	CChars		sz;
	CFileUtil	cFileUtil;
	CFileBasic	cFile;
	CDiskFile	cDisk;
	uint32		uiSize;
	bool		bResult;

	sz.Init();

	cFileUtil.CurrentDirectory(&sz);
	cFileUtil.AppendToPath(&sz, "Input");
	cFileUtil.AppendToPath(&sz, szMemoryFileame);

	cDisk.Init(sz);
	sz.Kill();
	cFile.Init(&cDisk);
	bResult = cFile.Open(EFM_Read);
	if (!bResult)
	{
		cDisk.Kill();
		cFile.Kill();
		return;
	}

	uiSize = (uint32)cDisk.Size();
	
	Init(uiSize, 0xff);

	uiSize = (uint32)cFile.Read(mauiMemory, uiSize, 1);
	cFile.Close();
	cDisk.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestW65C816Context::Kill(void)
{
	muiLastAddress = 0;
	mszSequence.Kill();
	free(mauiMemory);
	mauiMemory = NULL;
	mbPrintSet = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestW65C816Context::SetPrint(bool bReset, bool bStop, bool bMnemonic, bool bCycle, bool bOperation, bool bA, bool bX, bool bY, bool bPC, bool bS, bool bDP, bool bDB, bool bP, bool mbMerged)
{
	if (!mbPrintSet)
	{
		mbPrintReset = bReset;
		mbPrintStop = bStop;
		mbPrintMnemonic = bMnemonic;
		mbPrintCycle = bCycle;
		mbPrintOperation = bOperation;
		mbPrintA = bA;
		mbPrintX = bX;
		mbPrintY = bY;
		mbPrintPC = bPC;
		mbPrintS = bS;
		mbPrintDP = bDP;
		mbPrintDB = bDB;
		mbPrintP = bP;
		mbPrintMergedCycles = mbMerged;

		mbPrintSet = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestW65C816Context::SetSignalPrint(bool bAddressBus, bool bDataBus, bool bBank, bool bVPB, bool bRDY, bool bMLB, bool bVPA, bool bVDA, bool bWM, bool bWX, bool bE, bool bRWB)
{
	mbPrintAddressBus = bAddressBus;
	mbPrintDataBus = bDataBus;
	mbPrintBank = bBank;
	mbPrintVPB = bVPB;
	mbPrintRDY = bRDY;
	mbPrintMLB = bMLB;
	mbPrintVPA = bVPA;
	mbPrintVDA = bVDA;
	mbPrintWM = bWM;
	mbPrintWX = bWX;
	mbPrintE = bE;
	mbPrintRWB = bRWB;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestW65C816Context::SetOperationPrint(bool bAddressOperation, bool bDataOperation, bool bDescription)
{
	mbAddressOperation = bAddressOperation;
	mbDataOperation = bDataOperation;
	mbDescription = bDescription;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestW65C816Context::DebugSpew(void)
{
	mbDebugSpew = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8* CTestW65C816Context::GetMemory(void)
{
	return mauiMemory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CTestW65C816Context::GetByte(uint32 uiAddress)
{
	if (uiAddress < muiMemorySize)
	{
		return mauiMemory[uiAddress];
	}
	else
	{
		return muiFill;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CTestW65C816Context::GetShort(uint32 uiAddress)
{
	if (uiAddress < muiMemorySize)
	{
		return *((uint16*)&mauiMemory[uiAddress]);
	}
	else
	{
		return muiFill;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestW65C816Context::SetByte(uint32 uiAddress, uint8 uiValue)
{
	if (uiAddress < muiMemorySize)
	{
		mauiMemory[uiAddress] = uiValue;
	}
	muiLastAddress = uiAddress;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestW65C816Context::SetByte(uint8 uiValue)
{
	SetByte(muiLastAddress + 1, uiValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestW65C816Context::SetShort(uint32 uiAddress, uint16 uiValue)
{
	if (uiAddress < muiMemorySize)
	{
		*((uint16*)&mauiMemory[uiAddress]) = uiValue;
	}
	muiLastAddress = uiAddress + 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestW65C816Context::SetShort(uint16 uiValue)
{
	SetShort(muiLastAddress + 1, uiValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestW65C816Context::SetOpcd(uint32 uiAddress, uint8 uiOpcode)
{
	muiInstructions++;
	SetByte(uiAddress, uiOpcode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestW65C816Context::SetOpcd(uint8 uiOpcode)
{
	muiInstructions++;
	SetByte(uiOpcode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CTestW65C816Context::GetSequence(void)
{
	return &mszSequence;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CTestW65C816Context::SequenceText(void)
{
	return mszSequence.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTestW65C816Context::GetStartOfPrevPrevLine(void)
{
	return muiStartOfPrevPrevLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestW65C816Context::SetStartOfPreviousLine(size uiStartOfPreviousLine)
{
	muiStartOfPrevPrevLine = muiStartOfPreviousLine;
	muiStartOfPreviousLine = uiStartOfPreviousLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestW65C816Context::StripToInstruction(char* szInstruction)
{
	size	uiIndex;

	uiIndex = mszSequence.Find(szInstruction);
	uiIndex = mszSequence.FindFromEnd(uiIndex, "OPC");
	mszSequence.RemoveFromStart(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void MergeEndCycleString(CTestW65C816Context* pcContext, bool bFetchOpcode, size uiPreviousStart, CChars* psz)
{
	size	uiPos1;
	size	uiPos2;

	if (pcContext->mbPrintMergedCycles && bFetchOpcode)
	{
		uiPos1 = psz->Find(pcContext->GetStartOfPrevPrevLine(), ':');
		uiPos2 = psz->Find(uiPreviousStart, ':');
		if (uiPos1 != uiPos2)
		{
			psz->Remove(uiPos1, uiPos2);
		}
		else
		{
			psz->Clear();
		}
		uiPreviousStart = psz->Length();
		pcContext->SetStartOfPreviousLine(uiPreviousStart);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CTestW65C816Context::Run(CW65C816Meta* pcMPU)
{
	uint64		i;
	bool		bRunning;
	size		uiEnd;
	size		uiLength;

	mpcMPU = pcMPU;
	for (i = 0;; i++)
	{
		bRunning = pcMPU->TickInstruction();
		if ((muiMaxInstructions != 0) && (i >= muiMaxInstructions))
		{
			break;
		}
		if (!bRunning)
		{
			if (mbEnablePrint)
			{
				if (mbPrintMergedCycles && mbPrintStop)
				{
					uiLength = mszSequence.Length();
					uiEnd = mszSequence.FindFromEnd(uiLength - 2, '\n');
					mszSequence.Remove(uiEnd, uiLength);
					mszSequence.AppendNewLine();
				}
			}
			break;
		}
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C81ContextDebugSpew(CW65C816Meta* pcMPU, CTestW65C816Context* pcContext)
{
	CChars					szTemp;

	if (pcContext->mbDebugSpew)
	{
		szTemp.Init();
		pcMPU->Print(&szTemp,
			pcContext->mbPrintMnemonic,
			pcContext->mbPrintCycle,
			pcContext->mbPrintOperation,
			pcContext->mbPrintA,
			pcContext->mbPrintX,
			pcContext->mbPrintY,
			pcContext->mbPrintPC,
			pcContext->mbPrintS,
			pcContext->mbPrintDP,
			pcContext->mbPrintDB,
			pcContext->mbPrintP,
			pcContext->mbPrintAddressBus,
			pcContext->mbPrintDataBus,
			pcContext->mbPrintBank,
			pcContext->mbPrintVPB,
			pcContext->mbPrintRDY,
			pcContext->mbPrintMLB,
			pcContext->mbPrintVPA,
			pcContext->mbPrintVDA,
			pcContext->mbPrintWM,
			pcContext->mbPrintWX,
			pcContext->mbPrintE,
			pcContext->mbPrintRWB,
			pcContext->mbAddressOperation,
			pcContext->mbDataOperation,
			pcContext->mbDescription);
		szTemp.AppendNewLine();
		szTemp.DumpKill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C81ContextPrint(CW65C816Meta* pcMPU, CTestW65C816Context* pcContext)
{
	bool					bReset;
	bool					bStop;
	bool					bFetchOpcode;
	CChars*					psz;
	size					uiPreviousStart;

	if (pcContext->mbEnablePrint)
	{
		bFetchOpcode = pcMPU->IsFetchOpcodeCycle();
		bReset = pcMPU->IsResetInstruction();
		bStop = pcMPU->IsStopInstruction();

		if ((bReset && pcContext->mbPrintReset) ||
			(bStop && pcContext->mbPrintStop) ||
			(!bReset && !bStop))
		{
			psz = pcContext->GetSequence();
			uiPreviousStart = psz->Length();
			if (pcContext->mbPrintMergedCycles && bFetchOpcode)
			{
				pcContext->SetStartOfPreviousLine(uiPreviousStart);
			}

			pcMPU->Print(pcContext->GetSequence(),
				pcContext->mbPrintMnemonic,
				pcContext->mbPrintCycle,
				pcContext->mbPrintOperation,
				pcContext->mbPrintA,
				pcContext->mbPrintX,
				pcContext->mbPrintY,
				pcContext->mbPrintPC,
				pcContext->mbPrintS,
				pcContext->mbPrintDP,
				pcContext->mbPrintDB,
				pcContext->mbPrintP,
				pcContext->mbPrintAddressBus,
				pcContext->mbPrintDataBus,
				pcContext->mbPrintBank,
				pcContext->mbPrintVPB,
				pcContext->mbPrintRDY,
				pcContext->mbPrintMLB,
				pcContext->mbPrintVPA,
				pcContext->mbPrintVDA,
				pcContext->mbPrintWM,
				pcContext->mbPrintWX,
				pcContext->mbPrintE,
				pcContext->mbPrintRWB,
				pcContext->mbAddressOperation,
				pcContext->mbDataOperation,
				pcContext->mbDescription);
			psz->AppendNewLine();

			MergeEndCycleString(pcContext, bFetchOpcode, uiPreviousStart, psz);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C81ContextTickLow(CW65C816Meta* pcMPU, void* pvContext)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C81ContextTickHigh(CW65C816Meta* pcMPU, void* pvContext)
{
	uint32					uiAddress;
	bool					bRead;
	bool					bWrite;
	bool					bValidAddress;
	uint8					uiData;
	CTestW65C816Context*	pcContext;

	pcContext = (CTestW65C816Context*)pvContext;

	uiAddress = pcMPU->GetAddress()->Get();
	uiAddress |= pcMPU->GetBank() << 16;

	bWrite = pcMPU->GetRWB()->IsLow();
	bRead = pcMPU->GetRWB()->IsHigh();

	bValidAddress = pcMPU->GetVDA()->IsHigh() || pcMPU->GetVPA()->IsHigh();

	if (bValidAddress)
	{
		if (bRead)
		{
			uiData = pcContext->GetByte(uiAddress);
			pcMPU->GetData()->Set(uiData);
		}
		if (bWrite)
		{
			uiData = pcMPU->GetData()->Get();
			pcContext->SetByte(uiAddress, uiData);
		}
	}

	TestW65C81ContextDebugSpew(pcMPU, pcContext);
	TestW65C81ContextPrint(pcMPU, pcContext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816RunInContext(CTestW65C816Context* pcContext, char* szExpected)
{
	CW65C816Meta	cMPU;
	uint64			uiInstructions;

	CW65C816InstructionFactory::GetInstance()->Init();

	pcContext->SetPrint(true, true, true, false, false, true, true, true, true, false, false, false, true, true);
	if (pcContext->muiMaxInstructions == 0)
	{
		pcContext->muiMaxInstructions = 10000;
	}

	cMPU.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, pcContext);

	uiInstructions = pcContext->Run(&cMPU);

	cMPU.Kill();

	AssertLong(pcContext->muiInstructions, uiInstructions);

	if (szExpected)
	{
		AssertString(szExpected, pcContext->SequenceText());
	}

	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816InContext(CTestW65C816Context* pcContext, char* szExpected)
{
	TestW65C816RunInContext(pcContext, szExpected);

	pcContext->Kill();
}


