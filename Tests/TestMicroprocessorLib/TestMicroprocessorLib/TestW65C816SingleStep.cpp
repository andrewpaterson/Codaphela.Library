#include <stdio.h>
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "MicroprocessorLib/W65C816.h"
#include "MicroprocessorLib/W65C816InstructionFactory.h"
#include "MicroprocessorLib/W65C816Meta.h"
#include "TestLib/Assert.h"
#include "TestSingleStep.h"


//////////////////////////////////////////////////////////////////////////
//
// The W65C816S tests from https://github.com/SingleStepTests/65816 (MIT licence).
// Input/SingleStep holds the first tests of each opcode file, one file per opcode and
// mode (xx.n.bin native, xx.e.bin emulation).
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816SingleStepTickHigh(CW65C816Meta* pcMPU, void* pvContext)
{
	uint8	uiSignals;
	bool	bEnabled;

	uiSignals = 0;
	uiSignals |= pcMPU->GetVDA()->IsHigh() ? SINGLE_STEP_VDA : 0;
	uiSignals |= pcMPU->GetVPA()->IsHigh() ? SINGLE_STEP_VPA : 0;
	uiSignals |= pcMPU->GetVPB()->IsLow() ? SINGLE_STEP_VPB : 0;
	uiSignals |= pcMPU->GetRWB()->IsHigh() ? SINGLE_STEP_READ : 0;
	uiSignals |= pcMPU->GetE()->IsHigh() ? SINGLE_STEP_E : 0;
	uiSignals |= pcMPU->GetMX()->IsHigh() ? SINGLE_STEP_M : 0;
	uiSignals |= pcMPU->GetX().IsHigh() ? SINGLE_STEP_X : 0;
	uiSignals |= pcMPU->GetMLB()->IsLow() ? SINGLE_STEP_MLB : 0;

	//As in the tests, memory only responds to VDA, VPA or VPB.
	bEnabled = (uiSignals & (SINGLE_STEP_VDA | SINGLE_STEP_VPA | SINGLE_STEP_VPB)) != 0;
	TestSingleStepRecordCycle(pcMPU, (CTestSingleStepContext*)pvContext, uiSignals, bEnabled);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816SingleStepTickLow(CW65C816Meta* pcMPU, void* pvContext)
{
}


class CSingleStepW65C816 : public CSingleStepProcessor
{
protected:
	CW65C816Meta	mcMPU;

public:
	void		Init(CTestSingleStepContext* pcContext) override	{ mcMPU.Init(TestW65C816SingleStepTickHigh, TestW65C816SingleStepTickLow, pcContext); }
	void		Kill(void) override									{ mcMPU.Kill(); }
	CW65CMeta*	GetMeta(void) override								{ return &mcMPU; }
	CW65CCore*	GetCPU(void) override								{ return mcMPU.GetMPU(); }
	uint8		GetSignalMask(void) override						{ return 0xFF; }
	uint8		GetStatusMask(void) override						{ return 0xFF; }
	bool		IsWaitOrStop(uint8 uiOpcode) override				{ return (uiOpcode == WAI_Implied) || (uiOpcode == STP_Implied); }

	void SetState(SSingleStepState* psState) override
	{
		mcMPU.GetMPU()->TestSetDirectPage(psState->uiD);
		mcMPU.GetMPU()->TestSetDataBank(psState->uiDBR);
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C816SingleStep(void)
{
	int					iOpcode;
	CSingleStepW65C816	cProcessor;
	char				szName[8];

	TypeConverterInit();
	NumberInit();
	BeginTests();

	CW65C816InstructionFactory::GetInstance()->Init();

	for (iOpcode = 0; iOpcode < 256; iOpcode++)
	{
		snprintf(szName, sizeof(szName), "%02x.n", iOpcode);
		TestSingleStepFile(&cProcessor, "SingleStep", szName, (uint8)iOpcode);
		snprintf(szName, sizeof(szName), "%02x.e", iOpcode);
		TestSingleStepFile(&cProcessor, "SingleStep", szName, (uint8)iOpcode);
	}

	CW65C816InstructionFactory::GetInstance()->Kill();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

