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
// The trace's voltage now.
//
//////////////////////////////////////////////////////////////////////////
float32 TestPowerVoltage(CTestBoard* pcBoard, CTrace* pcTrace)
{
	return pcTrace->GetVoltage(pcBoard->mpcTimeline->GetTime());
}


//////////////////////////////////////////////////////////////////////////
//
// VCC rises 0.1V every 100ns.  The constant and the inverter come alive
// at LVC's lowest VCC, 1.8V, the inverter's output high at 1.8V's VOH,
// and the output rises with the rail to 3.3V's.
//
//////////////////////////////////////////////////////////////////////////
void TestPowerRamp(void)
{
	CTestBoard		cBoard;
	CTrace*			pcOut;
	int64			iStep;
	CPowerOutPort*	pcPort;

	cBoard.Init();
	pcPort = cBoard.mpcPower->GetPowerOutPort();
	cBoard.mpcPower->SetRiseTime(NanosecondsToTime((int64)3300));
	iStep = NanosecondsToTime((int64)100);
	AssertLong(iStep, pcPort->CalculateStepTime());
	pcOut = cBoard.Not(cBoard.Input("A", false));
	cBoard.Start();
	AssertFloat(0.0f, TestPowerVoltage(&cBoard, cBoard.mpcVCC), 3);
	AssertTrue(pcPort->IsOn());

	cBoard.mpcTimeline->RunToTime(17 * iStep + NanosecondsToTime((int64)50));
	AssertFloat(1.7f, TestPowerVoltage(&cBoard, cBoard.mpcVCC), 3);
	AssertInt(TV_Undriven, cBoard.Value(pcOut));

	cBoard.mpcTimeline->RunToTime(iStep);
	AssertFloat(1.8f, TestPowerVoltage(&cBoard, cBoard.mpcVCC), 3);
	AssertFloat(1.6f, TestPowerVoltage(&cBoard, pcOut), 2);

	cBoard.mpcTimeline->RunToTime(15 * iStep);
	AssertFloat(3.3f, TestPowerVoltage(&cBoard, cBoard.mpcVCC), 3);
	AssertFloat(3.1f, TestPowerVoltage(&cBoard, pcOut), 2);
	AssertInt(TV_High, cBoard.Value(pcOut));

	cBoard.Run();
	AssertFloat(3.3f, TestPowerVoltage(&cBoard, cBoard.mpcVCC), 3);
	AssertFloat(3.1f, TestPowerVoltage(&cBoard, pcOut), 2);

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Switched off, VCC falls 0.1V every 100ns and the inverter stops
// driving once it is below 1.8V.  Switched on again it rises from where
// it is.
//
//////////////////////////////////////////////////////////////////////////
void TestPowerSwitchedOff(void)
{
	CTestBoard	cBoard;
	CTrace*		pcOut;
	int64		iStep;

	cBoard.Init();
	cBoard.mpcPower->SetRiseTime(NanosecondsToTime((int64)3300));
	iStep = NanosecondsToTime((int64)100);
	pcOut = cBoard.Not(cBoard.Input("A", false));
	cBoard.Start();
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcOut));

	cBoard.mpcPower->Switch(cBoard.mpcTimeline, false);
	AssertFalse(cBoard.mpcPower->IsOn());
	cBoard.mpcTimeline->RunToTime(15 * iStep + NanosecondsToTime((int64)50));
	AssertFloat(1.8f, TestPowerVoltage(&cBoard, cBoard.mpcVCC), 3);
	AssertFloat(1.6f, TestPowerVoltage(&cBoard, pcOut), 2);
	cBoard.mpcTimeline->RunToTime(iStep);
	AssertFloat(1.7f, TestPowerVoltage(&cBoard, cBoard.mpcVCC), 3);
	AssertInt(TV_Undriven, cBoard.Value(pcOut));

	cBoard.mpcPower->Switch(cBoard.mpcTimeline, true);
	cBoard.mpcTimeline->RunToTime(iStep + NanosecondsToTime((int64)50));
	AssertFloat(1.8f, TestPowerVoltage(&cBoard, cBoard.mpcVCC), 3);
	AssertFloat(1.6f, TestPowerVoltage(&cBoard, pcOut), 2);

	cBoard.Run();
	AssertFloat(3.3f, TestPowerVoltage(&cBoard, cBoard.mpcVCC), 3);
	AssertInt(TV_High, cBoard.Value(pcOut));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A buffer powered from VCC with its input tied to a second rail that
