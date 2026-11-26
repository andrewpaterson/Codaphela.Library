#include "MicroprocessorLib/Pull.h"
#include "TestCircuitBoard.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// A pull of fResistance from pcSupply to pcSignal.
//
//////////////////////////////////////////////////////////////////////////
CPull* TestPullAdd(CTestBoard* pcBoard, CTrace* pcSupply, CTrace* pcSignal, float32 fResistance)
{
	CPull*	pcPull;

	pcPull = &OMalloc<CPull>(pcBoard->mpcTop, "R1", fResistance);
	pcPull->GetSupplyPort()->Connect(pcSupply);
	pcPull->GetSignalPort()->Connect(pcSignal);
	return pcPull;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 TestPullVoltage(CTestBoard* pcBoard, CTrace* pcTrace)
{
	return pcTrace->GetVoltage(pcBoard->mpcTimeline->GetTime());
}


//////////////////////////////////////////////////////////////////////////
//
// A line nothing drives is held at its pull's supply, and a chip reading
// it sees that as it starts; two pulls divide.
//
//////////////////////////////////////////////////////////////////////////
void TestPullHolds(void)
{
	CTestBoard	cBoard;
	CTrace*		pcLine;
	CTrace*		pcOut;
	CTrace*		pcDivided;
	CPull*		pcPull;

	cBoard.Init();
	pcLine = cBoard.NewTrace();
	pcPull = TestPullAdd(&cBoard, cBoard.mpcVCC, pcLine, 10000.0f);
	AssertString(PULL_PIN_SIGNAL, pcPull->GetSignalPort()->GetName());
	AssertFloat(10000.0f, pcPull->GetSignalPort()->GetResistance(), 1);
	pcOut = cBoard.Not(pcLine);
	pcDivided = cBoard.NewTrace();
	TestPullAdd(&cBoard, cBoard.mpcVCC, pcDivided, 10000.0f);
	TestPullAdd(&cBoard, cBoard.mpcGND, pcDivided, 10000.0f);
	cBoard.Start();
	cBoard.Run();
	AssertFloat(3.3f, TestPullVoltage(&cBoard, pcLine), 3);
	AssertFalse(pcLine->HasStrongDriver(cBoard.mpcTimeline->GetTime()));
	AssertInt(TV_Low, cBoard.Value(pcOut));
	AssertFloat(1.65f, TestPullVoltage(&cBoard, pcDivided), 3);

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An output driving a pulled-up line low wins; let go, the line goes back
// to the pull and its reader is told; driven low again it slews from the
// pulled level, so the reader sees the fall.  Driven high from the pulled
// 3.3V it falls to its 3.1V, never below VIH: no transition.
//
//////////////////////////////////////////////////////////////////////////
void TestPullStrongWins(void)
{
	CTestBoard		cBoard;
	CTrace*			pcLine;
	CTrace*			pcOut;
	CConstant*		pcDriver;
	CLogicPort*		pcOutput;

	cBoard.Init();
	pcDriver = cBoard.Input("A", 0, 1, &pcLine);
	pcOutput = pcDriver->GetConstantPins()->GetOutput(0);
	TestPullAdd(&cBoard, cBoard.mpcVCC, pcLine, 10000.0f);
	pcOut = cBoard.Not(pcLine);
	cBoard.Start();
	cBoard.Run();
	AssertTrue(TestPullVoltage(&cBoard, pcLine) < 0.3f);
	AssertTrue(pcLine->HasStrongDriver(cBoard.mpcTimeline->GetTime()));
	AssertInt(TV_High, cBoard.Value(pcOut));

	pcOutput->WriteImpedance(cBoard.mpcTimeline);
	cBoard.Run();
	AssertFloat(3.3f, TestPullVoltage(&cBoard, pcLine), 3);
	AssertInt(TV_Low, cBoard.Value(pcOut));

	pcOutput->Write(cBoard.mpcTimeline, false);
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcLine));
	AssertInt(TV_High, cBoard.Value(pcOut));

	pcOutput->WriteImpedance(cBoard.mpcTimeline);
	cBoard.Run();
	pcOutput->Write(cBoard.mpcTimeline, true);
	cBoard.Run();
	AssertFloat(3.1f, TestPullVoltage(&cBoard, pcLine), 2);
	AssertInt(TV_Low, cBoard.Value(pcOut));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An open drain output with a pull-up: let go, it pulls its line to its
// VCC, weakly, so another output driving the line low wins.
//
//////////////////////////////////////////////////////////////////////////
void TestPullOpenDrain(void)
{
	CTestBoard		cBoard;
	CTrace*			pcLine;
	CTrace*			pcOther;
	CConstant*		pcDriver;
	CConstant*		pcSecond;
	CLogicPort*		pcOutput;
	float32			fResistance;

	cBoard.Init();
	pcDriver = cBoard.Input("RST", 0, 1, &pcLine);
	pcOutput = pcDriver->GetConstantPins()->GetOutput(0);
	pcOutput->SetPullUp(5500.0f);
	AssertFloat(5500.0f, pcOutput->GetPullUp(), 1);
	pcSecond = cBoard.Input("Button", 1, 1, &pcOther);
	cBoard.Start();
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcLine));

	pcOutput->WriteImpedance(cBoard.mpcTimeline);
	cBoard.Run();
	AssertFloat(3.3f, pcOutput->GetDrive(cBoard.mpcTimeline->GetTime(), &fResistance), 3);
	AssertFloat(5500.0f, fResistance, 1);
	AssertFloat(3.3f, TestPullVoltage(&cBoard, pcLine), 3);

	pcSecond->GetConstantPins()->GetOutput(0)->Disconnect(cBoard.mpcTimeline);
	AssertTrue(pcSecond->GetConstantPins()->GetOutput(0)->ConnectLive(pcLine, cBoard.mpcTimeline));
	cBoard.Write(pcSecond, false);
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcLine));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A line pulled up to a rail that ramps: its reader, powered from VCC,
// sees it go high as the rail passes VIH, 2.0V.
//
//////////////////////////////////////////////////////////////////////////
void TestPullRamp(void)
{
	CTestBoard		cBoard;
	CTrace*			pcRail;
	CTrace*			pcLine;
	CTrace*			pcOut;
	CPowerSource*	pcRamp;
	int64			iStep;

	cBoard.Init();
	pcRail = cBoard.NewTrace();
	pcRamp = &OMalloc<CPowerSource>(cBoard.mpcTop, "Ramp", 3.3f);
	pcRamp->SetRiseTime(NanosecondsToTime((int64)3300));
	pcRamp->GetPowerOutPort()->Connect(pcRail);
	pcLine = cBoard.NewTrace();
	TestPullAdd(&cBoard, pcRail, pcLine, 10000.0f);
	pcOut = cBoard.Not(pcLine);
	iStep = NanosecondsToTime((int64)100);
	cBoard.Start();

	cBoard.mpcTimeline->RunToTime(19 * iStep + NanosecondsToTime((int64)50));
	AssertInt(TV_High, cBoard.Value(pcOut));
	cBoard.mpcTimeline->RunToTime(iStep);
	AssertFloat(2.0f, TestPullVoltage(&cBoard, pcLine), 3);
	AssertInt(TV_Low, cBoard.Value(pcOut));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPull(void)
{
	BeginTests();

	TestPullHolds();
	TestPullStrongWins();
	TestPullOpenDrain();
	TestPullRamp();

	TestStatistics();
}
