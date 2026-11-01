#include "MicroprocessorLib/SlewEvent.h"
#include "TestCircuitBoard.h"
#include "TestLib/Assert.h"


//The wiring examples are from:
//  NAND only gates and multiplexer:  https://en.wikipedia.org/wiki/NAND_logic
//  Half adder, full adder and the 4 bit ripple carry adder:  https://en.wikipedia.org/wiki/Adder_(electronics)
//  SR NOR and NAND latches:  https://en.wikipedia.org/wiki/Flip-flop_(electronics)
//  Ripple counter (positive edge D flip-flops, QB to D and to the next clock):
//    https://forum.allaboutcircuits.com/threads/d-flip-flop-counter.44242/latest


//////////////////////////////////////////////////////////////////////////
//
// The two input truth table of a gate as 4 bits: bit (A + 2B) is the
// output for inputs A and B.
//
//////////////////////////////////////////////////////////////////////////
uint32 TruthTable(EGate eGate)
{
	CTestBoard	cBoard;
	size		uiRow;
	CProbe*		pcProbe;
	uint32		uiTable;

	uiTable = 0;
	for (uiRow = 0; uiRow < 4; uiRow++)
	{
		cBoard.Init();
		pcProbe = cBoard.Probe(cBoard.Gate(eGate, cBoard.Input("A", (uiRow & 1) != 0), cBoard.Input("B", (uiRow & 2) != 0)));
		cBoard.Start();
		cBoard.Run();
		uiTable |= (uint32)cBoard.Read(pcProbe) << uiRow;
		cBoard.Kill();
	}
	return uiTable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStandardGates(void)
{
	CTestBoard	cBoard;
	CProbe*		pcProbe;
	size		uiRow;
	CTrace*		apcIn[3];

	//Rows 00, 10, 01, 11 (A is bit 0).
	AssertInt(0x8, TruthTable(G_And));
	AssertInt(0x7, TruthTable(G_Nand));
	AssertInt(0xE, TruthTable(G_Or));
	AssertInt(0x1, TruthTable(G_Nor));
	AssertInt(0x6, TruthTable(G_Xor));
	AssertInt(0x9, TruthTable(G_Xnor));

	//Three input gates; XOR is odd parity.
	for (uiRow = 0; uiRow < 8; uiRow++)
	{
		cBoard.Init();
		cBoard.Input("In", uiRow, 3, apcIn);
		pcProbe = cBoard.Probe(cBoard.Gate(G_And, apcIn[0], apcIn[1], apcIn[2]));
		cBoard.Start();
		cBoard.Run();
		AssertLong((uiRow == 7) ? 1 : 0, cBoard.Read(pcProbe));
		cBoard.Kill();

		cBoard.Init();
		cBoard.Input("In", uiRow, 3, apcIn);
		pcProbe = cBoard.Probe(cBoard.Gate(G_Or, apcIn[0], apcIn[1], apcIn[2]));
		cBoard.Start();
		cBoard.Run();
		AssertLong((uiRow != 0) ? 1 : 0, cBoard.Read(pcProbe));
		cBoard.Kill();

		cBoard.Init();
		cBoard.Input("In", uiRow, 3, apcIn);
		pcProbe = cBoard.Probe(cBoard.Gate(G_Xor, apcIn[0], apcIn[1], apcIn[2]));
		cBoard.Start();
		cBoard.Run();
		AssertLong(((uiRow & 1) ^ ((uiRow >> 1) & 1) ^ ((uiRow >> 2) & 1)), cBoard.Read(pcProbe));
		cBoard.Kill();
	}

	//Buffer and inverter.
	for (uiRow = 0; uiRow < 2; uiRow++)
	{
		cBoard.Init();
		apcIn[0] = cBoard.Input("A", uiRow == 1);
		apcIn[1] = cBoard.Buffer(apcIn[0], false);
		apcIn[2] = cBoard.Not(apcIn[0]);
		pcProbe = cBoard.Probe(&apcIn[1], 2);
		cBoard.Start();
		cBoard.Run();
		AssertLong((uiRow == 1) ? 1 : 2, cBoard.Read(pcProbe));
		cBoard.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// One gate with input A driven and input B left floating.
//
//////////////////////////////////////////////////////////////////////////
ETraceValue GateWithFloatingInput(EGate eGate, bool bA)
{
	CTestBoard	cBoard;
	CTrace*		pcOut;
	ETraceValue	eValue;

	cBoard.Init();
	pcOut = cBoard.Gate(eGate, cBoard.Input("A", bA), cBoard.NewTrace());
	cBoard.Start();
	cBoard.Run();
	eValue = cBoard.Value(pcOut);
	cBoard.Kill();
	return eValue;
}


//////////////////////////////////////////////////////////////////////////
//
// An undriven input is unknown: a gate whose output it could change
// stops driving.  A low on an AND, or a high on an OR, decides it anyway.
//
//////////////////////////////////////////////////////////////////////////
void TestStandardGatesUndriven(void)
{
	CTestBoard	cBoard;
	CTrace*		pcOut;

	AssertInt(TV_Low, GateWithFloatingInput(G_And, false));
	AssertInt(TV_Undriven, GateWithFloatingInput(G_And, true));
	AssertInt(TV_High, GateWithFloatingInput(G_Nand, false));
	AssertInt(TV_Undriven, GateWithFloatingInput(G_Nand, true));
	AssertInt(TV_High, GateWithFloatingInput(G_Or, true));
	AssertInt(TV_Undriven, GateWithFloatingInput(G_Or, false));
	AssertInt(TV_Low, GateWithFloatingInput(G_Nor, true));
	AssertInt(TV_Undriven, GateWithFloatingInput(G_Nor, false));
	AssertInt(TV_Undriven, GateWithFloatingInput(G_Xor, true));
	AssertInt(TV_Undriven, GateWithFloatingInput(G_Xnor, false));

	cBoard.Init();
	pcOut = cBoard.Gate(G_Or, cBoard.NewTrace(), cBoard.NewTrace());
	cBoard.Start();
	cBoard.Run();
	AssertInt(TV_Undriven, cBoard.Value(pcOut));
	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Disconnecting the only driver of a trace while the simulation runs: the
// ports on it see it go undriven and the unknown spreads.  Reconnecting
// it drives them again.  A trace with another driver stays driven and
// its ports aren't told.
//
//////////////////////////////////////////////////////////////////////////
void TestStandardUndrivenSpreads(void)
{
	CTestBoard	cBoard;
	CConstant*	pcB;
	CConstant*	pcC;
	CTrace*		pcA;
	CTrace*		pcBTrace;
	CTrace*		pcCTrace;
	CTrace*		pcAnd;
	CTrace*		pcNot;
	CLogicPort*	pcBOut;
	CLogicPort*	pcCOut;

	cBoard.Init();
	pcA = cBoard.Input("A", true);
	pcB = cBoard.Input("B", 1, 1, &pcBTrace);
	pcC = cBoard.Input("C", 1, 1, &pcCTrace);
	pcAnd = cBoard.Gate(G_And, pcA, pcBTrace);
	pcNot = cBoard.Not(pcAnd);
	cBoard.Start();
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcAnd));
	AssertInt(TV_Low, cBoard.Value(pcNot));

	//B's only driver disconnected: the AND, then the inverter, stop driving.
	pcBOut = pcB->GetConstantPins()->GetOutput(0);
	pcBOut->Disconnect(cBoard.mpcTimeline);
	AssertInt(TV_Undriven, cBoard.Value(pcBTrace));
	cBoard.Run();
	AssertInt(TV_Undriven, cBoard.Value(pcAnd));
	AssertInt(TV_Undriven, cBoard.Value(pcNot));

	//Reconnected, everything is driven again.
	AssertTrue(pcBOut->ConnectLive(pcBTrace, cBoard.mpcTimeline));
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcAnd));
	AssertInt(TV_Low, cBoard.Value(pcNot));

	//A second driver on B's trace.  Disconnecting it leaves B driven; only the port itself is told.
	pcCOut = pcC->GetConstantPins()->GetOutput(0);
	pcCOut->Disconnect(cBoard.mpcTimeline);
	cBoard.Run();
	AssertTrue(pcCOut->ConnectLive(pcBTrace, cBoard.mpcTimeline));
	cBoard.Run();
	pcCOut->Disconnect(cBoard.mpcTimeline);
	AssertSize(1, cBoard.mpcTimeline->NumSimultaneousEvents());
	AssertSize(1, cBoard.mpcTimeline->GetFirstSimultaneousEvents()->NumEvents());
	AssertString("CTransitionEvent", cBoard.mpcTimeline->GetFirstSimultaneousEvents()->GetFirst()->ClassName());
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcAnd));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Taking a gate out of a running circuit, as Logicim's editor does: the
// inverter it drove sees its input go undriven.
//
//////////////////////////////////////////////////////////////////////////
void TestStandardRemoveComponent(void)
{
	CTestBoard			cBoard;
	CTrace*				pcAnd;
	CTrace*				pcNot;
	CIntegratedCircuit*	pcGate;
	CCircuit*			pcCircuit;

	cBoard.Init();
	pcAnd = cBoard.Gate(G_And, cBoard.Input("A", true), cBoard.Input("B", true));
	pcNot = cBoard.Not(pcAnd);
	cBoard.Start();
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcNot));

	pcCircuit = cBoard.mcSimulation.GetCircuit();
	pcGate = ((CPins*)pcAnd->GetConnectedPort(0)->GetHolder())->GetIntegratedCircuit();
	AssertString(AND_GATE_TYPE, pcGate->GetType());
	pcGate->Disconnect(cBoard.mpcTimeline);
	pcGate->DisconnectPorts(cBoard.mpcTimeline);
	AssertTrue(pcCircuit->Remove(pcGate));

	cBoard.Run();
	AssertInt(TV_Undriven, cBoard.Value(pcAnd));
	AssertInt(TV_Undriven, cBoard.Value(pcNot));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The descriptions and lookups Logicim's views and debugging use.
