#ifndef __TEST_CIRCUIT_BOARD_H__
#define __TEST_CIRCUIT_BOARD_H__
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "MicroprocessorLib/LongTime.h"
#include "MicroprocessorLib/Units.h"
#include "MicroprocessorLib/Voltage.h"
#include "MicroprocessorLib/CircuitSimulation.h"
#include "MicroprocessorLib/PowerSource.h"
#include "MicroprocessorLib/PowerInPort.h"
#include "MicroprocessorLib/FamilyVoltageConfigurationStore.h"
#include "MicroprocessorLib/AndGate.h"
#include "MicroprocessorLib/OrGate.h"
#include "MicroprocessorLib/XorGate.h"
#include "MicroprocessorLib/Buffer.h"
#include "MicroprocessorLib/Constant.h"
#include "MicroprocessorLib/Probe.h"
#include "MicroprocessorLib/ClockOscillator.h"
#include "MicroprocessorLib/DTypeFlipFlop.h"
#include "MicroprocessorLib/Counter.h"


enum EGate
{
	G_And,
	G_Nand,
	G_Or,
	G_Nor,
	G_Xor,
	G_Xnor,
};


//////////////////////////////////////////////////////////////////////////
//
// A circuit simulation powered at 3.3V with LVC parts from Logicim's
// propagation times, and helpers to wire it.
//
//////////////////////////////////////////////////////////////////////////
class CTestBoard
{
public:
	CCircuitSimulation					mcSimulation;
	CFamilyVoltageConfigurationStore	mcStore;
	CVoltageConfigurationSource*		mpcFamily;
	CSubcircuitSimulation*				mpcTop;
	CTimeline*							mpcTimeline;
	CTrace*								mpcVCC;
	CTrace*								mpcGND;

	void Init(void)
	{
		CPowerSource*	pcPower;

		CEvent::ResetNextID();
		CTrace::ResetNextID();
		mcStore.Init();
		mcStore.Load("Input/PropagationTimes.csv");
		mpcFamily = mcStore.Get("LVC");

		mcSimulation.Init("Board");
		mpcTop = mcSimulation.GetSubcircuitTopSimulation();
		mpcTimeline = mcSimulation.GetTimeline();
		mpcVCC = NewTrace();
		mpcGND = NewTrace();
		pcPower = &OMalloc<CPowerSource>(mpcTop, "VCC", 3.3f);
		pcPower->GetPowerOutPort()->Connect(mpcVCC);
		pcPower = &OMalloc<CPowerSource>(mpcTop, "GND", 0.0f);
		pcPower->GetPowerOutPort()->Connect(mpcGND);
	}

	void Kill(void)
	{
		mcSimulation.Kill();
		mcStore.Kill();
	}

	CTrace* NewTrace(void)
	{
		return mcSimulation.GetCircuit()->AddTrace();
	}

	void Power(CIntegratedCircuit* pcIntegratedCircuit)
	{
		pcIntegratedCircuit->GetPins()->GetVoltageCommon()->Connect(mpcVCC);
		pcIntegratedCircuit->GetPins()->GetVoltageGround()->Connect(mpcGND);
	}

	//A constant on a new trace, driven when the simulation starts.
	CConstant* Input(const char* szName, int64 iValue, size uiBits, CTrace** apcTraces)
	{
		CConstantPins*	pcPins;
		CConstant*		pcConstant;
		size			i;

		pcConstant = &OMalloc<CConstant>(mpcTop, szName, OMalloc<CConstantPins>(mpcFamily, uiBits), iValue);
		pcPins = pcConstant->GetConstantPins();
		Power(pcConstant);
		for (i = 0; i < uiBits; i++)
		{
			apcTraces[i] = NewTrace();
			pcPins->GetOutput(i)->Connect(apcTraces[i]);
		}
		return pcConstant;
	}

	CTrace* Input(const char* szName, bool bValue)
	{
		CTrace*	pcTrace;

		Input(szName, bValue ? 1 : 0, 1, &pcTrace);
		return pcTrace;
	}

