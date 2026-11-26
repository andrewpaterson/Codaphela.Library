#include "MicroprocessorLib/PowerOutPort.h"
#include "TestCircuitBoard.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// An inverter of a low input, its GND wired but not its VCC, with its
// output on a new trace.
//
//////////////////////////////////////////////////////////////////////////
CInverter* TestPowerUnpoweredInverter(CTestBoard* pcBoard, CTrace** ppcOut)
{
	CInverter*	pcInverter;

	pcInverter = &OMalloc<CInverter>(pcBoard->mpcTop, "U1", OMalloc<CBufferPins>(1, pcBoard->mpcFamily));
	pcInverter->GetPins()->GetVoltageGround()->Connect(pcBoard->mpcGND);
	pcInverter->GetBufferPins()->GetInput(0)->Connect(pcBoard->Input("A", false));
	*ppcOut = pcBoard->NewTrace();
	pcInverter->GetBufferPins()->GetOutput(0)->Connect(*ppcOut);
	return pcInverter;
}


//////////////////////////////////////////////////////////////////////////
//
// A chip starts when its VCC is wired while the simulation runs, reading
// the input that is already low, and stops driving when it is cut.
//
//////////////////////////////////////////////////////////////////////////
void TestPowerWiredWhileRunning(void)
{
	CTestBoard		cBoard;
	CInverter*		pcInverter;
	CTrace*			pcOut;
	CPowerInPort*	pcVCC;

	cBoard.Init();
	pcInverter = TestPowerUnpoweredInverter(&cBoard, &pcOut);
	pcVCC = pcInverter->GetPins()->GetVoltageCommon();
	cBoard.Start();
	cBoard.Run();
	AssertFalse(pcInverter->IsPowered(cBoard.mpcTimeline->GetTime()));
	AssertInt(TV_Undriven, cBoard.Value(pcOut));

	AssertTrue(pcVCC->ConnectLive(cBoard.mpcVCC, cBoard.mpcTimeline));
	cBoard.Run();
	AssertTrue(pcInverter->IsPowered(cBoard.mpcTimeline->GetTime()));
	AssertInt(TV_High, cBoard.Value(pcOut));

	pcVCC->Disconnect(cBoard.mpcTimeline);
	cBoard.Run();
	AssertInt(TV_Undriven, cBoard.Value(pcOut));

	AssertTrue(pcVCC->ConnectLive(cBoard.mpcVCC, cBoard.mpcTimeline));
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcOut));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// LVC works from 1.8V: on a 1.5V supply the inverter has no power and
// drives nothing.
//
//////////////////////////////////////////////////////////////////////////
void TestPowerBelowWorkingVoltage(void)
{
	CTestBoard		cBoard;
	CInverter*		pcInverter;
	CTrace*			pcOut;
	CTrace*			pcLowRail;
	CPowerSource*	pcLowSupply;

	cBoard.Init();
	pcInverter = TestPowerUnpoweredInverter(&cBoard, &pcOut);
	pcLowRail = cBoard.NewTrace();
	pcLowSupply = &OMalloc<CPowerSource>(cBoard.mpcTop, "Low", 1.5f);
	pcLowSupply->GetPowerOutPort()->Connect(pcLowRail);
	pcInverter->GetPins()->GetVoltageCommon()->Connect(pcLowRail);
	cBoard.Start();
	cBoard.Run();
	AssertFalse(pcInverter->IsPowered(cBoard.mpcTimeline->GetTime()));
	AssertInt(TV_Undriven, cBoard.Value(pcOut));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPower(void)
{
	BeginTests();

	TestPowerWiredWhileRunning();
	TestPowerBelowWorkingVoltage();

	TestStatistics();
}