//
//////////////////////////////////////////////////////////////////////////
void TestStandardDescriptions(void)
{
	CTestBoard		cBoard;
	CAndGatePins*	pcPins;
	CAndGate*		pcGate;
	CPowerSource*	pcPower;
	CTrace*			pcOut;
	CChars			sz;
	CEvent*			pcEvent;
	CPortEvent*		pcPortEvent;

	cBoard.Init();
	pcGate = &OMalloc<CAndGate>(cBoard.mpcTop, "U1", OMalloc<CAndGatePins>(2, cBoard.mpcFamily));
	pcPins = pcGate->GetAndGatePins();
	cBoard.Power(pcGate);
	pcOut = cBoard.NewTrace();
	pcPins->GetOutput()->Connect(pcOut);
	pcPins->GetInput(0)->Connect(cBoard.Input("A", true));
	pcPins->GetInput(1)->Connect(cBoard.Input("B", true));
	cBoard.Start();

	AssertTrue(pcPins->GetComponent() == pcGate);
	AssertString(AND_GATE_TYPE, pcPins->GetType());

	sz.Init();
	pcPins->GetOutput()->AppendDebugString(&sz);
	AssertString("U1.Output(3)", sz.Text());
	sz.Clear();
	pcPins->GetOutput()->AppendType(&sz);
	AssertString("Logic", sz.Text());
	sz.Clear();
	pcPins->GetVoltageCommon()->AppendType(&sz);
	AssertString("PowerIn", sz.Text());
	sz.Clear();

	pcPower = (CPowerSource*)cBoard.mpcVCC->GetConnectedPort(0)->GetHolder()->GetComponent();
	AssertString("Power Source", pcPower->GetType());
	pcPower->GetPowerOutPort()->AppendDebugString(&sz);
	AssertString("Power.Power (3.3V)", sz.Text());
	sz.Clear();

	//A port event knows its chip and its port's owner.
	cBoard.Run();
	pcPins->GetOutput()->Write(cBoard.mpcTimeline, false);
	pcEvent = pcPins->GetOutput()->GetEvent(0);
	AssertTrue(pcEvent->GetIntegratedCircuit() == pcGate);
	pcPortEvent = (CPortEvent*)pcEvent;
	pcPortEvent->AppendPortOwner(&sz);
	AssertString("AND Gate \"U1\"", sz.Text());
	sz.Clear();
	((CSlewEvent*)pcEvent)->AppendDebugString(&sz);
	AssertString("U1.Output(3)", sz.Text());
	sz.Kill();

	//Disconnecting the gate takes its own events off the timeline; it has none.
	cBoard.mcSimulation.GetCircuit()->DisconnectComponent(pcGate, cBoard.mpcTimeline);
	AssertSize(1, pcPins->GetOutput()->NumEvents());

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Wiring while the simulation runs, as Logicim's editor does.  A port
// connected to a driven trace sees its level; a driver connected to an
// undriven trace is seen by the ports already on it.  Ports on a trace
// that was already driven are not told again.
//
//////////////////////////////////////////////////////////////////////////
void TestStandardLiveWiring(void)
{
	CTestBoard		cBoard;
	CTrace*			pcA;
	CTrace*			pcB;
	CTrace*			pcT;
	CTrace*			pcOut;
	COrGatePins*	pcPins;
	CNorGate*		pcNor;
	CPowerSource*	pcHigh;
	CProbePins*		pcProbePins;
	CProbe*			pcProbe;

	cBoard.Init();
	pcA = cBoard.Input("A", false);
	pcB = cBoard.Input("B", false);
	pcNor = &OMalloc<CNorGate>(cBoard.mpcTop, "U1", OMalloc<COrGatePins>(2, cBoard.mpcFamily));
	pcPins = pcNor->GetOrGatePins();
	cBoard.Power(pcNor);
	pcOut = cBoard.NewTrace();
	pcPins->GetOutput()->Connect(pcOut);
	cBoard.Start();
	cBoard.Run();
	AssertInt(TV_Undriven, cBoard.Value(pcOut));

	//The inputs wired to the low traces each see the low, next tick; the constants driving them aren't told.
	AssertTrue(pcPins->GetInput(0)->ConnectLive(pcA, cBoard.mpcTimeline));
	AssertTrue(pcPins->GetInput(1)->ConnectLive(pcB, cBoard.mpcTimeline));
	AssertSize(1, cBoard.mpcTimeline->NumSimultaneousEvents());
	AssertSize(2, cBoard.mpcTimeline->GetFirstSimultaneousEvents()->NumEvents());
	AssertLong(cBoard.mpcTimeline->GetTime() + 1, cBoard.mpcTimeline->GetFirstSimultaneousEvents()->GetTime());
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcOut));

	//Input 1 disconnected sees undriven; with input 0 low the NOR can't tell, so it stops driving.
	pcPins->GetInput(1)->Disconnect(cBoard.mpcTimeline);
	cBoard.Run();
	AssertInt(TV_Undriven, cBoard.Value(pcOut));

	//Input 1 on an undriven trace, then a supply wired onto it: the input sees the high.
	pcT = cBoard.NewTrace();
	pcPins->GetInput(1)->Connect(pcT);
	pcHigh = &OMalloc<CPowerSource>(cBoard.mpcTop, "High", 3.3f);
	AssertTrue(pcHigh->GetPowerOutPort()->ConnectLive(pcT, cBoard.mpcTimeline));
	AssertSize(1, cBoard.mpcTimeline->NumSimultaneousEvents());
	AssertSize(1, cBoard.mpcTimeline->GetFirstSimultaneousEvents()->NumEvents());
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcOut));

	//A probe wired to the driven output: only the probe is told.
	pcProbe = &OMalloc<CProbe>(cBoard.mpcTop, "P", OMalloc<CProbePins>(cBoard.mpcFamily, 1));
	pcProbePins = pcProbe->GetProbePins();
	cBoard.Power(pcProbe);
	AssertTrue(pcProbePins->GetInput(0)->ConnectLive(pcOut, cBoard.mpcTimeline));
	AssertSize(1, cBoard.mpcTimeline->GetFirstSimultaneousEvents()->NumEvents());
	cBoard.Run();
	AssertLong(0, cBoard.Read(pcProbe));
	AssertInt(TV_Low, pcProbePins->GetInput(0)->Read(cBoard.mpcTimeline).eValue);

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The other gates from NANDs only, each against its truth table.
//
//////////////////////////////////////////////////////////////////////////
void TestStandardNandLogic(void)
{
	CTestBoard	cBoard;
	size		uiRow;
	bool		bA;
	bool		bB;
	CTrace*		pcA;
	CTrace*		pcB;
	CTrace*		apcOut[7];
	CTrace*		pcX;
	CTrace*		pcNotA;
	CTrace*		pcNotB;
	CProbe*		pcProbe;
	uint64		uiOut;

	for (uiRow = 0; uiRow < 4; uiRow++)
	{
		bA = (uiRow & 1) != 0;
		bB = (uiRow & 2) != 0;
		cBoard.Init();
		pcA = cBoard.Input("A", bA);
		pcB = cBoard.Input("B", bB);

		//NOT: Q = A NAND A.
		apcOut[0] = cBoard.Gate(G_Nand, pcA, pcA);

		//AND: Q = (A NAND B) NAND (A NAND B).
		pcX = cBoard.Gate(G_Nand, pcA, pcB);
		apcOut[1] = cBoard.Gate(G_Nand, pcX, pcX);

		//OR: Q = (A NAND A) NAND (B NAND B).
		pcNotA = cBoard.Gate(G_Nand, pcA, pcA);
		pcNotB = cBoard.Gate(G_Nand, pcB, pcB);
		apcOut[2] = cBoard.Gate(G_Nand, pcNotA, pcNotB);

		//NOR: the OR, inverted.
		apcOut[3] = cBoard.Gate(G_Nand, apcOut[2], apcOut[2]);

		//XOR, four gates: X = A NAND B, Q = (A NAND X) NAND (B NAND X).
		apcOut[4] = cBoard.Gate(G_Nand, cBoard.Gate(G_Nand, pcA, pcX), cBoard.Gate(G_Nand, pcB, pcX));

		//XNOR, five gates: Q = OR(A, B) NAND (A NAND B).
		apcOut[5] = cBoard.Gate(G_Nand, apcOut[2], cBoard.Gate(G_Nand, pcA, pcB));

		//XOR, five gates, disjunctive form: Q = (B NAND NOT A) NAND (A NAND NOT B).
		apcOut[6] = cBoard.Gate(G_Nand, cBoard.Gate(G_Nand, pcB, pcNotA), cBoard.Gate(G_Nand, pcA, pcNotB));

		pcProbe = cBoard.Probe(apcOut, 7);
		cBoard.Start();
		cBoard.Run();
		uiOut = cBoard.Read(pcProbe);

		AssertLong(!bA, (uiOut >> 0) & 1);
		AssertLong(bA && bB, (uiOut >> 1) & 1);
		AssertLong(bA || bB, (uiOut >> 2) & 1);
		AssertLong(!(bA || bB), (uiOut >> 3) & 1);
		AssertLong(bA != bB, (uiOut >> 4) & 1);
		AssertLong(bA == bB, (uiOut >> 5) & 1);
		AssertLong(bA != bB, (uiOut >> 6) & 1);
		cBoard.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Q = (A NAND NOT S) NAND (B NAND S): A when S is low, B when it is high.
//
//////////////////////////////////////////////////////////////////////////
void TestStandardNandMultiplexer(void)
{
	CTestBoard	cBoard;
	size		uiRow;
	CTrace*		apcIn[3];
	CTrace*		pcOut;
	CProbe*		pcProbe;
	bool		bA;
	bool		bB;
	bool		bS;

	for (uiRow = 0; uiRow < 8; uiRow++)
	{
		bA = (uiRow & 1) != 0;
		bB = (uiRow & 2) != 0;
		bS = (uiRow & 4) != 0;
		cBoard.Init();
		cBoard.Input("ABS", uiRow, 3, apcIn);
		pcOut = cBoard.Gate(G_Nand,
							cBoard.Gate(G_Nand, apcIn[0], cBoard.Gate(G_Nand, apcIn[2], apcIn[2])),
							cBoard.Gate(G_Nand, apcIn[1], apcIn[2]));
		pcProbe = cBoard.Probe(pcOut);
		cBoard.Start();
		cBoard.Run();
		AssertLong(bS ? bB : bA, cBoard.Read(pcProbe));
		cBoard.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Half adder: S = A XOR B, C = A AND B.
//
//////////////////////////////////////////////////////////////////////////
void TestStandardHalfAdder(void)
{
	CTestBoard	cBoard;
	size		uiRow;
	CTrace*		apcIn[2];
	CTrace*		apcOut[2];
	CProbe*		pcProbe;

	for (uiRow = 0; uiRow < 4; uiRow++)
	{
		cBoard.Init();
		cBoard.Input("AB", uiRow, 2, apcIn);
		apcOut[0] = cBoard.Gate(G_Xor, apcIn[0], apcIn[1]);
		apcOut[1] = cBoard.Gate(G_And, apcIn[0], apcIn[1]);
		pcProbe = cBoard.Probe(apcOut, 2);
		cBoard.Start();
		cBoard.Run();
		AssertLong((uiRow & 1) + (uiRow >> 1), cBoard.Read(pcProbe));
		cBoard.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Full adder: X1 = A XOR B, S = X1 XOR Cin, Cout = (A AND B) OR (Cin AND
// X1).  Returns Cout, and S in ppcSum.
//
//////////////////////////////////////////////////////////////////////////
CTrace* FullAdder(CTestBoard* pcBoard, CTrace* pcA, CTrace* pcB, CTrace* pcCarryIn, CTrace** ppcSum)
{
	CTrace*	pcX1;

	pcX1 = pcBoard->Gate(G_Xor, pcA, pcB);
	*ppcSum = pcBoard->Gate(G_Xor, pcX1, pcCarryIn);
	return pcBoard->Gate(G_Or, pcBoard->Gate(G_And, pcA, pcB), pcBoard->Gate(G_And, pcCarryIn, pcX1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStandardFullAdder(void)
{
	CTestBoard	cBoard;
	size		uiRow;
	CTrace*		apcIn[3];
	CTrace*		apcOut[2];
	CProbe*		pcProbe;

	for (uiRow = 0; uiRow < 8; uiRow++)
	{
		cBoard.Init();
		cBoard.Input("ABC", uiRow, 3, apcIn);
		apcOut[1] = FullAdder(&cBoard, apcIn[0], apcIn[1], apcIn[2], &apcOut[0]);
		pcProbe = cBoard.Probe(apcOut, 2);
		cBoard.Start();
		cBoard.Run();
		AssertLong((uiRow & 1) + ((uiRow >> 1) & 1) + (uiRow >> 2), cBoard.Read(pcProbe));
		cBoard.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Four full adders, each stage's carry out the next's carry in, the
// first with a carry in of 0.  Every pair of 4 bit numbers.
//
//////////////////////////////////////////////////////////////////////////
void TestStandardRippleCarryAdder(void)
{
	CTestBoard	cBoard;
	size		uiA;
	size		uiB;
	CTrace*		apcA[4];
	CTrace*		apcB[4];
	CTrace*		apcSum[5];
	CTrace*		pcCarry;
	CProbe*		pcProbe;
	size		i;
	size		uiFailures;

	uiFailures = 0;
	for (uiA = 0; uiA < 16; uiA++)
	{
		for (uiB = 0; uiB < 16; uiB++)
		{
			cBoard.Init();
			cBoard.Input("A", uiA, 4, apcA);
			cBoard.Input("B", uiB, 4, apcB);
			pcCarry = cBoard.Input("Cin", false);
			for (i = 0; i < 4; i++)
			{
				pcCarry = FullAdder(&cBoard, apcA[i], apcB[i], pcCarry, &apcSum[i]);
			}
			apcSum[4] = pcCarry;
			pcProbe = cBoard.Probe(apcSum, 5);
			cBoard.Start();
			cBoard.Run();
			if (cBoard.Read(pcProbe) != uiA + uiB)
			{
				uiFailures++;
			}
			cBoard.Kill();
		}
	}
	AssertSize(0, uiFailures);
}


//////////////////////////////////////////////////////////////////////////
//
// Two cross coupled NOR gates; S goes to the set NOR, R to the reset NOR,
// whose output is Q.
//
//////////////////////////////////////////////////////////////////////////
void TestStandardSRNorLatch(void)
{
	CTestBoard	cBoard;
	CConstant*	pcS;
	CConstant*	pcR;
	CTrace*		pcSTrace;
	CTrace*		pcRTrace;
	CTrace*		pcQ;
	CTrace*		pcQB;

	cBoard.Init();
	pcS = cBoard.Input("S", 0, 1, &pcSTrace);
	pcR = cBoard.Input("R", 1, 1, &pcRTrace);
	pcQ = cBoard.NewTrace();
	pcQB = cBoard.NewTrace();
	cBoard.GateTo(G_Nor, pcRTrace, pcQB, pcQ);
	cBoard.GateTo(G_Nor, pcSTrace, pcQ, pcQB);
	cBoard.Start();

	//Reset.
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcQ));
	AssertInt(TV_High, cBoard.Value(pcQB));

	//Hold.
	cBoard.Write(pcR, false);
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcQ));
	AssertInt(TV_High, cBoard.Value(pcQB));

	//Set, then hold.
	cBoard.Write(pcS, true);
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcQ));
	AssertInt(TV_Low, cBoard.Value(pcQB));
	cBoard.Write(pcS, false);
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcQ));
	AssertInt(TV_Low, cBoard.Value(pcQB));

	//Not allowed: both outputs low.
	cBoard.Write(pcS, true);
	cBoard.Write(pcR, true);
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcQ));
	AssertInt(TV_Low, cBoard.Value(pcQB));

	//Reset again.
	cBoard.Write(pcS, false);
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcQ));
	AssertInt(TV_High, cBoard.Value(pcQB));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Two cross coupled NAND gates with active low S and R.
//
//////////////////////////////////////////////////////////////////////////
void TestStandardSRNandLatch(void)
{
	CTestBoard	cBoard;
	CConstant*	pcS;
	CConstant*	pcR;
	CTrace*		pcSTrace;
	CTrace*		pcRTrace;
	CTrace*		pcQ;
	CTrace*		pcQB;

	cBoard.Init();
	pcS = cBoard.Input("S", 0, 1, &pcSTrace);
	pcR = cBoard.Input("R", 1, 1, &pcRTrace);
	pcQ = cBoard.NewTrace();
	pcQB = cBoard.NewTrace();
	cBoard.GateTo(G_Nand, pcSTrace, pcQB, pcQ);
	cBoard.GateTo(G_Nand, pcRTrace, pcQ, pcQB);
	cBoard.Start();

	//Set.
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcQ));
	AssertInt(TV_Low, cBoard.Value(pcQB));

	//No change.
	cBoard.Write(pcS, true);
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcQ));
	AssertInt(TV_Low, cBoard.Value(pcQB));

	//Reset, then no change.
	cBoard.Write(pcR, false);
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcQ));
	AssertInt(TV_High, cBoard.Value(pcQB));
	cBoard.Write(pcR, true);
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcQ));
	AssertInt(TV_High, cBoard.Value(pcQB));

	//Not allowed: both outputs high.
	cBoard.Write(pcS, false);
	cBoard.Write(pcR, false);
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcQ));
	AssertInt(TV_High, cBoard.Value(pcQB));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A constant drives its bits, the least significant on Output 0.