// ramps: high once the rail reaches VIH, 2.0V, and low once it falls to
// VIL, 0.8V; between them the input is unsettled and changes nothing.
//
//////////////////////////////////////////////////////////////////////////
void TestPowerTiedInput(void)
{
	CTestBoard		cBoard;
	CTrace*			pcRail;
	CTrace*			pcOut;
	CPowerSource*	pcRamp;
	int64			iStep;
	int64			iSettle;

	cBoard.Init();
	pcRail = cBoard.NewTrace();
	pcRamp = &OMalloc<CPowerSource>(cBoard.mpcTop, "Ramp", 3.3f);
	pcRamp->SetRiseTime(NanosecondsToTime((int64)3300));
	pcRamp->GetPowerOutPort()->Connect(pcRail);
	iStep = NanosecondsToTime((int64)100);
	iSettle = NanosecondsToTime((int64)50);
	pcOut = cBoard.Buffer(pcRail, false);
	cBoard.Start();

	cBoard.mpcTimeline->RunToTime(19 * iStep + iSettle);
	AssertInt(TV_Undriven, cBoard.Value(pcOut));
	cBoard.mpcTimeline->RunToTime(iStep);
	AssertFloat(2.0f, TestPowerVoltage(&cBoard, pcRail), 3);
	AssertInt(TV_High, cBoard.Value(pcOut));
	cBoard.Run();

	pcRamp->Switch(cBoard.mpcTimeline, false);
	cBoard.mpcTimeline->RunToTime(24 * iStep + iSettle);
	AssertFloat(0.9f, TestPowerVoltage(&cBoard, pcRail), 3);
	AssertInt(TV_High, cBoard.Value(pcOut));
	cBoard.mpcTimeline->RunToTime(iStep);
	AssertInt(TV_Low, cBoard.Value(pcOut));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 3.3V regulator with a 1.1V dropout fed by a rail rising 0.1V every
// 100ns to 5V: on, at once, once its feed reaches 4.4V, and off again as
// the feed falls below it.
//
//////////////////////////////////////////////////////////////////////////
void TestPowerRegulator(void)
{
	CTestBoard		cBoard;
	CTrace*			pcFeed;
	CTrace*			pcRail;
	CPowerSource*	pcSupply;
	CPowerSource*	pcRegulator;
	int64			iStep;

	cBoard.Init();
	pcFeed = cBoard.NewTrace();
	pcRail = cBoard.NewTrace();
	pcRegulator = &OMalloc<CPowerSource>(cBoard.mpcTop, "Regulator", 3.3f);
	pcRegulator->SetFeed(1.1f);
	pcRegulator->GetFeedPort()->Connect(pcFeed);
	pcRegulator->GetPowerOutPort()->Connect(pcRail);
	pcSupply = &OMalloc<CPowerSource>(cBoard.mpcTop, "Supply", 5.0f);
	pcSupply->SetRiseTime(NanosecondsToTime((int64)5000));
	pcSupply->GetPowerOutPort()->Connect(pcFeed);
	iStep = NanosecondsToTime((int64)100);
	AssertString(POWER_PIN_FEED, pcRegulator->GetFeedPort()->GetName());
	AssertFloat(1.1f, pcRegulator->GetDropout(), 3);
	cBoard.Start();
	AssertFloat(0.0f, TestPowerVoltage(&cBoard, pcRail), 3);

	cBoard.mpcTimeline->RunToTime(43 * iStep + 2);
	AssertFloat(4.3f, TestPowerVoltage(&cBoard, pcFeed), 3);
	AssertFalse(pcRegulator->IsOn());
	AssertFloat(0.0f, TestPowerVoltage(&cBoard, pcRail), 3);
	cBoard.mpcTimeline->RunToTime(2 * iStep);
	AssertTrue(pcRegulator->IsOn());
	AssertFloat(3.3f, TestPowerVoltage(&cBoard, pcRail), 3);
	cBoard.Run();
	AssertFloat(5.0f, TestPowerVoltage(&cBoard, pcFeed), 3);

	pcSupply->Switch(cBoard.mpcTimeline, false);
	cBoard.mpcTimeline->RunToTime(5 * iStep + 2);
	AssertFloat(4.5f, TestPowerVoltage(&cBoard, pcFeed), 3);
	AssertTrue(pcRegulator->IsOn());
	cBoard.mpcTimeline->RunToTime(2 * iStep);
	AssertFalse(pcRegulator->IsOn());
	AssertFloat(0.0f, TestPowerVoltage(&cBoard, pcRail), 3);

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
	TestPowerRamp();
	TestPowerSwitchedOff();
	TestPowerTiedInput();
	TestPowerRegulator();

	TestStatistics();
}