	//A gate whose inputs are connected to the traces, with its output on a new trace.
	CTrace* Gate(EGate eGate, CTrace* pcA, CTrace* pcB, CTrace* pcC = NULL)
	{
		CIntegratedCircuit*	pcGate;
		CArrayLogicPortPtr*	papcInputs;
		CLogicPort*			pcOutput;
		CTrace*				apcInputs[3];
		size				uiInputs;
		size				i;
		CTrace*				pcOut;

		apcInputs[0] = pcA;
		apcInputs[1] = pcB;
		apcInputs[2] = pcC;
		uiInputs = (pcC == NULL) ? 2 : 3;

		if ((eGate == G_And) || (eGate == G_Nand))
		{
			CAndGate*	pcAnd;

			if (eGate == G_And)
			{
				pcAnd = &OMalloc<CAndGate>(mpcTop, "", OMalloc<CAndGatePins>(uiInputs, mpcFamily));
			}
			else
			{
				pcAnd = &OMalloc<CNandGate>(mpcTop, "", OMalloc<CAndGatePins>(uiInputs, mpcFamily));
			}
			pcGate = pcAnd;
			papcInputs = pcAnd->GetAndGatePins()->GetInputs();
			pcOutput = pcAnd->GetAndGatePins()->GetOutput();
		}
		else if ((eGate == G_Or) || (eGate == G_Nor))
		{
			COrGate*	pcOr;

			if (eGate == G_Or)
			{
				pcOr = &OMalloc<COrGate>(mpcTop, "", OMalloc<COrGatePins>(uiInputs, mpcFamily));
			}
			else
			{
				pcOr = &OMalloc<CNorGate>(mpcTop, "", OMalloc<COrGatePins>(uiInputs, mpcFamily));
			}
			pcGate = pcOr;
			papcInputs = pcOr->GetOrGatePins()->GetInputs();
			pcOutput = pcOr->GetOrGatePins()->GetOutput();
		}
		else
		{
			CXorGate*	pcXor;

			if (eGate == G_Xor)
			{
				pcXor = &OMalloc<CXorGate>(mpcTop, "", OMalloc<CXorGatePins>(uiInputs, mpcFamily));
			}
			else
			{
				pcXor = &OMalloc<CXnorGate>(mpcTop, "", OMalloc<CXorGatePins>(uiInputs, mpcFamily));
			}
			pcGate = pcXor;
			papcInputs = pcXor->GetXorGatePins()->GetInputs();
			pcOutput = pcXor->GetXorGatePins()->GetOutput();
		}

		Power(pcGate);
		for (i = 0; i < uiInputs; i++)
		{
			papcInputs->GetPtr(i)->Connect(apcInputs[i]);
		}
		pcOut = NewTrace();
		pcOutput->Connect(pcOut);
		return pcOut;
	}

	//A gate whose output drives an existing trace, for feedback.
	void GateTo(EGate eGate, CTrace* pcA, CTrace* pcB, CTrace* pcOut)
	{
		CTrace*		pcNew;
		CPort*		pcOutput;

		pcNew = Gate(eGate, pcA, pcB);
		pcOutput = pcNew->GetConnectedPort(0);
		pcOutput->Disconnect(mpcTimeline);
		pcOutput->Connect(pcOut);
	}

	CTrace* Buffer(CTrace* pcIn, bool bInvert)
	{
		CBuffer*		pcBuffer;
		CTrace*			pcOut;

		if (bInvert)
		{
			pcBuffer = &OMalloc<CInverter>(mpcTop, "", OMalloc<CBufferPins>(1, mpcFamily));
		}
		else
		{
			pcBuffer = &OMalloc<CBuffer>(mpcTop, "", OMalloc<CBufferPins>(1, mpcFamily));
		}
		Power(pcBuffer);
		pcBuffer->GetBufferPins()->GetInput(0)->Connect(pcIn);
		pcOut = NewTrace();
		pcBuffer->GetBufferPins()->GetOutput(0)->Connect(pcOut);
		return pcOut;
	}

	CTrace* Not(CTrace* pcIn)
	{
		return Buffer(pcIn, true);
	}

	CProbe* Probe(CTrace** apcTraces, size uiBits)
	{
		CProbePins*	pcPins;
		CProbe*		pcProbe;
		size		i;

		pcProbe = &OMalloc<CProbe>(mpcTop, "", OMalloc<CProbePins>(mpcFamily, uiBits));
		pcPins = pcProbe->GetProbePins();
		Power(pcProbe);
		for (i = 0; i < uiBits; i++)
		{
			pcPins->GetInput(i)->Connect(apcTraces[i]);
		}
		return pcProbe;
	}

	CProbe* Probe(CTrace* pcTrace)
	{
		return Probe(&pcTrace, 1);
	}

	void Start(void)
	{
		mcSimulation.SimulationStarted();
	}

	void Run(void)
	{
		mpcTimeline->Run();
	}

	uint64 Read(CProbe* pcProbe)
	{
		return pcProbe->ReadValue(mpcTimeline);
	}

	ETraceValue Value(CTrace* pcTrace)
	{
		return mpcFamily->GetValue(pcTrace->GetVoltage(mpcTimeline->GetTime()), 3.3f);
	}

	void Write(CConstant* pcConstant, bool bValue)
	{
		pcConstant->GetConstantPins()->GetOutput(0)->Write(mpcTimeline, bValue);
	}
};


#endif // __TEST_CIRCUIT_BOARD_H__

