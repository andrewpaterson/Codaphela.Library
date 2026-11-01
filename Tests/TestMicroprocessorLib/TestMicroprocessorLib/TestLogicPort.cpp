#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Logger.h"
#include "BaseLib/PointerFunctions.h"
#include "MicroprocessorLib/LongTime.h"
#include "MicroprocessorLib/Voltage.h"
#include "MicroprocessorLib/Timeline.h"
#include "MicroprocessorLib/CircuitSimulation.h"
#include "MicroprocessorLib/PowerSource.h"
#include "MicroprocessorLib/PowerInPort.h"
#include "MicroprocessorLib/PowerOutPort.h"
#include "MicroprocessorLib/LogicPort.h"
#include "MicroprocessorLib/LogicPortBus.h"
#include "MicroprocessorLib/PortEvent.h"
#include "MicroprocessorLib/TickEvent.h"
#include "MicroprocessorLib/FixedVoltageConfiguration.h"
#include "MicroprocessorLib/FamilyVoltageConfiguration.h"
#include "MicroprocessorLib/W65C02Pins.h"
#include "MicroprocessorLib/MetaTrace.h"
#include "MicroprocessorLib/MetaBus8.h"
#include "MicroprocessorLib/MetaBus16.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// An input and an output.
//
//////////////////////////////////////////////////////////////////////////
class CTestBufferPins : public CPins
{
CONSTRUCTABLE(CTestBufferPins);
public:
	CLogicPort*		mpcIn;
	CLogicPort*		mpcOut;

	void Init(CVoltageConfigurationSource* pcSource)
	{
		PreInit();

		CPins::Init();
		mpcIn = NewMalloc<CLogicPort>();
		mpcIn->Init(PT_Input, this, "In", pcSource);
		mpcOut = NewMalloc<CLogicPort>();
		mpcOut->Init(PT_Output, this, "Out", pcSource);

		PostInit();
	}

	void Class(void) override
	{
		CPins::Class();

		U_Pointer(mpcIn);
		U_Pointer(mpcOut);
	}
};


//////////////////////////////////////////////////////////////////////////
//
// Copies its input to its output, and logs "name:value@time;" for each
// input transition.
//
//////////////////////////////////////////////////////////////////////////
class CTestBuffer : public CIntegratedCircuit
{
CONSTRUCTABLE(CTestBuffer);
public:
	CChars*		mpszLog;

	void Init(CSubcircuitSimulation* pcSimulation, const char* szName, CVoltageConfigurationSource* pcSource, CChars* pszLog)
	{
		PreInit();

		CIntegratedCircuit::Init(pcSimulation, szName, OMalloc<CTestBufferPins>(pcSource));
		mpszLog = pszLog;

		PostInit();
	}

	void Class(void) override
	{
		CIntegratedCircuit::Class();

		U_Pointer(mpszLog);
	}

	CTestBufferPins* GetBufferPins(void) { return (CTestBufferPins*)&mpPins; }
	void SimulationStarted(CTimeline* pcTimeline) {}
	const char* GetType(void) { return "Buffer"; }

	void InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
	{
		STraceValue	sValue;

		sValue = pcPort->Read(pcTimeline);
		mpszLog->Append(mszName);
		mpszLog->Append(':');
		sValue.Print(mpszLog);
		mpszLog->Append('@');
		mpszLog->Append(pcTimeline->GetTime());
		mpszLog->Append(';');

		if (sValue.IsHigh() || sValue.IsLow())
		{
			GetBufferPins()->mpcOut->Write(pcTimeline, sValue.IsHigh());
		}
	}
};


//////////////////////////////////////////////////////////////////////////
//
// Toggles its output every period, from ticks.
//
//////////////////////////////////////////////////////////////////////////
class CTestClock : public CIntegratedCircuit
{
CONSTRUCTABLE(CTestClock);
public:
	int64	miPeriod;
	bool	mbHigh;
	size	muiTicks;

	void Init(CSubcircuitSimulation* pcSimulation, const char* szName, CVoltageConfigurationSource* pcSource, int64 iPeriod)
	{
		PreInit();

		CIntegratedCircuit::Init(pcSimulation, szName, OMalloc<CTestBufferPins>(pcSource));
		miPeriod = iPeriod;
		ResetState();

		PostInit();
	}

