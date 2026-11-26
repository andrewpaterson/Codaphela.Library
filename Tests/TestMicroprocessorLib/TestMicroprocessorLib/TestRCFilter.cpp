#include <math.h>
#include "MicroprocessorLib/RCFilter.h"
#include "MicroprocessorLib/AnalogComparator.h"
#include "MicroprocessorLib/PowerSource.h"
#include "MicroprocessorLib/LongTime.h"
#include "TestCircuitBoard.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// An RC element from pcIn to a new trace, which it returns.
//
//////////////////////////////////////////////////////////////////////////
CTrace* TestRCFilterAdd(CTestBoard* pcBoard, CTrace* pcIn, int64 iRiseTimeConstant, int64 iFallTimeConstant, CRCFilter** ppcFilter)
{
	CRCFilter*	pcFilter;
	CTrace*		pcOut;

	pcFilter = &OMalloc<CRCFilter>(pcBoard->mpcTop, "RC", OMalloc<CRCFilterPins>(), iRiseTimeConstant, iFallTimeConstant);
	pcFilter->GetRCFilterPins()->GetIn()->Connect(pcIn);
	pcOut = pcBoard->NewTrace();
	pcFilter->GetRCFilterPins()->GetOut()->Connect(pcOut);
	if (ppcFilter != NULL)
	{
		*ppcFilter = pcFilter;
	}
	return pcOut;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the board on to iTime.
//
//////////////////////////////////////////////////////////////////////////
void TestRCFilterRunTo(CTestBoard* pcBoard, int64 iTime)
{
	pcBoard->mpcTimeline->RunToTime(iTime - pcBoard->mpcTimeline->GetTime());
}


//////////////////////////////////////////////////////////////////////////
//
// When the filter's output curve, started at iStart from fStart towards
// fEnd with iTimeConstant, reaches fVoltage.
//
//////////////////////////////////////////////////////////////////////////
int64 TestRCFilterCrossing(int64 iStart, float32 fStart, float32 fEnd, int64 iTimeConstant, float32 fVoltage)
{
	return iStart + (int64)(iTimeConstant * log((fStart - fEnd) / (fVoltage - fEnd)) + 0.5);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether iActual is within iTolerance of iExpected, the rounding of the
// figures worked out by hand.
//
//////////////////////////////////////////////////////////////////////////
bool TestRCFilterNear(int64 iExpected, int64 iActual, int64 iTolerance)
{
	return (iActual >= iExpected - iTolerance) && (iActual <= iExpected + iTolerance);
}


//////////////////////////////////////////////////////////////////////////
//
// A 10ns RC on an LVC output at 3.3V, written high: the input's slew
// (1.45ns hold, 2.9ns from 0.2V to 3.1V) is half way at 2.9ns, when the
// curve starts from 0.2V towards 3.1V.  It is at 3.1 - 2.9 / e = 2.033V
// one time constant on, and passes VIH, 2.0V, at 10 x ln(2.9 / 1.1) =
// 9.69ns, 12.6ns after the write, where the inverter reading it sees it
// go high.
//
//////////////////////////////////////////////////////////////////////////
void TestRCFilterRise(void)
{
	CTestBoard		cBoard;
	CTrace*			pcIn;
	CTrace*			pcOut;
	CTrace*			pcNot;
	CConstant*		pcDriver;
	CRCFilter*		pcFilter;
	CPortOutput*	pcCurve;
	int64			iTimeConstant;
	int64			iWrite;
	int64			iStart;
	int64			iCrossing;

	cBoard.Init();
	iTimeConstant = NanosecondsToTime((int64)10);
	pcDriver = cBoard.Input("A", 0, 1, &pcIn);
	pcOut = TestRCFilterAdd(&cBoard, pcIn, iTimeConstant, iTimeConstant, &pcFilter);
	pcNot = cBoard.Not(pcOut);
	cBoard.Start();
	cBoard.Run();
	AssertFloat(0.2f, pcOut->GetVoltage(cBoard.mpcTimeline->GetTime()), 3);
	AssertInt(TV_High, cBoard.Value(pcNot));
	AssertTrue(pcFilter->IsPowered(cBoard.mpcTimeline->GetTime()));

	iWrite = cBoard.mpcTimeline->GetTime();
	cBoard.Write(pcDriver, true);
	TestRCFilterRunTo(&cBoard, iWrite + NanosecondsToTime((int64)4));
	pcCurve = pcFilter->GetRCFilterPins()->GetOut()->GetOutput();
	AssertTrue(pcCurve->IsCurve());
	iStart = pcCurve->GetTime();
	AssertTrue(TestRCFilterNear(iWrite + NanosecondsToTime(2.9), iStart, 4));
	AssertFloat(0.2f, pcCurve->GetStartVoltage(), 3);
	AssertFloat(3.1f, pcCurve->GetEndVoltage(), 3);
	AssertLong(RC_CURVE_TIME_CONSTANTS * iTimeConstant, pcCurve->GetSlewTime());

	TestRCFilterRunTo(&cBoard, iStart + iTimeConstant);
	AssertFloat(2.033f, pcOut->GetVoltage(cBoard.mpcTimeline->GetTime()), 3);

	iCrossing = TestRCFilterCrossing(iStart, 0.2f, 3.1f, iTimeConstant, 2.0f);
	AssertTrue(TestRCFilterNear(iStart + NanosecondsToTime(9.69), iCrossing, 10));
	TestRCFilterRunTo(&cBoard, iCrossing - 2);
	AssertFalse(cBoard.Value(pcOut) == TV_High);
	TestRCFilterRunTo(&cBoard, iCrossing + 2);
	AssertInt(TV_High, cBoard.Value(pcOut));
	AssertInt(TV_High, cBoard.Value(pcNot));
	TestRCFilterRunTo(&cBoard, iCrossing + NanosecondsToTime((int64)6));
	AssertInt(TV_Low, cBoard.Value(pcNot));

	cBoard.Run();
	AssertFloat(3.1f, pcOut->GetVoltage(cBoard.mpcTimeline->GetTime()), 3);

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 2ns pulse into the 10ns RC: the curve gets to 0.2 + 2.9 x (1 -
// e^-0.2) = 0.73V and turns back, never reaching VIH, so the inverter
// reading it never changes, though a transition was pending for where
// the first curve would have crossed it.
//
//////////////////////////////////////////////////////////////////////////
void TestRCFilterPulse(void)
{
	CTestBoard		cBoard;
	CTrace*			pcIn;
	CTrace*			pcOut;
	CTrace*			pcNot;
	CConstant*		pcDriver;
	int64			iTimeConstant;
	int64			iWrite;
	int64			iTime;
	float32			fHighest;
	float32			fVoltage;

	cBoard.Init();
	iTimeConstant = NanosecondsToTime((int64)10);
	pcDriver = cBoard.Input("A", 0, 1, &pcIn);
	pcOut = TestRCFilterAdd(&cBoard, pcIn, iTimeConstant, iTimeConstant, NULL);
	pcNot = cBoard.Not(pcOut);
	cBoard.Start();
	cBoard.Run();

	iWrite = cBoard.mpcTimeline->GetTime();
	cBoard.Write(pcDriver, true);
	TestRCFilterRunTo(&cBoard, iWrite + NanosecondsToTime((int64)2));
	cBoard.Write(pcDriver, false);
	fHighest = 0.0f;
	for (iTime = iWrite; iTime < iWrite + NanosecondsToTime((int64)40); iTime += NanosecondsToTime(0.25))
	{
		TestRCFilterRunTo(&cBoard, iTime);
		fVoltage = pcOut->GetVoltage(iTime);
		if (fVoltage > fHighest)
		{
			fHighest = fVoltage;
		}
		AssertInt(TV_High, cBoard.Value(pcNot));
	}
	AssertTrue(fHighest > 0.6f);
	AssertTrue(fHighest < 0.8f);
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcOut));
	AssertInt(TV_High, cBoard.Value(pcNot));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A diode across the resistor: rising takes the 10ns time constant,
// falling 0.1ns, so the output is low within the input's own fall and
// still not high 5ns into a rise.
//
//////////////////////////////////////////////////////////////////////////
void TestRCFilterDiode(void)
{
	CTestBoard		cBoard;
	CTrace*			pcIn;
	CTrace*			pcOut;
	CConstant*		pcDriver;
	int64			iWrite;

	cBoard.Init();
	pcDriver = cBoard.Input("A", 1, 1, &pcIn);
	pcOut = TestRCFilterAdd(&cBoard, pcIn, NanosecondsToTime((int64)10), NanosecondsToTime(0.1), NULL);
	cBoard.Start();
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcOut));

	iWrite = cBoard.mpcTimeline->GetTime();
	cBoard.Write(pcDriver, false);
	TestRCFilterRunTo(&cBoard, iWrite + NanosecondsToTime((int64)4));
	AssertInt(TV_Low, cBoard.Value(pcOut));
	cBoard.Run();

	iWrite = cBoard.mpcTimeline->GetTime();
	cBoard.Write(pcDriver, true);
	TestRCFilterRunTo(&cBoard, iWrite + NanosecondsToTime((int64)5));
	AssertInt(TV_Low, cBoard.Value(pcOut));
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcOut));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Its input let go, the capacitor holds its voltage.
//
//////////////////////////////////////////////////////////////////////////
void TestRCFilterHolds(void)
{
	CTestBoard		cBoard;
	CTrace*			pcIn;
	CTrace*			pcOut;
	CConstant*		pcDriver;

	cBoard.Init();
	pcDriver = cBoard.Input("A", 1, 1, &pcIn);
	pcOut = TestRCFilterAdd(&cBoard, pcIn, NanosecondsToTime((int64)10), NanosecondsToTime((int64)10), NULL);
	cBoard.Start();
	cBoard.Run();
	AssertFloat(3.1f, pcOut->GetVoltage(cBoard.mpcTimeline->GetTime()), 3);

	pcDriver->GetConstantPins()->GetOutput(0)->WriteImpedance(cBoard.mpcTimeline);
	cBoard.Run();
	AssertFalse(IsVoltage(pcIn->GetVoltage(cBoard.mpcTimeline->GetTime())));
	AssertFloat(3.1f, pcOut->GetVoltage(cBoard.mpcTimeline->GetTime()), 3);

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The Clock Doubler's comparator: IN+ on a 1.6V supply, IN- an RC of
// 12ns (750 ohm into 16pF).  IN- low, OUT is high; written high, the
// curve passes 1.601V at 12 x ln(2.9 / 1.499) = 7.92ns, and OUT starts
// falling its 2.5ns hold after.
//
//////////////////////////////////////////////////////////////////////////
void TestRCFilterComparator(void)
{
	CTestBoard				cBoard;
	CTrace*					pcIn;
	CTrace*					pcMinus;
	CTrace*					pcPlus;
	CTrace*					pcOut;
	CConstant*				pcDriver;
	CRCFilter*				pcFilter;
	CAnalogComparator*		pcComparator;
	CPowerSource*			pcReference;
	CPortOutput*			pcCurve;
	CPortOutput*			pcOutput;
	int64					iTimeConstant;
	int64					iWrite;
	int64					iCrossing;

	cBoard.Init();
	iTimeConstant = NanosecondsToTime((int64)12);
	pcDriver = cBoard.Input("Fast CLK", 0, 1, &pcIn);
	pcMinus = TestRCFilterAdd(&cBoard, pcIn, iTimeConstant, iTimeConstant, &pcFilter);
	pcPlus = cBoard.NewTrace();
	pcReference = &OMalloc<CPowerSource>(cBoard.mpcTop, "1.6V", 1.6f);
	pcReference->GetPowerOutPort()->Connect(pcPlus);
	pcComparator = &OMalloc<CAnalogComparator>(cBoard.mpcTop, "IC3", OMalloc<CAnalogComparatorPins>());
	cBoard.Power(pcComparator);
	pcComparator->GetAnalogComparatorPins()->GetPlus()->Connect(pcPlus);
	pcComparator->GetAnalogComparatorPins()->GetMinus()->Connect(pcMinus);
	pcOut = cBoard.NewTrace();
	pcComparator->GetAnalogComparatorPins()->GetOut()->Connect(pcOut);
	AssertString(ANALOG_COMPARATOR_TYPE, pcComparator->GetType());
	cBoard.Start();
	cBoard.Run();
	AssertFloat(3.3f, pcOut->GetVoltage(cBoard.mpcTimeline->GetTime()), 3);

	iWrite = cBoard.mpcTimeline->GetTime();
	cBoard.Write(pcDriver, true);
	TestRCFilterRunTo(&cBoard, iWrite + NanosecondsToTime((int64)4));
	pcCurve = pcFilter->GetRCFilterPins()->GetOut()->GetOutput();
	iCrossing = TestRCFilterCrossing(pcCurve->GetTime(), 0.2f, 3.1f, iTimeConstant, 1.6f + ANALOG_COMPARATOR_OFFSET);
	AssertTrue(TestRCFilterNear(pcCurve->GetTime() + NanosecondsToTime(7.92), iCrossing, 10));

	TestRCFilterRunTo(&cBoard, iCrossing + NanosecondsToTime(2.4));
	AssertFloat(3.3f, pcOut->GetVoltage(cBoard.mpcTimeline->GetTime()), 3);
	TestRCFilterRunTo(&cBoard, iCrossing + NanosecondsToTime((int64)3));
	pcOutput = pcComparator->GetAnalogComparatorPins()->GetOut()->GetOutput();
	AssertTrue(pcOutput->IsSlew());
	AssertFloat(0.3f, pcOutput->GetEndVoltage(), 3);
	AssertTrue(TestRCFilterNear(iCrossing + NanosecondsToTime(2.5), pcOutput->GetTime(), 4));
	cBoard.Run();
	AssertFloat(0.3f, pcOut->GetVoltage(cBoard.mpcTimeline->GetTime()), 3);

	cBoard.Write(pcDriver, false);
	cBoard.Run();
	AssertFloat(3.3f, pcOut->GetVoltage(cBoard.mpcTimeline->GetTime()), 3);

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 74LVC1G17's family: LVC with its VT- and VT+ as VIL and VIH (1.0V and
// 1.5V at 3V, 1.5V and 2.1V at 4.5V, so 1.1V and 1.62V at 3.3V).  Fed by
// a 100ns RC, it goes high where the curve passes 1.62V, 100 x
// ln(2.9 / 1.48) = 67.3ns on, not at LVC's 2.0V, 96.9ns on.
//
//////////////////////////////////////////////////////////////////////////
void TestRCFilterSchmitt(void)
{
	CTestBoard						cBoard;
	CFamilyVoltageConfiguration*	pcSchmitt;
	CTrace*							pcIn;
	CTrace*							pcOut;
	CTrace*							pcBuffered;
	CConstant*						pcDriver;
	CRCFilter*						pcFilter;
	CBuffer*						pcBuffer;
	CPortOutput*					pcCurve;
	int64							iTimeConstant;
	int64							iWrite;
	int64							iCrossing;

	cBoard.Init();
	pcSchmitt = cBoard.mcStore.GetPart("LVC", "17");
	AssertNotNull(pcSchmitt);
	AssertString("LVC 17", pcSchmitt->GetFamily());
	AssertTrue(pcSchmitt == cBoard.mcStore.GetPart("LVC", "17"));
	AssertTrue(cBoard.mcStore.GetPart("LVC", "04") == cBoard.mcStore.Get("LVC"));
	AssertNull(cBoard.mcStore.GetPart("XYZ", "17"));
	AssertFloat(1.62f, pcSchmitt->GetHighVoltageIn(3.3f), 3);
	AssertFloat(1.1f, pcSchmitt->GetLowVoltageIn(3.3f), 3);
	AssertFloat(3.1f, pcSchmitt->GetVoltageOut(true, 3.3f), 3);
	AssertFloat(2.0f, cBoard.mcStore.Get("LVC")->GetHighVoltageIn(3.3f), 3);
	AssertFloat(1.6f, cBoard.mcStore.GetPart("LVC", "14")->GetHighVoltageIn(3.3f), 3);
	AssertFloat(1.475f, cBoard.mcStore.GetPart("VHCT", "14")->GetHighVoltageIn(5.0f), 3);

	iTimeConstant = NanosecondsToTime((int64)100);
	pcDriver = cBoard.Input("A", 0, 1, &pcIn);
	pcOut = TestRCFilterAdd(&cBoard, pcIn, iTimeConstant, iTimeConstant, &pcFilter);
	pcBuffer = &OMalloc<CBuffer>(cBoard.mpcTop, "IC4", OMalloc<CBufferPins>(1, pcSchmitt));
	cBoard.Power(pcBuffer);
	pcBuffer->GetBufferPins()->GetInput(0)->Connect(pcOut);
	pcBuffered = cBoard.NewTrace();
	pcBuffer->GetBufferPins()->GetOutput(0)->Connect(pcBuffered);
	cBoard.Start();
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcBuffered));

	iWrite = cBoard.mpcTimeline->GetTime();
	cBoard.Write(pcDriver, true);
	TestRCFilterRunTo(&cBoard, iWrite + NanosecondsToTime((int64)4));
	pcCurve = pcFilter->GetRCFilterPins()->GetOut()->GetOutput();
	iCrossing = TestRCFilterCrossing(pcCurve->GetTime(), 0.2f, 3.1f, iTimeConstant, 1.62f);
	AssertTrue(TestRCFilterNear(pcCurve->GetTime() + NanosecondsToTime(67.3), iCrossing, 50));
	TestRCFilterRunTo(&cBoard, iCrossing + NanosecondsToTime((int64)6));
	AssertInt(TV_High, cBoard.Value(pcBuffered));
	AssertFalse(cBoard.Value(pcOut) == TV_High);

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRCFilter(void)
{
	BeginTests();

	TestRCFilterRise();
	TestRCFilterPulse();
	TestRCFilterDiode();
	TestRCFilterHolds();
	TestRCFilterComparator();
	TestRCFilterSchmitt();

	TestStatistics();
}