//
//////////////////////////////////////////////////////////////////////////
void TestStandardConstantAndProbe(void)
{
	CTestBoard	cBoard;
	CTrace*		apcBits[8];
	CProbe*		pcProbe;
	CConstant*	pcConstant;

	cBoard.Init();
	pcConstant = cBoard.Input("K", 0xA5, 8, apcBits);
	pcProbe = cBoard.Probe(apcBits, 8);
	AssertString(CONSTANT_TYPE, pcConstant->GetType());
	AssertString(PROBE_TYPE, pcProbe->GetType());
	AssertLong(0, cBoard.Read(pcProbe));
	cBoard.Start();
	AssertLong(0xA5, pcConstant->GetConstantValue());
	cBoard.Run();
	AssertLong(0xA5, cBoard.Read(pcProbe));
	AssertString("Output 7", pcConstant->GetConstantPins()->GetOutput(7)->GetName());
	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 10MHz clock: half cycles of 50ns, the first tick (high) after half a
// cycle.
//
//////////////////////////////////////////////////////////////////////////
CClockOscillator* AddClock(CTestBoard* pcBoard, CTrace** ppcOut, CTrace** ppcOutInverse)
{
	CClockOscillatorPins*	pcPins;
	CClockOscillator*		pcClock;

	pcClock = &OMalloc<CClockOscillator>(pcBoard->mpcTop, "Clock", OMalloc<CClockOscillatorPins>(pcBoard->mpcFamily, ppcOutInverse != NULL), 10 * FREQUENCY_MHZ);
	pcPins = pcClock->GetClockOscillatorPins();
	pcBoard->Power(pcClock);
	*ppcOut = pcBoard->NewTrace();
	pcPins->GetOutput()->Connect(*ppcOut);
	if (ppcOutInverse)
	{
		*ppcOutInverse = pcBoard->NewTrace();
		pcPins->GetOutputInverse()->Connect(*ppcOutInverse);
	}
	return pcClock;
}


#define CLOCK_HALF_CYCLE	NanosecondsToTime((int64)50)


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStandardClock(void)
{
	CTestBoard			cBoard;
	CClockOscillator*	pcClock;
	CTrace*				pcOut;
	CTrace*				pcOutInverse;

	cBoard.Init();
	pcClock = AddClock(&cBoard, &pcOut, &pcOutInverse);
	AssertLong(CLOCK_HALF_CYCLE, pcClock->GetHalfCycleTime());
	AssertString(CLOCK_OSCILLATOR_TYPE, pcClock->GetType());

	//Low until its first tick.
	AssertFloat(0.2f, pcClock->GetInternalVoltage(0), 3);

	cBoard.Start();
	AssertFloat(0.2f, pcClock->GetInternalVoltage(0), 3);

	//Just after the first tick's slew: high.
	cBoard.mpcTimeline->RunToTime(CLOCK_HALF_CYCLE + NanosecondsToTime((int64)10));
	AssertInt(TV_High, cBoard.Value(pcOut));
	AssertInt(TV_Low, cBoard.Value(pcOutInverse));
	AssertLong(1, pcClock->GetFullTicks());
	AssertFloat(3.1f, pcClock->GetInternalVoltage(0), 3);

	cBoard.mpcTimeline->RunToTime(CLOCK_HALF_CYCLE);
	AssertInt(TV_Low, cBoard.Value(pcOut));
	AssertInt(TV_High, cBoard.Value(pcOutInverse));

	//Ten full cycles on.
	cBoard.mpcTimeline->RunToTime(20 * CLOCK_HALF_CYCLE);
	AssertLong(11, pcClock->GetFullTicks());
	AssertInt(TV_Low, cBoard.Value(pcOut));
	cBoard.Kill();

	//Without power it doesn't tick.
	cBoard.Init();
	{
		CClockOscillatorPins*	pcPins;

		pcClock = &OMalloc<CClockOscillator>(cBoard.mpcTop, "Clock", OMalloc<CClockOscillatorPins>(cBoard.mpcFamily, false), 10 * FREQUENCY_MHZ);
		pcPins = pcClock->GetClockOscillatorPins();
		AssertNull(pcPins->GetOutputInverse());
	}
	cBoard.Start();
	cBoard.mpcTimeline->RunToTime(10 * CLOCK_HALF_CYCLE);
	AssertLong(0, pcClock->GetFullTicks());
	AssertSize(0, cBoard.mpcTimeline->NumSimultaneousEvents());

	//Wiring its power starts it.
	pcClock->GetPins()->GetVoltageGround()->Connect(cBoard.mpcGND);
	AssertTrue(pcClock->GetPins()->GetVoltageCommon()->ConnectLive(cBoard.mpcVCC, cBoard.mpcTimeline));
	AssertSize(1, pcClock->NumEvents());
	cBoard.mpcTimeline->RunToTime(4 * CLOCK_HALF_CYCLE);
	AssertLong(2, pcClock->GetFullTicks());

	//Unwiring its power stops it ticking and driving.
	pcOut = cBoard.NewTrace();
	(pcClock->GetClockOscillatorPins())->GetOutput()->ConnectLive(pcOut, cBoard.mpcTimeline);
	cBoard.mpcTimeline->RunToTime(CLOCK_HALF_CYCLE / 2);
	AssertTrue(IsVoltage(pcOut->GetVoltage(cBoard.mpcTimeline->GetTime())));
	pcClock->GetPins()->GetVoltageCommon()->Disconnect(cBoard.mpcTimeline);
	cBoard.mpcTimeline->RunToTime(10 * CLOCK_HALF_CYCLE);
	AssertLong(2, pcClock->GetFullTicks());
	AssertInt(TV_Undriven, cBoard.Value(pcOut));
	AssertSize(0, cBoard.mpcTimeline->NumSimultaneousEvents());
	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A D flip-flop's pins on the board, with set and reset on traces.
//
//////////////////////////////////////////////////////////////////////////
CDTypeFlipFlopPins* AddFlipFlop(CTestBoard* pcBoard, CTrace* pcClock, CTrace* pcSet, CTrace* pcReset, CDTypeFlipFlop** ppcFlipFlop)
{
	CDTypeFlipFlopPins*	pcPins;
	CDTypeFlipFlop*		pcFlipFlop;

	pcFlipFlop = &OMalloc<CDTypeFlipFlop>(pcBoard->mpcTop, "", OMalloc<CDTypeFlipFlopPins>(pcBoard->mpcFamily));
	pcPins = pcFlipFlop->GetDTypeFlipFlopPins();
	pcBoard->Power(pcFlipFlop);
	pcPins->GetClock()->Connect(pcClock);
	pcPins->GetSet()->Connect(pcSet);
	pcPins->GetReset()->Connect(pcReset);
	if (ppcFlipFlop)
	{
		*ppcFlipFlop = pcFlipFlop;
	}
	return pcPins;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStandardDFlipFlop(void)
{
	CTestBoard			cBoard;
	CConstant*			pcClock;
	CConstant*			pcData;
	CConstant*			pcSet;
	CConstant*			pcReset;
	CTrace*				pcClockTrace;
	CTrace*				pcDataTrace;
	CTrace*				pcSetTrace;
	CTrace*				pcResetTrace;
	CTrace*				pcQ;
	CTrace*				pcQB;
	CDTypeFlipFlopPins*	pcPins;
	CDTypeFlipFlop*		pcFlipFlop;

	cBoard.Init();
	pcClock = cBoard.Input("CP", 0, 1, &pcClockTrace);
	pcData = cBoard.Input("D", 1, 1, &pcDataTrace);
	pcSet = cBoard.Input("SD", 1, 1, &pcSetTrace);
	pcReset = cBoard.Input("RD", 1, 1, &pcResetTrace);
	pcPins = AddFlipFlop(&cBoard, pcClockTrace, pcSetTrace, pcResetTrace, &pcFlipFlop);
	pcPins->GetData()->Connect(pcDataTrace);
	pcQ = cBoard.NewTrace();
	pcQB = cBoard.NewTrace();
	pcPins->GetOutputQ()->Connect(pcQ);
	pcPins->GetOutputQB()->Connect(pcQB);
	AssertString(D_TYPE_FLIP_FLOP_TYPE, pcFlipFlop->GetType());
	cBoard.Start();
	cBoard.Run();

	//Q and QB are driven from the low state it starts in.
	AssertInt(TV_Low, cBoard.Value(pcQ));
	AssertInt(TV_High, cBoard.Value(pcQB));
	AssertFalse(pcFlipFlop->GetState());

	//Rising clock takes D.
	cBoard.Write(pcClock, true);
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcQ));
	AssertInt(TV_Low, cBoard.Value(pcQB));

	//D changing while the clock is high, or the clock falling, does nothing.
	cBoard.Write(pcData, false);
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcQ));
	cBoard.Write(pcClock, false);
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcQ));

	cBoard.Write(pcClock, true);
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcQ));
	AssertInt(TV_High, cBoard.Value(pcQB));

	//Active low set and reset don't wait for the clock.
	cBoard.Write(pcSet, false);
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcQ));
	cBoard.Write(pcSet, true);
	cBoard.Write(pcReset, false);
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcQ));

	//Reset holds it against the clock.
	cBoard.Write(pcData, true);
	cBoard.Write(pcClock, false);
	cBoard.Run();
	cBoard.Write(pcClock, true);
	cBoard.Run();
	AssertInt(TV_Low, cBoard.Value(pcQ));

	//Both together set it.
	cBoard.Write(pcSet, false);
	cBoard.Run();
	AssertInt(TV_High, cBoard.Value(pcQ));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Pulses the active low set and then reset of the flip-flops, which
