#include "MicroprocessorLib/Pull.h"
#include "MicroprocessorLib/LongTime.h"
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
// Runs the board on for iNanoseconds.
//
//////////////////////////////////////////////////////////////////////////
void TestPullRunFor(CTestBoard* pcBoard, int64 iNanoseconds)
{
	pcBoard->mpcTimeline->RunToTime(NanosecondsToTime(iNanoseconds));
}


//////////////////////////////////////////////////////////////////////////
//
// An output driving a pulled-up line low wins; let go, the line curves
// back to the pull (10k into the trace's 3pF, 30ns) and its reader is
// told; driven low again it slews from the pulled level, so the reader
// sees the fall.  Driven high from the pulled 3.3V it falls to its 3.1V,
// never below VIH: no transition.
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
	TestPullRunFor(&cBoard, 200);
	AssertFloat(3.3f, TestPullVoltage(&cBoard, pcLine), 3);
	AssertInt(TV_Low, cBoard.Value(pcOut));

	pcOutput->Write(cBoard.mpcTimeline, false);
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcLine));
	AssertInt(TV_High, cBoard.Value(pcOut));

	pcOutput->WriteImpedance(cBoard.mpcTimeline);
	cBoard.Run();
	TestPullRunFor(&cBoard, 200);
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
	TestPullRunFor(&cBoard, 100);
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
// A line pulled up to a rail that ramps 0.1V every 100ns: the line
// follows each step along a 30ns curve (10k into 3pF), so it only reaches
// 2.0V, its reader's VIH, as it curves to the next step, 2.1V, a few ns
// after 2.1us, and the reader sees it go high then.
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
	AssertTrue(TestPullVoltage(&cBoard, pcLine) < 2.0f);
	AssertInt(TV_High, cBoard.Value(pcOut));
	cBoard.mpcTimeline->RunToTime(NanosecondsToTime((int64)130));
	AssertTrue(TestPullVoltage(&cBoard, pcLine) > 2.0f);
	AssertInt(TV_Low, cBoard.Value(pcOut));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 10k pull-up on a line of 10pF, driven low (0.2V) and let go: it
// curves to 3.3V with a time constant of 100ns, at 3.3 - 3.1 / e =
// 2.160V one time constant on, and its reader sees it pass VIH, 2.0V, at
// 100 x ln(3.1 / 1.3) = 86.9ns.  Written low again 50ns into the curve,
// at 3.3 - 3.1 x e^-0.5 = 1.420V, the driver slews down from where the
// curve is after its 1.45ns hold, 3.3 - 3.1 x e^-0.5145 = 1.447V.
//
//////////////////////////////////////////////////////////////////////////
void TestPullRelease(void)
{
	CTestBoard		cBoard;
	CTrace*			pcLine;
	CTrace*			pcOut;
	CConstant*		pcDriver;
	CLogicPort*		pcOutput;
	CPortOutput*	pcCurve;
	int64			iRelease;
	int64			iCrossing;

	cBoard.Init();
	pcDriver = cBoard.Input("A", 0, 1, &pcLine);
	pcOutput = pcDriver->GetConstantPins()->GetOutput(0);
	pcLine->SetCapacitance(10e-12f);
	TestPullAdd(&cBoard, cBoard.mpcVCC, pcLine, 10000.0f);
	pcOut = cBoard.Not(pcLine);
	cBoard.Start();
	cBoard.Run();
	AssertFloat(0.2f, TestPullVoltage(&cBoard, pcLine), 3);

	pcOutput->WriteImpedance(cBoard.mpcTimeline);
	TestPullRunFor(&cBoard, 1);
	pcCurve = pcLine->GetWeakCurve();
	AssertTrue(pcCurve->IsCurve());
	iRelease = pcCurve->GetTime();
	AssertFloat(0.2f, pcCurve->GetStartVoltage(), 3);
	AssertFloat(3.3f, pcCurve->GetEndVoltage(), 3);
	AssertTrue((pcCurve->GetTimeConstant() >= NanosecondsToTime((int64)100) - 1) && (pcCurve->GetTimeConstant() <= NanosecondsToTime((int64)100) + 1));

	iCrossing = iRelease + NanosecondsToTime(86.9);
	cBoard.mpcTimeline->RunToTime(iCrossing - NanosecondsToTime(0.5) - cBoard.mpcTimeline->GetTime());
	AssertFalse(cBoard.Value(pcLine) == TV_High);
	AssertInt(TV_High, cBoard.Value(pcOut));
	cBoard.mpcTimeline->RunToTime(NanosecondsToTime((int64)1));
	AssertInt(TV_High, cBoard.Value(pcLine));
	cBoard.mpcTimeline->RunToTime(iRelease + NanosecondsToTime((int64)100) - cBoard.mpcTimeline->GetTime());
	AssertFloat(2.160f, TestPullVoltage(&cBoard, pcLine), 3);
	AssertInt(TV_Low, cBoard.Value(pcOut));
	cBoard.Kill();

	cBoard.Init();
	pcDriver = cBoard.Input("A", 0, 1, &pcLine);
	pcOutput = pcDriver->GetConstantPins()->GetOutput(0);
	pcLine->SetCapacitance(10e-12f);
	TestPullAdd(&cBoard, cBoard.mpcVCC, pcLine, 10000.0f);
	cBoard.Start();
	cBoard.Run();
	pcOutput->WriteImpedance(cBoard.mpcTimeline);
	TestPullRunFor(&cBoard, 1);
	iRelease = pcLine->GetWeakCurve()->GetTime();
	cBoard.mpcTimeline->RunToTime(iRelease + NanosecondsToTime((int64)50) - cBoard.mpcTimeline->GetTime());
	AssertFloat(1.420f, TestPullVoltage(&cBoard, pcLine), 3);
	pcOutput->Write(cBoard.mpcTimeline, false);
	TestPullRunFor(&cBoard, 2);
	AssertTrue(pcOutput->GetOutput()->IsSlew());
	AssertFloat(1.447f, pcOutput->GetOutput()->GetStartVoltage(), 3);
	AssertFalse(pcLine->GetWeakCurve()->IsSlew());
	cBoard.Run();
	AssertFloat(0.2f, TestPullVoltage(&cBoard, pcLine), 3);

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
	TestPullRelease();

	TestStatistics();
}
