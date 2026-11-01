#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/Logger.h"
#include "BaseLib/ArrayChars.h"
#include "MicroprocessorLib/Voltage.h"
#include "MicroprocessorLib/CircuitSimulation.h"
#include "MicroprocessorLib/SubcircuitSimulations.h"
#include "MicroprocessorLib/PowerSource.h"
#include "MicroprocessorLib/Pin.h"
#include "MicroprocessorLib/Splitter.h"
#include "MicroprocessorLib/SubcircuitInstance.h"
#include "MicroprocessorLib/PowerPinNames.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPassiveResetIDs(void)
{
	CCircuitSimulation::ResetNextID();
	CSubcircuitSimulation::ResetNextID();
	CTrace::ResetNextID();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPassiveCircuitSimulation(void)
{
	CCircuitSimulation		cSimulation;
	CCircuitSimulation		cUnnamed;
	CSubcircuitSimulation*	pcTop;
	CPowerSource*			pcPower;
	CChars					sz;
	SLogConfig				sLogConfig;

	TestPassiveResetIDs();
	cSimulation.Init("Main");
	cUnnamed.Init(7, "");
	AssertLong(1, cSimulation.GetID());
	AssertLong(7, cUnnamed.GetID());
	AssertString("Main", cSimulation.GetName());
	AssertLong(0, cSimulation.GetTime());
	AssertTrue(cSimulation.GetTimeline()->GetContext() == &cSimulation);

	sz.Init();
	cSimulation.AppendDescription(&sz);
	AssertString("Main (1)", sz.Text());
	sz.Clear();
	cUnnamed.AppendDescription(&sz);
	AssertString("7", sz.Text());
	sz.Clear();

	pcTop = cSimulation.GetSubcircuitTopSimulation();
	AssertTrue(pcTop->IsTop());
	AssertFalse(pcTop->IsInstance());
	AssertLong(1, pcTop->GetID());
	AssertLong(2, cUnnamed.GetSubcircuitTopSimulation()->GetID());
	AssertTrue(pcTop->GetCircuit() == cSimulation.GetCircuit());
	AssertTrue(pcTop->GetTimeline() == cSimulation.GetTimeline());
	AssertString("Main", pcTop->GetName());
	pcTop->AppendDescription(&sz);
	AssertString("Type[Top] ID[1] Simulation[Main (1)] ", sz.Text());
	sz.Kill();

	AssertTrue(cSimulation.GetCircuit()->IsEmpty());
	pcPower = &OMalloc<CPowerSource>(pcTop, "VCC", 5.0f);
	AssertFalse(cSimulation.GetCircuit()->IsEmpty());
	AssertSize(1, cSimulation.GetCircuit()->NumPassives());

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cSimulation.Reset());
	gcLogger.SetConfig(&sLogConfig);

	AssertTrue(cSimulation.GetCircuit()->Remove(pcPower));
	AssertTrue(cSimulation.GetCircuit()->IsEmpty());
	AssertTrue(cSimulation.Reset());
	AssertTrue(cSimulation.GetTimeline()->GetContext() == &cSimulation);

	cSimulation.Kill();
	cUnnamed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPassivePowerSource(void)
{
	CCircuitSimulation	cSimulation;
	CPowerSource*		pcPower;
	CTrace*				pcTrace;
	CChars				sz;

	TestPassiveResetIDs();
	cSimulation.Init("Main");
	pcPower = &OMalloc<CPowerSource>(cSimulation.GetSubcircuitTopSimulation(), "Supply", 3.3f);
	AssertString("Power Source", pcPower->GetType());
	AssertString("Supply", pcPower->GetComponentName());
	AssertSize(1, pcPower->NumPorts());
	AssertTrue(pcPower->GetPort(POWER_PIN_POWER) == pcPower->GetPowerOutPort());
	AssertInt(PT_PowerOut, pcPower->GetPowerOutPort()->GetPortType());

	sz.Init();
	pcPower->AppendDescription(&sz);
	AssertString("Power Source \"Supply\"", sz.Text());
	sz.Clear();
	pcPower->GetPowerOutPort()->AppendDescription(&sz);
	AssertString("Power (3.3V)", sz.Text());
	sz.Kill();

	pcTrace = cSimulation.GetCircuit()->AddTrace();
	pcPower->GetPowerOutPort()->Connect(pcTrace);
	AssertFloat(3.3f, pcTrace->GetVoltage(0), 3);
	cSimulation.SimulationStarted();
	AssertFloat(3.3f, pcTrace->GetVoltage(0), 3);

	cSimulation.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPassivePin(void)
{
	CCircuitSimulation	cSimulation;
	CSubcircuitSimulation*	pcTop;
	CPin*				pcPin;
	CPowerSource*		pcPower;
	CTrace*				pcVCC;
	CTrace*				pcBit;
	SLogConfig			sLogConfig;

	TestPassiveResetIDs();
	cSimulation.Init("Main");
	pcTop = cSimulation.GetSubcircuitTopSimulation();
	pcPin = &OMalloc<CPin>(pcTop, "Data", 4);
	AssertString("Pin", pcPin->GetType());
	AssertSize(4, pcPin->NumTracePorts());
	AssertSize(6, pcPin->NumPorts());
	AssertString("Port 0", pcPin->GetTracePort(0)->GetName());
	AssertString("Port 3", pcPin->GetTracePort(3)->GetName());
	AssertNull(pcPin->GetTracePort(4));
	AssertInt(PT_Passive, pcPin->GetTracePort(0)->GetPortType());
	AssertTrue(pcPin->GetPort("Port 2") == pcPin->GetTracePort(2));
	AssertTrue(pcPin->GetPort(POWER_PIN_VCC) == pcPin->GetVoltageCommon());
	AssertTrue(pcPin->GetPort(POWER_PIN_GND) == pcPin->GetVoltageGround());
	AssertTrue(pcPin->GetVoltageCommon()->IsPowerIn());

	sLogConfig = gcLogger.SetSilent();
	AssertNull(pcPin->GetPort("Port 4"));
	gcLogger.SetConfig(&sLogConfig);

	AssertFalse(IsVoltage(pcPin->GetVCC(0)));
	pcVCC = cSimulation.GetCircuit()->AddTrace();
	pcPower = &OMalloc<CPowerSource>(pcTop, "VCC", 5.0f);
	pcPower->GetPowerOutPort()->Connect(pcVCC);
	pcPin->GetVoltageCommon()->Connect(pcVCC);
	AssertFloat(5.0f, pcPin->GetVCC(0), 3);

	//A trace port neither drives nor reads.
	pcBit = cSimulation.GetCircuit()->AddTrace();
	pcPin->GetTracePort(0)->Connect(pcBit);
	AssertFalse(IsVoltage(pcBit->GetVoltage(0)));
	AssertSize(1, pcBit->NumConnectedPorts());

	//Removing the pin frees it, which disconnects its ports.
	AssertTrue(cSimulation.GetCircuit()->Remove(pcPin));
	AssertSize(0, pcBit->NumConnectedPorts());
	AssertSize(1, pcVCC->NumConnectedPorts());

	cSimulation.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPassiveSplitter(void)
{
	CCircuitSimulation	cSimulation;
	CSplitter*			pcSplitter;
	CArrayChars			aszStart;
	CArrayChars			aszEnd;

	TestPassiveResetIDs();
	cSimulation.Init("Main");
	aszStart.Init();
	aszStart.Add("D0");
	aszStart.Add("D1");
	aszEnd.Init();
	aszEnd.Add("D[0..1]");

	pcSplitter = &OMalloc<CSplitter>(cSimulation.GetSubcircuitTopSimulation(), "Split", &aszStart, &aszEnd);
	aszStart.Kill();
	aszEnd.Kill();

	AssertString("Splitter", pcSplitter->GetType());
	AssertSize(2, pcSplitter->NumStartPorts());
	AssertSize(1, pcSplitter->NumEndPorts());
	AssertSize(3, pcSplitter->NumPorts());
	AssertString("D0", pcSplitter->GetStartPort(0)->GetName());
	AssertString("D1", pcSplitter->GetStartPort(1)->GetName());
	AssertString("D[0..1]", pcSplitter->GetEndPort(0)->GetName());
	AssertNull(pcSplitter->GetEndPort(1));
	AssertTrue(pcSplitter->GetPort("D1") == pcSplitter->GetStartPort(1));

	cSimulation.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPassiveSubcircuitInstance(void)
{
	CCircuitSimulation				cSimulation;
	CSubcircuitInstance*			pcInstance;
	CSubcircuitInstanceSimulation*	pcInstanceSimulation;
	CArrayTracePortPtr*				papcPorts;
	CChars							sz;
	SLogConfig						sLogConfig;

	TestPassiveResetIDs();
	cSimulation.Init("Main");
	pcInstance = &OMalloc<CSubcircuitInstance>(cSimulation.GetSubcircuitTopSimulation(), "ALU");
	AssertString("Subcircuit instance", pcInstance->GetType());

	papcPorts = pcInstance->AddTracePorts("A", 8);
	AssertNotNull(papcPorts);
	AssertSize(8, papcPorts->NumElements());
	AssertString("A 0", papcPorts->GetPtr(0)->GetName());
	AssertString("A 7", papcPorts->GetPtr(7)->GetName());
	AssertNotNull(pcInstance->AddTracePorts("Carry", 1));
	AssertSize(2, pcInstance->NumNamedPins());
	AssertString("Carry", pcInstance->GetPinName(1));
	AssertNull(pcInstance->GetPinName(2));
	AssertSize(9, pcInstance->NumPorts());
	AssertTrue(pcInstance->GetTracePorts("A") == papcPorts);
	AssertSize(1, pcInstance->GetTracePorts("Carry")->NumElements());

	sLogConfig = gcLogger.SetSilent();
	AssertNull(pcInstance->AddTracePorts("A", 1));
	AssertNull(pcInstance->GetTracePorts("B"));
	gcLogger.SetConfig(&sLogConfig);
	AssertSize(9, pcInstance->NumPorts());

	AssertNull(pcInstance->GetSubcircuitInstanceSimulation());
	pcInstanceSimulation = cSimulation.AddInstanceSimulation(pcInstance);
	pcInstance->SetSubcircuitInstanceSimulation(pcInstanceSimulation);
	AssertTrue(pcInstance->GetSubcircuitInstanceSimulation() == pcInstanceSimulation);
	AssertTrue(pcInstanceSimulation->GetSubcircuitInstance() == pcInstance);
	AssertTrue(pcInstanceSimulation->IsInstance());
	AssertFalse(pcInstanceSimulation->IsTop());
	AssertLong(2, pcInstanceSimulation->GetID());
	AssertTrue(pcInstanceSimulation->GetCircuit() == cSimulation.GetCircuit());
	AssertSize(1, cSimulation.NumInstanceSimulations());
	AssertTrue(cSimulation.GetInstanceSimulation(0) == pcInstanceSimulation);

	sz.Init();
	pcInstanceSimulation->AppendDescription(&sz);
	AssertString("Type[Instance] ID[2] Simulation[Main (1)]  Name[ALU]", sz.Text());
	sz.Kill();

	//A loaded one is given its id; later ones come after it.
	pcInstanceSimulation = cSimulation.AddInstanceSimulation(10);
	AssertLong(10, pcInstanceSimulation->GetID());
	AssertNull(pcInstanceSimulation->GetSubcircuitInstance());
	AssertLong(11, cSimulation.AddInstanceSimulation(pcInstance)->GetID());

	cSimulation.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPassiveSubcircuitSimulations(void)
{
	CCircuitSimulation				cMain;
	CCircuitSimulation				cOther;
	CSubcircuitSimulations			cSimulations;
	CSubcircuitInstanceSimulation*	pcInstance;
	CArraySubcircuitSimulationPtr	apcSimulations;
	SLogConfig						sLogConfig;

	TestPassiveResetIDs();
	cMain.Init("Main");
	cOther.Init("Other");
	pcInstance = cMain.AddInstanceSimulation((CSubcircuitInstance*)NULL);

	cSimulations.Init();
	AssertTrue(cSimulations.Add(cMain.GetSubcircuitTopSimulation()));
	AssertTrue(cSimulations.Add(cOther.GetSubcircuitTopSimulation()));
	AssertTrue(cSimulations.Add(pcInstance));
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cSimulations.Add(pcInstance));
	gcLogger.SetConfig(&sLogConfig);

	AssertSize(2, cSimulations.NumCircuitSimulations());
	AssertTrue(cSimulations.GetCircuitSimulation(0) == &cMain);
	AssertTrue(cSimulations.GetCircuitSimulation(1) == &cOther);
	AssertNull(cSimulations.GetCircuitSimulation(2));
	AssertSize(2, cSimulations.GetSubcircuitSimulations(&cMain)->NumElements());

	apcSimulations.Init();
	cSimulations.GetSubcircuitTopSimulations(&apcSimulations);
	AssertSize(2, apcSimulations.NumElements());
	AssertTrue(apcSimulations.GetPtr(0) == cMain.GetSubcircuitTopSimulation());
	apcSimulations.ReInit();
	cSimulations.GetSubcircuitInstanceSimulations(&apcSimulations);
	AssertSize(1, apcSimulations.NumElements());
	AssertTrue(apcSimulations.GetPtr(0) == pcInstance);
	apcSimulations.ReInit();
	cSimulations.GetSubcircuitSimulations(&apcSimulations);
	AssertSize(3, apcSimulations.NumElements());
	apcSimulations.Kill();

	//The last of a circuit simulation's drops it.
	AssertTrue(cSimulations.Remove(cOther.GetSubcircuitTopSimulation()));
	AssertSize(1, cSimulations.NumCircuitSimulations());
	AssertNull(cSimulations.GetSubcircuitSimulations(&cOther));
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cSimulations.Remove(cOther.GetSubcircuitTopSimulation()));
	gcLogger.SetConfig(&sLogConfig);

	cSimulations.Clear();
	AssertSize(0, cSimulations.NumCircuitSimulations());
	cSimulations.Kill();

	cMain.Kill();
	cOther.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPassive(void)
{
	BeginTests();

	TestPassiveCircuitSimulation();
	TestPassivePowerSource();
	TestPassivePin();
	TestPassiveSplitter();
	TestPassiveSubcircuitInstance();
	TestPassiveSubcircuitSimulations();

	TestStatistics();
}

