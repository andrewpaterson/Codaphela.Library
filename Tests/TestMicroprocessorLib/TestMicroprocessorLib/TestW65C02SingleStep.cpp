#include <stdio.h>
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "MicroprocessorLib/W65C02.h"
#include "MicroprocessorLib/W65C02InstructionFactory.h"
#include "MicroprocessorLib/W65C02Meta.h"
#include "TestLib/Assert.h"
#include "TestSingleStep.h"


//////////////////////////////////////////////////////////////////////////
//
// The W65C02S tests from https://github.com/SingleStepTests/65x02 wdc65c02 (MIT
// licence).  Input/SingleStep65C02 holds the first tests of each opcode file (xx.bin)
// converted by ConvertSingleStep65C02Tests.py.  The tests give each cycle's address,
// value and read or write.  WAI (CB) and STP (DB) have no tests.
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C02SingleStepTickHigh(CW65C02Meta* pcMPU, void* pvContext)
{
	//Every W65C02S cycle reads or writes memory.  RWB is not driven before the first cycle.
	uint8	uiSignals;
	bool	bDriven;

	uiSignals = pcMPU->GetRWB()->IsHigh() ? SINGLE_STEP_READ : 0;
	bDriven = pcMPU->GetRWB()->IsHigh() || pcMPU->GetRWB()->IsLow();
	TestSingleStepRecordCycle(pcMPU, (CTestSingleStepContext*)pvContext, uiSignals, bDriven);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C02SingleStepTickLow(CW65C02Meta* pcMPU, void* pvContext)
{
}


class CSingleStepW65C02 : public CSingleStepProcessor
{
protected:
	CW65C02Meta		mcMPU;

public:
	void		Init(CTestSingleStepContext* pcContext) override	{ mcMPU.Init(TestW65C02SingleStepTickHigh, TestW65C02SingleStepTickLow, pcContext); }
	void		Kill(void) override									{ mcMPU.Kill(); }
	CW65CMeta*	GetMeta(void) override								{ return &mcMPU; }
	CW65CCore*	GetCPU(void) override								{ return mcMPU.GetMPU(); }
	void		SetState(SSingleStepState* psState) override		{ }
	uint8		GetSignalMask(void) override						{ return SINGLE_STEP_READ; }
	uint8		GetStatusMask(void) override						{ return 0xEF; }  //Bit 4 (B) only exists on the stack.
	bool		IsWaitOrStop(uint8 uiOpcode) override				{ return (uiOpcode == W65C02_WAI_Implied) || (uiOpcode == W65C02_STP_Implied); }
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C02SingleStep(void)
{
	int					iOpcode;
	CSingleStepW65C02	cProcessor;
	char				szName[8];

	TypeConverterInit();
	NumberInit();
	BeginTests();

	CW65C02InstructionFactory::GetInstance()->Init();

	for (iOpcode = 0; iOpcode < 256; iOpcode++)
	{
		if ((iOpcode != W65C02_WAI_Implied) && (iOpcode != W65C02_STP_Implied))
		{
			snprintf(szName, sizeof(szName), "%02x", iOpcode);
			TestSingleStepFile(&cProcessor, "SingleStep65C02", szName, (uint8)iOpcode);
		}
	}

	CW65C02InstructionFactory::GetInstance()->Kill();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