	void Class(void) override
	{
		CIntegratedCircuit::Class();

		U_Int64(miPeriod);
		U_Bool(mbHigh);
		U_Size(muiTicks);
	}

	const char* GetType(void) { return "Clock"; }
	void InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort) {}

	void ResetState(void)
	{
		mbHigh = false;
		muiTicks = 0;
	}

	void SimulationStarted(CTimeline* pcTimeline)
	{
		CTickEvent::Schedule(miPeriod, this, pcTimeline);
	}

	void ExecuteTick(CTimeline* pcTimeline)
	{
		mbHigh = !mbHigh;
		muiTicks++;
		((CTestBufferPins*)&mpPins)->mpcOut->Write(pcTimeline, mbHigh);
		CTickEvent::Schedule(miPeriod, this, pcTimeline);
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A timeline, a circuit and its 4V supply.
//
//////////////////////////////////////////////////////////////////////////
class CTestLogicCircuit
{
public:
	CCircuitSimulation			mcSimulation;
	CTimeline*					mpcTimeline;
	CCircuit*					mpcCircuit;
	CSubcircuitSimulation*		mpcTop;
	CTrace*						mpcVCC;
	CTrace*						mpcGND;
	CFixedVoltageConfiguration	mcFixed;
	CChars						mszLog;

	void Init(void)
	{
		CEvent::ResetNextID();
		CTrace::ResetNextID();
		mcSimulation.Init("Test");
		mpcTimeline = mcSimulation.GetTimeline();
		mpcCircuit = mcSimulation.GetCircuit();
		mpcTop = mcSimulation.GetSubcircuitTopSimulation();
		mpcVCC = mpcCircuit->AddTrace();
		mpcGND = mpcCircuit->AddTrace();
		AddPower("VCC", 4.0f)->GetPowerOutPort()->Connect(mpcVCC);
		AddPower("GND", 0.0f)->GetPowerOutPort()->Connect(mpcGND);

		//Outputs 0V and 4V, inputs low at 1V and high at 3V, 1024 to propagate: holds of 512 and slews of 1024.
		mcFixed.Init(4.0f, 1.0f, 3.0f, 0.0f, 4.0f, 1024, 1024);
		mszLog.Init();
	}

	void Kill(void)
	{
		mcSimulation.Kill();
		mcFixed.Kill();
		mszLog.Kill();
	}

	CPowerSource* AddPower(const char* szName, float32 fVoltage)
	{
		CPowerSource*	pcPower;

		pcPower = &OMalloc<CPowerSource>(mpcTop, szName, fVoltage);
		return pcPower;
	}

	void Power(CIntegratedCircuit* pcIntegratedCircuit)
	{
		pcIntegratedCircuit->GetPins()->GetVoltageCommon()->Connect(mpcVCC);
		pcIntegratedCircuit->GetPins()->GetVoltageGround()->Connect(mpcGND);
	}

	CTestBuffer* AddBuffer(const char* szName, CVoltageConfigurationSource* pcSource)
	{
		CTestBuffer*	pcBuffer;

		pcBuffer = &OMalloc<CTestBuffer>(mpcTop, szName, pcSource, &mszLog);
		Power(pcBuffer);
		return pcBuffer;
	}
};


#define AssertLog(e)	AssertString(e, cTest.mszLog.Text()); cTest.mszLog.Clear()


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogicPortTrace(void)
{
	CTestLogicCircuit	cTest;
	CTrace*				pcTrace;
	CPowerSource*		pcHigh;
	CPowerSource*		pcLow;
	CChars				sz;
	SLogConfig			sLogConfig;

	cTest.Init();
	pcTrace = cTest.mpcCircuit->AddTrace();
	AssertLong(3, pcTrace->GetID());
	AssertFalse(IsVoltage(pcTrace->GetVoltage(0)));
	AssertFloat(0.0f, pcTrace->GetShortVoltage(0), 3);

	pcHigh = cTest.AddPower("High", 4.0f);
	pcLow = cTest.AddPower("Low", 1.0f);
	AssertTrue(pcHigh->GetPowerOutPort()->Connect(pcTrace));
	AssertFloat(4.0f, pcTrace->GetVoltage(0), 3);
	AssertTrue(pcLow->GetPowerOutPort()->Connect(pcTrace));
	AssertFloat(2.5f, pcTrace->GetVoltage(0), 3);
	AssertFloat(3.0f, pcTrace->GetShortVoltage(0), 3);
	AssertFloat(1.0f, pcTrace->GetMinimumVoltage(0), 3);
	AssertFloat(4.0f, pcTrace->GetMaximumVoltage(0), 3);
	AssertSize(2, pcTrace->NumConnectedPorts());
	AssertLong(3, pcHigh->GetPowerOutPort()->GetTraceID());

	sz.Init();
	pcTrace->AppendVoltage(&sz, 0);
	AssertString("2.5V", sz.Text());
	sz.Clear();
	pcHigh->GetPowerOutPort()->AppendDescription(&sz);
	AssertString("Power (4.0V)", sz.Text());
	sz.Kill();

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(pcHigh->GetPowerOutPort()->Connect(pcTrace));
	gcLogger.SetConfig(&sLogConfig);

	pcHigh->GetPowerOutPort()->Disconnect(cTest.mpcTimeline);
	AssertSize(1, pcTrace->NumConnectedPorts());
	AssertFloat(1.0f, pcTrace->GetVoltage(0), 3);
	AssertLong(0, pcHigh->GetPowerOutPort()->GetTraceID());

	AssertTrue(cTest.mpcCircuit->Remove(pcLow));
	AssertSize(0, pcTrace->NumConnectedPorts());
	cTest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A driver's rising edge through a buffer.
//
//////////////////////////////////////////////////////////////////////////
void TestLogicPortSlewAndTransition(void)
{
	CTestLogicCircuit	cTest;
	CTestBuffer*		pcDriver;
	CTestBuffer*		pcBuffer;
	CLogicPort*			pcDriverOut;
	CLogicPort*			pcBufferIn;
	CLogicPort*			pcBufferOut;
	CTrace*				pcA;
	CTrace*				pcB;
	CChars				sz;
	int64				iT;

	cTest.Init();
	pcDriver = cTest.AddBuffer("D", &cTest.mcFixed);
	pcBuffer = cTest.AddBuffer("U1", &cTest.mcFixed);
	cTest.mcSimulation.SimulationStarted();
	AssertTrue(pcBuffer->IsPowered(0));

	pcDriverOut = pcDriver->GetBufferPins()->mpcOut;
	pcBufferIn = pcBuffer->GetBufferPins()->mpcIn;
	pcBufferOut = pcBuffer->GetBufferPins()->mpcOut;
	pcA = cTest.mpcCircuit->AddTrace();
	pcB = cTest.mpcCircuit->AddTrace();
	pcDriverOut->Connect(pcA);
	pcBufferIn->Connect(pcA);
	pcBufferOut->Connect(pcB);

	AssertInt(TV_Undriven, pcBufferIn->ReadValue(0).eValue);

	//An undriven trace driven high takes the level at once, after the hold of 512, and the buffer sees it next tick.
	pcDriverOut->Write(cTest.mpcTimeline, true);
	AssertTrue(cTest.mpcTimeline->RunToTime(512));
	AssertTrue(pcDriverOut->GetOutput()->IsSlew());
	AssertFloat(4.0f, pcDriverOut->GetOutput()->GetStartVoltage(), 3);
	AssertLong(1, pcDriverOut->GetOutput()->GetSlewTime());
	AssertInt(TV_High, pcBufferIn->ReadValue(512).eValue);
	AssertLong(513, pcBufferIn->GetEvent(0)->GetTime());
	cTest.mpcTimeline->Run();
	AssertLog("U1:1@513;");
	AssertLong(1026, cTest.mpcTimeline->GetTime());  //The buffer's output, driven from undriven too.
	AssertFloat(4.0f, pcB->GetVoltage(1026), 3);

	//Falling at 1026: hold 512, slew 4V to 0V over 1024, crossing 1V 768 later.
	pcDriverOut->Write(cTest.mpcTimeline, false);
	cTest.mpcTimeline->Run();
	AssertLog("U1:0@2306;");
	AssertLong(2306 + 512 + 1024, cTest.mpcTimeline->GetTime());
	AssertFloat(0.0f, pcB->GetVoltage(cTest.mpcTimeline->GetTime()), 3);
	iT = cTest.mpcTimeline->GetTime();

	//Rising from 0V: hold 512 then slew 0V to 4V over 1024, crossing 3V at 512 + 768.
	pcDriverOut->Write(cTest.mpcTimeline, true);
	AssertSize(1, pcDriverOut->NumEvents());
	sz.Init();
	pcDriverOut->AppendEvents(&sz);
	AssertString("Events [1]:  Slew: Time [4354] ---- -> 4.0V,  ", sz.Text());
	sz.Kill();

	AssertTrue(cTest.mpcTimeline->RunToTime(512));
	AssertLong(iT + 512, cTest.mpcTimeline->GetTime());
	AssertTrue(pcDriverOut->GetOutput()->IsSlew());
	AssertLong(1024, pcDriverOut->GetOutput()->GetSlewTime());
	AssertFloat(0.0f, pcDriverOut->GetOutput()->GetStartVoltage(), 3);
	AssertSize(1, pcDriverOut->NumEvents());  //The drive at the end of the slew.
	AssertSize(1, pcBufferIn->NumEvents());  //The transition.
	AssertLong(iT + 1536, pcDriverOut->GetEvent(0)->GetTime());
	AssertLong(iT + 1280, pcBufferIn->GetEvent(0)->GetTime());

	AssertFloat(0.0f, pcA->GetVoltage(iT + 512), 3);
	AssertFloat(2.0f, pcA->GetVoltage(iT + 1024), 3);
	AssertInt(TV_Unsettled, pcBufferIn->ReadValue(iT + 1024).eValue);
	AssertFloat(3.0f, pcA->GetVoltage(iT + 1280), 3);
	AssertInt(TV_High, pcBufferIn->ReadValue(iT + 1280).eValue);
	AssertFloat(4.0f, pcA->GetVoltage(iT + 1536), 3);
	AssertFalse(IsVoltage(pcA->GetVoltage(iT + 1537)));  //Until the drive executes.

	AssertTrue(cTest.mpcTimeline->RunToTime(1280 - 512));
	AssertLog("U1:1@5122;");
	AssertSize(1, pcBufferOut->NumEvents());
	AssertLong(iT + 1280 + 512, pcBufferOut->GetEvent(0)->GetTime());

	cTest.mpcTimeline->Run();
	AssertLong(iT + 1280 + 512 + 1024, cTest.mpcTimeline->GetTime());
	AssertTrue(pcDriverOut->GetOutput()->IsDrive());
	AssertFloat(4.0f, pcA->GetVoltage(cTest.mpcTimeline->GetTime()), 3);
	AssertFloat(4.0f, pcB->GetVoltage(cTest.mpcTimeline->GetTime()), 3);
	AssertInt(TV_High, pcBufferIn->Read(cTest.mpcTimeline).eValue);
	AssertSize(0, pcDriverOut->NumEvents());
	AssertSize(0, pcBufferIn->NumEvents());
	AssertSize(0, pcBufferOut->NumEvents());
	AssertLog("");

	cTest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Driving low from undriven doesn't change the voltage, which starts
// from the low voltage out, but the inputs on the trace still see it.
// Driving low again on a driven trace is not a transition.
//
//////////////////////////////////////////////////////////////////////////
void TestLogicPortUndrivenToLow(void)
{
	CTestLogicCircuit	cTest;
	CTestBuffer*		pcDriver;
	CTestBuffer*		pcBuffer;
	CLogicPort*			pcDriverOut;
	CTrace*				pcA;

	cTest.Init();
	pcDriver = cTest.AddBuffer("D", &cTest.mcFixed);
	pcBuffer = cTest.AddBuffer("U1", &cTest.mcFixed);
	cTest.mcSimulation.SimulationStarted();
	pcDriverOut = pcDriver->GetBufferPins()->mpcOut;
	pcA = cTest.mpcCircuit->AddTrace();
	pcDriverOut->Connect(pcA);
	pcBuffer->GetBufferPins()->mpcIn->Connect(pcA);

	//A hold of 512, the average of the two as the voltage doesn't change, then a transition one later.
	pcDriverOut->Write(cTest.mpcTimeline, false);
	AssertTrue(cTest.mpcTimeline->RunToTime(512));
	AssertSize(1, pcBuffer->GetBufferPins()->mpcIn->NumEvents());
	AssertLong(513, pcBuffer->GetBufferPins()->mpcIn->GetEvent(0)->GetTime());
	cTest.mpcTimeline->Run();
	AssertLog("U1:0@513;");

	pcDriverOut->Write(cTest.mpcTimeline, false);
	cTest.mpcTimeline->Run();
	AssertLog("");
	AssertInt(TV_Low, pcBuffer->GetBufferPins()->mpcIn->Read(cTest.mpcTimeline).eValue);

	cTest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A driver going to high impedance leaves its trace undriven; the input
// on it sees that next tick.  High impedance drops a later pending slew,
// so the last write wins.
//
//////////////////////////////////////////////////////////////////////////
void TestLogicPortImpedanceSeen(void)
{
	CTestLogicCircuit	cTest;
	CTestBuffer*		pcDriver;
	CTestBuffer*		pcBuffer;
	CLogicPort*			pcDriverOut;
	CTrace*				pcA;
	int64				iT;

	cTest.Init();
	pcDriver = cTest.AddBuffer("D", &cTest.mcFixed);
	pcBuffer = cTest.AddBuffer("U1", &cTest.mcFixed);
	cTest.mcSimulation.SimulationStarted();
	pcDriverOut = pcDriver->GetBufferPins()->mpcOut;
	pcA = cTest.mpcCircuit->AddTrace();
	pcDriverOut->Connect(pcA);
	pcBuffer->GetBufferPins()->mpcIn->Connect(pcA);

	pcDriverOut->Write(cTest.mpcTimeline, true);
	cTest.mpcTimeline->Run();
	AssertLog("U1:1@513;");
	iT = cTest.mpcTimeline->GetTime();
	AssertLong(2049, iT);  //The buffer slewed its unconnected output from 0V too.

	//The drive of NaN at 10 (0.01ns), the transition the tick after.
	pcDriverOut->WriteImpedance(cTest.mpcTimeline);
	cTest.mpcTimeline->Run();
	AssertLong(iT + 11, cTest.mpcTimeline->GetTime());
	AssertLog("U1:-@2060;");
	AssertFalse(IsVoltage(pcA->GetVoltage(cTest.mpcTimeline->GetTime())));

	//Driven high again, then at once high impedance: the slew is dropped.
	pcDriverOut->Write(cTest.mpcTimeline, true);
	AssertSize(1, pcDriverOut->NumEvents());
	pcDriverOut->WriteImpedance(cTest.mpcTimeline);
	AssertSize(1, pcDriverOut->NumEvents());
	AssertTrue(pcDriverOut->GetEvent(0)->IsDriveEvent());
	cTest.mpcTimeline->Run();
	AssertLog("");
	AssertFalse(IsVoltage(pcA->GetVoltage(cTest.mpcTimeline->GetTime())));

	cTest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogicPortImpedance(void)
{
	CTestLogicCircuit	cTest;
	CTestBuffer*		pcDriver;
	CLogicPort*			pcDriverOut;
	CTrace*				pcA;

	cTest.Init();
	pcDriver = cTest.AddBuffer("D", &cTest.mcFixed);
	cTest.mcSimulation.SimulationStarted();
	pcDriverOut = pcDriver->GetBufferPins()->mpcOut;
	pcA = cTest.mpcCircuit->AddTrace();
	pcDriverOut->Connect(pcA);

	pcDriverOut->Write(cTest.mpcTimeline, true);
	cTest.mpcTimeline->Run();
	AssertFloat(4.0f, pcA->GetVoltage(cTest.mpcTimeline->GetTime()), 3);

	//0.01ns later.
	pcDriverOut->WriteImpedance(cTest.mpcTimeline);
	AssertLong(cTest.mpcTimeline->GetTime() + 10, pcDriverOut->GetEvent(0)->GetTime());
	cTest.mpcTimeline->Run();
	AssertFalse(IsVoltage(pcA->GetVoltage(cTest.mpcTimeline->GetTime())));
	AssertInt(TV_Undriven, pcDriverOut->Read(cTest.mpcTimeline).eValue);

	cTest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A slew replaces the drive of an earlier slew that hasn't finished.
//
//////////////////////////////////////////////////////////////////////////
void TestLogicPortSlewInterrupted(void)
{
	CTestLogicCircuit	cTest;
	CTestBuffer*		pcDriver;
	CLogicPort*			pcDriverOut;
	CTrace*				pcA;
	int64				iT;

	cTest.Init();
	pcDriver = cTest.AddBuffer("D", &cTest.mcFixed);
	cTest.mcSimulation.SimulationStarted();
	pcDriverOut = pcDriver->GetBufferPins()->mpcOut;
	pcA = cTest.mpcCircuit->AddTrace();
	pcDriverOut->Connect(pcA);

	//Driven low first, from undriven: the slew at 512 and its drive at 513.
	pcDriverOut->Write(cTest.mpcTimeline, false);
	cTest.mpcTimeline->Run();
	iT = cTest.mpcTimeline->GetTime();
	AssertLong(513, iT);

	pcDriverOut->Write(cTest.mpcTimeline, true);
	cTest.mpcTimeline->RunToTime(1024);  //Slewing since 512, at 2V.
	AssertFloat(2.0f, pcA->GetVoltage(iT + 1024), 3);
	AssertSize(1, pcDriverOut->NumEvents());

	//Falling, so a hold of 512.  The first slew's drive runs first at 1536, so the second slew starts from 4V.
	pcDriverOut->Write(cTest.mpcTimeline, false);
	AssertSize(2, pcDriverOut->NumEvents());
	cTest.mpcTimeline->RunToTime(512);
	AssertLong(iT + 1536, cTest.mpcTimeline->GetTime());
	AssertTrue(pcDriverOut->GetOutput()->IsSlew());
	AssertFloat(4.0f, pcDriverOut->GetOutput()->GetStartVoltage(), 3);
	AssertLong(1024, pcDriverOut->GetOutput()->GetSlewTime());
	AssertSize(1, pcDriverOut->NumEvents());
	AssertLong(iT + 2560, pcDriverOut->GetEvent(0)->GetTime());

	cTest.mpcTimeline->Run();
	AssertLong(iT + 2560, cTest.mpcTimeline->GetTime());
	AssertFloat(0.0f, pcA->GetVoltage(iT + 2560), 3);

	cTest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// With a family's configuration a chip with no power doesn't drive.
//
//////////////////////////////////////////////////////////////////////////
void TestLogicPortNoPower(void)
{
	CTestLogicCircuit			cTest;
	CTestBuffer*				pcDriver;
	CLogicPort*					pcDriverOut;
	CFamilyVoltageConfiguration	cFamily;
	CVoltageConfiguration		cConfiguration;

	cTest.Init();
	cFamily.Init("TEST");
	cConfiguration.Init(4.0f, 1.0f, 3.0f, 0.0f, 4.0f, 1024, 1024);
	cFamily.Add(&cConfiguration, LOGIC_LEVEL_TTL);

	pcDriver = &OMalloc<CTestBuffer>(cTest.mpcTop, "D", &cFamily, &cTest.mszLog);
	cTest.mcSimulation.SimulationStarted();
	pcDriverOut = pcDriver->GetBufferPins()->mpcOut;
	AssertFalse(pcDriver->IsPowered(0));

	pcDriverOut->Write(cTest.mpcTimeline, true);
	AssertSize(0, pcDriverOut->NumEvents());

	cTest.Power(pcDriver);
	AssertTrue(pcDriver->IsPowered(0));
	pcDriverOut->Write(cTest.mpcTimeline, true);
	AssertSize(1, pcDriverOut->NumEvents());
	AssertLong(512, pcDriverOut->GetEvent(0)->GetTime());

	cTest.Kill();
	cFamily.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A port connected to a trace already high sees a transition at once.
//
//////////////////////////////////////////////////////////////////////////
void TestLogicPortTraceConnected(void)
{
	CTestLogicCircuit	cTest;
	CTestBuffer*		pcBuffer;
	CLogicPort*			pcBufferIn;
	CPowerSource*		pcHigh;
	CTrace*				pcA;

	cTest.Init();
	pcBuffer = cTest.AddBuffer("U1", &cTest.mcFixed);
	cTest.mcSimulation.SimulationStarted();
	pcBufferIn = pcBuffer->GetBufferPins()->mpcIn;

	pcA = cTest.mpcCircuit->AddTrace();
	pcHigh = cTest.AddPower("High", 4.0f);
	pcHigh->GetPowerOutPort()->Connect(pcA);
	pcBufferIn->Connect(pcA);
	pcBufferIn->TraceConnected(cTest.mpcTimeline);
	AssertSize(1, pcBufferIn->NumEvents());
	AssertLong(1, pcBufferIn->GetEvent(0)->GetTime());

	cTest.mpcTimeline->Run();
	AssertLog("U1:1@1;");

	cTest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Disconnecting a port takes its events off the timeline; killing the
// circuit takes them all off.
//
//////////////////////////////////////////////////////////////////////////
void TestLogicPortDisconnect(void)
{
	CTestLogicCircuit	cTest;
	CTestBuffer*		pcDriver;
	CTestBuffer*		pcBuffer;
	CLogicPort*			pcDriverOut;
	CTrace*				pcA;

	cTest.Init();
	pcDriver = cTest.AddBuffer("D", &cTest.mcFixed);
	pcBuffer = cTest.AddBuffer("U1", &cTest.mcFixed);
	cTest.mcSimulation.SimulationStarted();
	pcDriverOut = pcDriver->GetBufferPins()->mpcOut;
	pcA = cTest.mpcCircuit->AddTrace();
	pcDriverOut->Connect(pcA);
	pcBuffer->GetBufferPins()->mpcIn->Connect(pcA);

	pcDriverOut->Write(cTest.mpcTimeline, true);
	AssertSize(1, cTest.mpcTimeline->NumSimultaneousEvents());
	pcDriverOut->Disconnect(cTest.mpcTimeline);
	AssertSize(0, cTest.mpcTimeline->NumSimultaneousEvents());
	AssertSize(0, pcDriverOut->NumEvents());
	AssertNull(pcDriverOut->GetTrace());
	AssertSize(1, pcA->NumConnectedPorts());

	pcDriverOut->Connect(pcA);
	pcDriverOut->Write(cTest.mpcTimeline, true);
	cTest.mpcTimeline->RunToTime(512);
	AssertSize(1, cTest.mpcTimeline->NumSimultaneousEvents());  //The drive and the transition, both next tick.
	AssertSize(2, cTest.mpcTimeline->GetFirstSimultaneousEvents()->NumEvents());

	cTest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogicPortTick(void)
{
	CTestLogicCircuit	cTest;
	CTestClock*			pcClock;
	CTestBuffer*		pcBuffer;
	CTrace*				pcClockTrace;
	CChars				sz;

	cTest.Init();
	pcClock = &OMalloc<CTestClock>(cTest.mpcTop, "Clock", &cTest.mcFixed, 10000);
	cTest.Power(pcClock);
	pcBuffer = cTest.AddBuffer("U1", &cTest.mcFixed);
	pcClockTrace = cTest.mpcCircuit->AddTrace();
	((CTestBufferPins*)pcClock->GetPins())->mpcOut->Connect(pcClockTrace);
	pcBuffer->GetBufferPins()->mpcIn->Connect(pcClockTrace);

	cTest.mcSimulation.SimulationStarted();
	AssertSize(1, pcClock->NumEvents());

	cTest.mpcTimeline->RunToTime(45000);
	AssertSize(4, pcClock->muiTicks);
	AssertLog("U1:1@10513;U1:0@21280;U1:1@31280;U1:0@41280;");

	sz.Init();
	pcClock->AppendDescription(&sz);
	AssertString("Clock \"Clock\"", sz.Text());
	sz.Kill();

	//Starting again resets what it remembers.
	cTest.mcSimulation.SimulationStarted();
	AssertSize(0, pcClock->muiTicks);
	AssertFalse(pcClock->mbHigh);

	cTest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A CW65C02Pins on analog traces: a CLogicPortBus for the address and
// data and logic ports for the signals.
//
//////////////////////////////////////////////////////////////////////////
void TestLogicPortPins(void)
{
	CTestLogicCircuit	cTest;
	CTestBuffer*		apcBits[8];
	CTestBuffer*		pcSignals;
	CLogicPortBus		cData;
	CW65C02Pins			cPins;
	CTrace*				pcTrace;
	size				i;
	CMetaBus16			cMetaAddress;
	CMetaTrace			cMetaPHI2;

	cTest.Init();
	cData.Init();
	for (i = 0; i < 8; i++)
	{
		apcBits[i] = cTest.AddBuffer("", &cTest.mcFixed);
		cData.Add(apcBits[i]->GetBufferPins()->mpcOut);
		pcTrace = cTest.mpcCircuit->AddTrace();
		apcBits[i]->GetBufferPins()->mpcOut->Connect(pcTrace);
	}
	pcSignals = cTest.AddBuffer("", &cTest.mcFixed);
	cTest.mcSimulation.SimulationStarted();
	AssertSize(8, cData.GetWidth());
	AssertNull(cData.GetPort(8));

	//Address on an ideal bus, data on analog traces.
	cMetaAddress.Init();
	cMetaPHI2.Init(true);
	cPins.Init(&cMetaAddress, &cData, &cMetaPHI2, pcSignals->GetBufferPins()->mpcIn, pcSignals->GetBufferPins()->mpcOut);

	cPins.WriteAddress(cTest.mpcTimeline, 0x1234);
	AssertInt(0x1234, cMetaAddress.Get());
	AssertInt(0x1234, cPins.ReadAddress(cTest.mpcTimeline));

	cPins.WriteData(cTest.mpcTimeline, 0xA5);
	AssertInt(0, cPins.ReadData(cTest.mpcTimeline));  //Not slewed yet.
	cTest.mpcTimeline->Run();
	AssertInt(0xA5, cPins.ReadData(cTest.mpcTimeline));
	AssertInt(TV_High, cPins.ReadPhi2(cTest.mpcTimeline).eValue);
	AssertInt(TV_Undriven, cPins.ReadRES(cTest.mpcTimeline).eValue);
	AssertInt(TV_Undriven, cPins.ReadSOB(cTest.mpcTimeline).eValue);  //Not connected.

	cData.WriteImpedance(cTest.mpcTimeline);
	cTest.mpcTimeline->Run();
	AssertInt(0, cPins.ReadData(cTest.mpcTimeline));
	AssertInt(TV_Undriven, apcBits[0]->GetBufferPins()->mpcOut->Read(cTest.mpcTimeline).eValue);

	cPins.Kill();
	cData.Kill();
	cMetaAddress.Kill();
	cMetaPHI2.Kill();
	cTest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogicPortIdealSignals(void)
{
	CMetaTrace		cTrace;
	CMetaBus8		cBus;
	CLogicSignal*	pcSignal;
	CLogicBus*		pcBus;

	cTrace.Init();
	cBus.Init();
	pcSignal = &cTrace;
	pcBus = &cBus;

	pcSignal->Write(NULL, true);
	AssertTrue(cTrace.IsHigh());
	AssertInt(TV_High, pcSignal->Read(NULL).eValue);
	pcSignal->WriteImpedance(NULL);
	AssertInt(TV_Undriven, pcSignal->Read(NULL).eValue);

	pcBus->Write(NULL, 0x1FF);
	AssertInt(0xFF, cBus.Get());
	AssertInt(0xFF, pcBus->Read(NULL));
	AssertSize(8, pcBus->GetWidth());

	cTrace.Kill();
	cBus.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLogicPort(void)
{
	BeginTests();

	TestLogicPortIdealSignals();
	TestLogicPortTrace();
	TestLogicPortSlewAndTransition();
	TestLogicPortUndrivenToLow();
	TestLogicPortImpedance();
	TestLogicPortImpedanceSeen();
	TestLogicPortSlewInterrupted();
	TestLogicPortNoPower();
	TestLogicPortTraceConnected();
	TestLogicPortDisconnect();
	TestLogicPortTick();
	TestLogicPortPins();

	TestStatistics();
}