// leaves them all low however they started.
//
//////////////////////////////////////////////////////////////////////////
void PowerOnReset(CTestBoard* pcBoard, CConstant* pcSet, CConstant* pcReset)
{
	pcBoard->Write(pcSet, false);
	pcBoard->mpcTimeline->RunToTime(NanosecondsToTime((int64)10));
	pcBoard->Write(pcSet, true);
	pcBoard->Write(pcReset, false);
	pcBoard->mpcTimeline->RunToTime(NanosecondsToTime((int64)10));
	pcBoard->Write(pcReset, true);
	pcBoard->mpcTimeline->RunToTime(NanosecondsToTime((int64)10));
}


//////////////////////////////////////////////////////////////////////////
//
// QB to D toggles the flip-flop on every rising clock, halving the
// clock; chained QB to the next clock it is a ripple up counter.
//
//////////////////////////////////////////////////////////////////////////
void TestStandardRippleCounter(void)
{
	CTestBoard			cBoard;
	CClockOscillator*	pcClock;
	CTrace*				pcClockTrace;
	CConstant*			pcSet;
	CConstant*			pcReset;
	CTrace*				pcSetTrace;
	CTrace*				pcResetTrace;
	CDTypeFlipFlopPins*	apcStages[3];
	CTrace*				apcQ[3];
	CTrace*				pcQB;
	CTrace*				pcStageClock;
	CProbe*				pcProbe;
	size				i;
	size				uiEdges;
	int64				iStart;
	size				uiFailures;

	cBoard.Init();
	pcClock = AddClock(&cBoard, &pcClockTrace, NULL);
	pcSet = cBoard.Input("SD", 1, 1, &pcSetTrace);
	pcReset = cBoard.Input("RD", 1, 1, &pcResetTrace);

	pcStageClock = pcClockTrace;
	for (i = 0; i < 3; i++)
	{
		apcStages[i] = AddFlipFlop(&cBoard, pcStageClock, pcSetTrace, pcResetTrace, NULL);
		apcQ[i] = cBoard.NewTrace();
		pcQB = cBoard.NewTrace();
		apcStages[i]->GetOutputQ()->Connect(apcQ[i]);
		apcStages[i]->GetOutputQB()->Connect(pcQB);
		apcStages[i]->GetData()->Connect(pcQB);
		pcStageClock = pcQB;
	}
	pcProbe = cBoard.Probe(apcQ, 3);

	cBoard.Start();
	PowerOnReset(&cBoard, pcSet, pcReset);
	AssertLong(0, cBoard.Read(pcProbe));
	iStart = cBoard.mpcTimeline->GetTime();

	//Read 40ns after each rising edge, which leaves the third stage time to ripple.
	uiFailures = 0;
	for (uiEdges = 1; uiEdges <= 20; uiEdges++)
	{
		cBoard.mpcTimeline->RunToTime(CLOCK_HALF_CYCLE * (2 * uiEdges - 1) + NanosecondsToTime((int64)40) - cBoard.mpcTimeline->GetTime());
		if (cBoard.Read(pcProbe) != (uiEdges % 8))
		{
			uiFailures++;
		}
	}
	AssertSize(0, uiFailures);
	AssertLong(20, pcClock->GetFullTicks());
	AssertTrue(iStart < CLOCK_HALF_CYCLE);

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 4 bit counter counting to 9, as a decade counter, clocked at 10MHz.
//
//////////////////////////////////////////////////////////////////////////
void TestStandardCounter(void)
{
	CTestBoard			cBoard;
	CTrace*				pcClockTrace;
	CCounterPins*		pcPins;
	CCounter*			pcCounter;
	CConstant*			pcReset;
	CConstant*			pcLoad;
	CConstant*			pcEnable;
	CTrace*				pcTrace;
	CTrace*				apcData[4];
	CTrace*				apcCount[5];
	CProbe*				pcProbe;
	size				i;
	size				uiEdges;
	size				uiFailures;
	CChars				sz;

	cBoard.Init();
	AddClock(&cBoard, &pcClockTrace, NULL);

	pcCounter = &OMalloc<CCounter>(cBoard.mpcTop, "U1", OMalloc<CCounterPins>(cBoard.mpcFamily, 4), 4, 9);
	pcPins = pcCounter->GetCounterPins();
	cBoard.Power(pcCounter);
	AssertString(COUNTER_TYPE, pcCounter->GetType());
	AssertString("Data 3", pcPins->GetData()->GetPtr(3)->GetName());
	AssertString("Terminal Count", pcPins->GetTerminalOutput()->GetName());

	pcPins->GetClock()->Connect(pcClockTrace);
	pcReset = cBoard.Input("Reset", 1, 1, &pcTrace);
	pcPins->GetReset()->Connect(pcTrace);
	pcLoad = cBoard.Input("Load", 1, 1, &pcTrace);
	pcPins->GetLoad()->Connect(pcTrace);
	pcEnable = cBoard.Input("Enable", 1, 1, &pcTrace);
	pcPins->GetCountEnable()->Connect(pcTrace);
	pcPins->GetCountEnableTerminal()->Connect(pcTrace);
	cBoard.Input("Data", 6, 4, apcData);
	for (i = 0; i < 4; i++)
	{
		pcPins->GetData()->GetPtr(i)->Connect(apcData[i]);
		apcCount[i] = cBoard.NewTrace();
		pcPins->GetOutput()->GetPtr(i)->Connect(apcCount[i]);
	}
	apcCount[4] = cBoard.NewTrace();
	pcPins->GetTerminalOutput()->Connect(apcCount[4]);
	pcProbe = cBoard.Probe(apcCount, 5);
	cBoard.Start();

	//Before the first rising clock the count and terminal count are driven from the zero it starts at.
	cBoard.mpcTimeline->RunToTime(NanosecondsToTime((int64)10));
	for (i = 0; i < 5; i++)
	{
		AssertInt(TV_Low, cBoard.Value(apcCount[i]));
	}

	//Counts 1 to 9 then wraps; terminal count (bit 4) is high at 9.
	uiFailures = 0;
	for (uiEdges = 1; uiEdges <= 25; uiEdges++)
	{
		cBoard.mpcTimeline->RunToTime(CLOCK_HALF_CYCLE * (2 * uiEdges - 1) + NanosecondsToTime((int64)20) - cBoard.mpcTimeline->GetTime());
		if (cBoard.Read(pcProbe) != ((uiEdges % 10) | (((uiEdges % 10) == 9) ? 0x10 : 0)))
		{
			uiFailures++;
		}
	}
	AssertSize(0, uiFailures);
	AssertInt(5, pcCounter->GetCount());

	sz.Init();
	pcCounter->AppendCounterValueHex(&sz);
	AssertString("0x05", sz.Text());
	sz.Kill();

	//Load takes the data, 6, at once.
	cBoard.Write(pcLoad, false);
	cBoard.mpcTimeline->RunToTime(NanosecondsToTime((int64)10));
	AssertLong(6, cBoard.Read(pcProbe));
	cBoard.Write(pcLoad, true);

	//Reset clears it at once.
	cBoard.Write(pcReset, false);
	cBoard.mpcTimeline->RunToTime(NanosecondsToTime((int64)10));
	AssertLong(0, cBoard.Read(pcProbe));
	cBoard.Write(pcReset, true);

	//Disabled it doesn't count; terminal count goes low with enable terminal.
	cBoard.Write(pcEnable, false);
	cBoard.mpcTimeline->RunToTime(10 * CLOCK_HALF_CYCLE);
	AssertLong(0, cBoard.Read(pcProbe));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStandardIntegratedCircuits(void)
{
	BeginTests();

	TestStandardGates();
	TestStandardGatesUndriven();
	TestStandardUndrivenSpreads();
	TestStandardRemoveComponent();
	TestStandardLiveWiring();
	TestStandardDescriptions();
	TestStandardNandLogic();
	TestStandardNandMultiplexer();
	TestStandardHalfAdder();
	TestStandardFullAdder();
	TestStandardRippleCarryAdder();
	TestStandardSRNorLatch();
	TestStandardSRNandLatch();
	TestStandardConstantAndProbe();
	TestStandardClock();
	TestStandardDFlipFlop();
	TestStandardRippleCounter();
	TestStandardCounter();

	TestStatistics();
}

