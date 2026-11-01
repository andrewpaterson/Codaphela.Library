#include "MicroprocessorLib/GateArray.h"
#include "MicroprocessorLib/GroupPins.h"
#include "MicroprocessorLib/Register.h"
#include "MicroprocessorLib/BusBuffer.h"
#include "MicroprocessorLib/BusTransceiver.h"
#include "MicroprocessorLib/Multiplexer.h"
#include "MicroprocessorLib/Decoder.h"
#include "MicroprocessorLib/Adder.h"
#include "MicroprocessorLib/Comparator.h"
#include "MicroprocessorLib/FlipFlopArray.h"
#include "TestCircuitBoard.h"
#include "TestLib/Assert.h"


#define GROUP_TEST_MAX_BITS		16


//////////////////////////////////////////////////////////////////////////
//
// A one bit input for each bit of the value, so each can be written on
// its own, with their traces.
//
//////////////////////////////////////////////////////////////////////////
struct SGroupTestBus
{
	CConstant*	apcBits[GROUP_TEST_MAX_BITS];
	CTrace*		apcTraces[GROUP_TEST_MAX_BITS];
	size		uiBits;

	void Init(CTestBoard* pcBoard, const char* szName, uint32 uiValue, size uiCount)
	{
		size	i;

		uiBits = uiCount;
		for (i = 0; i < uiBits; i++)
		{
			apcBits[i] = pcBoard->Input(szName, (uiValue >> i) & 1, 1, &apcTraces[i]);
		}
	}

	void Write(CTestBoard* pcBoard, uint32 uiValue)
	{
		size	i;

		for (i = 0; i < uiBits; i++)
		{
			pcBoard->Write(apcBits[i], ((uiValue >> i) & 1) != 0);
		}
	}

	void Connect(CGroupPins* pcPins, size uiRole, size uiGroup)
	{
		size	i;

		for (i = 0; i < uiBits; i++)
		{
			pcPins->GetPin(uiRole, uiGroup, i)->Connect(apcTraces[i]);
		}
	}
};


//////////////////////////////////////////////////////////////////////////
//
// New traces on the role's pins of the group, and a probe of them.
//
//////////////////////////////////////////////////////////////////////////
struct SGroupTestOutputs
{
	CTrace*		apcTraces[GROUP_TEST_MAX_BITS];
	size		uiBits;
	CProbe*		pcProbe;

	void Init(CTestBoard* pcBoard, CGroupPins* pcPins, size uiRole, size uiGroup)
	{
		size	i;

		uiBits = pcPins->GetBitCount(uiRole);
		for (i = 0; i < uiBits; i++)
		{
			apcTraces[i] = pcBoard->NewTrace();
			pcPins->GetPin(uiRole, uiGroup, i)->Connect(apcTraces[i]);
		}
		pcProbe = pcBoard->Probe(apcTraces, uiBits);
	}

	uint64 Read(CTestBoard* pcBoard)
	{
		return pcBoard->Read(pcProbe);
	}

	bool IsUndriven(CTestBoard* pcBoard)
	{
		size	i;

		for (i = 0; i < uiBits; i++)
		{
			if (pcBoard->Value(apcTraces[i]) != TV_Undriven)
			{
				return false;
			}
		}
		return true;
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SettleGroupTest(CTestBoard* pcBoard)
{
	pcBoard->mpcTimeline->RunToTime(NanosecondsToTime((int64)50));
}


//////////////////////////////////////////////////////////////////////////
//
// A quad 2-input NAND: each gate follows its own inputs, named "1A", "1B"
// and "1Y" up.
//
//////////////////////////////////////////////////////////////////////////
void TestGroupGateArray(void)
{
	CTestBoard			cBoard;
	CGateArray*			pcGates;
	CGateArrayPins*		pcPins;
	SGroupTestBus		sA;
	SGroupTestBus		sB;
	CTrace*				apcOut[4];
	CProbe*				pcProbe;
	size				i;

	cBoard.Init();
	pcGates = &OMalloc<CGateArray>(cBoard.mpcTop, "U1", OMalloc<CGateArrayPins>(4, 2, cBoard.mpcFamily), GF_Nand);
	pcPins = pcGates->GetGateArrayPins();
	cBoard.Power(pcGates);
	AssertString("1A", pcPins->GetInput(0, 0)->GetName());
	AssertString("4B", pcPins->GetInput(3, 1)->GetName());
	AssertString("2Y", pcPins->GetOutput(1)->GetName());
	AssertString(GATE_ARRAY_TYPE, pcGates->GetType());

	sA.Init(&cBoard, "A", 0x0, 4);
	sB.Init(&cBoard, "B", 0x0, 4);
	for (i = 0; i < 4; i++)
	{
		pcPins->GetInput(i, 0)->Connect(sA.apcTraces[i]);
		pcPins->GetInput(i, 1)->Connect(sB.apcTraces[i]);
		apcOut[i] = cBoard.NewTrace();
		pcPins->GetOutput(i)->Connect(apcOut[i]);
	}
	pcProbe = cBoard.Probe(apcOut, 4);
	cBoard.Start();
	SettleGroupTest(&cBoard);
	AssertLong(0xF, cBoard.Read(pcProbe));

	sA.Write(&cBoard, 0x5);
	sB.Write(&cBoard, 0x6);
	SettleGroupTest(&cBoard);
	AssertLong(0xB, cBoard.Read(pcProbe));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 74LVC574: the data is taken on a rising clock and driven while output
// enable is low.  A 74LVC573: the data is followed while latch enable is
// high and held when it goes low.
//
//////////////////////////////////////////////////////////////////////////
void TestGroupRegister(void)
{
	CTestBoard			cBoard;
	SGroupPinRole		asRoles[8];
	size				uiRoles;
	CRegister*			pcRegister;
	CRegister*			pcLatch;
	SGroupTestBus		sData;
	SGroupTestBus		sClock;
	SGroupTestBus		sEnable;
	SGroupTestBus		sLatchEnable;
	SGroupTestOutputs	sQ;
	SGroupTestOutputs	sLatchQ;

	cBoard.Init();
	uiRoles = CRegister::GetRoles(asRoles, 8, false);
	pcRegister = &OMalloc<CRegister>(cBoard.mpcTop, "U1", OMalloc<CGroupPins>(1, asRoles, uiRoles, cBoard.mpcFamily), false);
	cBoard.Power(pcRegister);
	AssertString("D7", pcRegister->GetGroupPins()->GetPin(RR_Data, 0, 7)->GetName());
	AssertString("CP", pcRegister->GetGroupPins()->GetPin(RR_Clock, 0)->GetName());
	AssertString("OE", pcRegister->GetGroupPins()->GetPin(RR_OutputEnable, 0)->GetName());
	AssertString(REGISTER_TYPE, pcRegister->GetType());

	uiRoles = CRegister::GetRoles(asRoles, 8, true);
	pcLatch = &OMalloc<CRegister>(cBoard.mpcTop, "U2", OMalloc<CGroupPins>(1, asRoles, uiRoles, cBoard.mpcFamily), true);
	cBoard.Power(pcLatch);
	AssertString("LE", pcLatch->GetGroupPins()->GetPin(RR_Clock, 0)->GetName());

	sData.Init(&cBoard, "D", 0x5A, 8);
	sClock.Init(&cBoard, "CP", 0, 1);
	sEnable.Init(&cBoard, "OE", 0, 1);
	sLatchEnable.Init(&cBoard, "LE", 0, 1);
	sData.Connect(pcRegister->GetGroupPins(), RR_Data, 0);
	sClock.Connect(pcRegister->GetGroupPins(), RR_Clock, 0);
	sEnable.Connect(pcRegister->GetGroupPins(), RR_OutputEnable, 0);
	sData.Connect(pcLatch->GetGroupPins(), RR_Data, 0);
	sLatchEnable.Connect(pcLatch->GetGroupPins(), RR_Clock, 0);
	sEnable.Connect(pcLatch->GetGroupPins(), RR_OutputEnable, 0);
	sQ.Init(&cBoard, pcRegister->GetGroupPins(), RR_Output, 0);
	sLatchQ.Init(&cBoard, pcLatch->GetGroupPins(), RR_Output, 0);
	cBoard.Start();
	SettleGroupTest(&cBoard);
	AssertLong(0, sQ.Read(&cBoard));
	AssertLong(0, sLatchQ.Read(&cBoard));

	sClock.Write(&cBoard, 1);
	SettleGroupTest(&cBoard);
	AssertLong(0x5A, sQ.Read(&cBoard));
	AssertLong(0, sLatchQ.Read(&cBoard));

	//The register holds while the clock stays high; the open latch follows.
	sLatchEnable.Write(&cBoard, 1);
	sData.Write(&cBoard, 0x81);
	SettleGroupTest(&cBoard);
	AssertLong(0x5A, sQ.Read(&cBoard));
	AssertLong(0x81, sLatchQ.Read(&cBoard));
	AssertInt(0x5A, pcRegister->GetValue(0));

	//Closed, the latch holds.
	sLatchEnable.Write(&cBoard, 0);
	SettleGroupTest(&cBoard);
	sData.Write(&cBoard, 0x33);
	SettleGroupTest(&cBoard);
	AssertLong(0x81, sLatchQ.Read(&cBoard));

	//Output enable high stops them driving; low again drives what's held.
	sEnable.Write(&cBoard, 1);
	SettleGroupTest(&cBoard);
	AssertTrue(sQ.IsUndriven(&cBoard));
	AssertTrue(sLatchQ.IsUndriven(&cBoard));
	sEnable.Write(&cBoard, 0);
	SettleGroupTest(&cBoard);
	AssertLong(0x5A, sQ.Read(&cBoard));
	AssertLong(0x81, sLatchQ.Read(&cBoard));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 74LVC244: two groups of four, each driven while its own enable is
// low.  A 74LVC541's group needs both its enables low.
//
//////////////////////////////////////////////////////////////////////////
void TestGroupBusBuffer(void)
{
	CTestBoard			cBoard;
	SGroupPinRole		asRoles[8];
	size				uiRoles;
	CBusBuffer*			pcBuffer;
	CBusBuffer*			pcBuffer541;
	SGroupTestBus		sA1;
	SGroupTestBus		sA2;
	SGroupTestBus		sEnables;
	SGroupTestOutputs	sY1;
	SGroupTestOutputs	sY2;
	SGroupTestOutputs	sY541;
	CGroupPins*			pcPins;

	cBoard.Init();
	uiRoles = CBusBuffer::GetRoles(asRoles, 4, 1);
	pcBuffer = &OMalloc<CBusBuffer>(cBoard.mpcTop, "U1", OMalloc<CGroupPins>(2, asRoles, uiRoles, cBoard.mpcFamily), false);
	cBoard.Power(pcBuffer);
	pcPins = pcBuffer->GetGroupPins();
	AssertString("1A0", pcPins->GetPin(0, 0, 0)->GetName());
	AssertString("2Y3", pcPins->GetPin(1, 1, 3)->GetName());
	AssertString("2OE", pcPins->GetPin(2, 1)->GetName());

	uiRoles = CBusBuffer::GetRoles(asRoles, 4, 2);
	pcBuffer541 = &OMalloc<CBusBuffer>(cBoard.mpcTop, "U2", OMalloc<CGroupPins>(1, asRoles, uiRoles, cBoard.mpcFamily), true);
	cBoard.Power(pcBuffer541);
	AssertString("OE2", pcBuffer541->GetGroupPins()->GetPin(3, 0)->GetName());

	sA1.Init(&cBoard, "A1", 0x9, 4);
	sA2.Init(&cBoard, "A2", 0x6, 4);
	sEnables.Init(&cBoard, "OE", 0x1, 2);
	sA1.Connect(pcPins, 0, 0);
	sA2.Connect(pcPins, 0, 1);
	pcPins->GetPin(2, 0)->Connect(sEnables.apcTraces[0]);
	pcPins->GetPin(2, 1)->Connect(sEnables.apcTraces[1]);
	sA1.Connect(pcBuffer541->GetGroupPins(), 0, 0);
	pcBuffer541->GetGroupPins()->GetPin(2, 0)->Connect(sEnables.apcTraces[0]);
	pcBuffer541->GetGroupPins()->GetPin(3, 0)->Connect(sEnables.apcTraces[1]);
	sY1.Init(&cBoard, pcPins, 1, 0);
	sY2.Init(&cBoard, pcPins, 1, 1);
	sY541.Init(&cBoard, pcBuffer541->GetGroupPins(), 1, 0);
	cBoard.Start();
	SettleGroupTest(&cBoard);
	AssertTrue(sY1.IsUndriven(&cBoard));
	AssertLong(0x6, sY2.Read(&cBoard));
	AssertTrue(sY541.IsUndriven(&cBoard));

	sEnables.Write(&cBoard, 0x0);
	SettleGroupTest(&cBoard);
	AssertLong(0x9, sY1.Read(&cBoard));
	AssertLong(0x6, sY541.Read(&cBoard));

	sA1.Write(&cBoard, 0x3);
	SettleGroupTest(&cBoard);
	AssertLong(0x3, sY1.Read(&cBoard));
	AssertLong(0xC, sY541.Read(&cBoard));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 74LVC245: direction high drives B from A, low A from B; output enable
// high drives neither.
//
//////////////////////////////////////////////////////////////////////////
void TestGroupBusTransceiver(void)
{
	CTestBoard			cBoard;
	SGroupPinRole		asRoles[8];
	size				uiRoles;
	CBusTransceiver*	pcTransceiver;
	CBusBuffer*			pcDriverA;
	CBusBuffer*			pcDriverB;
	CGroupPins*			pcPins;
	SGroupTestBus		sValueA;
	SGroupTestBus		sValueB;
	SGroupTestBus		sControl;
	SGroupTestOutputs	sA;
	SGroupTestOutputs	sB;
	size				i;

	cBoard.Init();
	uiRoles = CBusTransceiver::GetRoles(asRoles, 4);
	pcTransceiver = &OMalloc<CBusTransceiver>(cBoard.mpcTop, "U1", OMalloc<CGroupPins>(1, asRoles, uiRoles, cBoard.mpcFamily));
	cBoard.Power(pcTransceiver);
	pcPins = pcTransceiver->GetGroupPins();
	AssertString("A3", pcPins->GetPin(BTR_A, 0, 3)->GetName());
	AssertString("B0", pcPins->GetPin(BTR_B, 0, 0)->GetName());
	AssertString("DIR", pcPins->GetPin(BTR_Direction, 0)->GetName());

	//Each side's bus has a three state driver of its own, enabled when the transceiver isn't driving that side.
	uiRoles = CBusBuffer::GetRoles(asRoles, 4, 1);
	pcDriverA = &OMalloc<CBusBuffer>(cBoard.mpcTop, "DA", OMalloc<CGroupPins>(1, asRoles, uiRoles, cBoard.mpcFamily), false);
	pcDriverB = &OMalloc<CBusBuffer>(cBoard.mpcTop, "DB", OMalloc<CGroupPins>(1, asRoles, uiRoles, cBoard.mpcFamily), false);
	cBoard.Power(pcDriverA);
	cBoard.Power(pcDriverB);
	sValueA.Init(&cBoard, "VA", 0xA, 4);
	sValueB.Init(&cBoard, "VB", 0x5, 4);
	sValueA.Connect(pcDriverA->GetGroupPins(), 0, 0);
	sValueB.Connect(pcDriverB->GetGroupPins(), 0, 0);

	//Control bit 0 is DIR, bit 1 OE, bit 2 the A driver's enable and bit 3 the B driver's.
	sControl.Init(&cBoard, "C", 0x9, 4);
	pcPins->GetPin(BTR_Direction, 0)->Connect(sControl.apcTraces[0]);
	pcPins->GetPin(BTR_OutputEnable, 0)->Connect(sControl.apcTraces[1]);
	pcDriverA->GetGroupPins()->GetPin(2, 0)->Connect(sControl.apcTraces[2]);
	pcDriverB->GetGroupPins()->GetPin(2, 0)->Connect(sControl.apcTraces[3]);
	sA.Init(&cBoard, pcPins, BTR_A, 0);
	sB.Init(&cBoard, pcPins, BTR_B, 0);
	for (i = 0; i < 4; i++)
	{
		pcDriverA->GetGroupPins()->GetPin(1, 0, i)->Connect(sA.apcTraces[i]);
		pcDriverB->GetGroupPins()->GetPin(1, 0, i)->Connect(sB.apcTraces[i]);
	}
	cBoard.Start();

	//A to B: A driven by its driver.
	SettleGroupTest(&cBoard);
	AssertLong(0xA, sA.Read(&cBoard));
	AssertLong(0xA, sB.Read(&cBoard));
	sValueA.Write(&cBoard, 0x3);
	SettleGroupTest(&cBoard);
	AssertLong(0x3, sB.Read(&cBoard));

	//B to A: B driven by its driver.
	sControl.Write(&cBoard, 0x4);
	SettleGroupTest(&cBoard);
	AssertLong(0x5, sB.Read(&cBoard));
	AssertLong(0x5, sA.Read(&cBoard));

	//Disabled, neither side is driven.
	sControl.Write(&cBoard, 0xE);
	SettleGroupTest(&cBoard);
	AssertTrue(sA.IsUndriven(&cBoard));
	AssertTrue(sB.IsUndriven(&cBoard));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 74LVC157 drives each channel's I0 or I1 as select says, low while
// disabled.  A 74LVC138 drives the addressed output low while enabled.
//
//////////////////////////////////////////////////////////////////////////
void TestGroupMultiplexerAndDecoder(void)
{
	CTestBoard			cBoard;
	SGroupPinRole		asRoles[8];
	size				uiRoles;
	CMultiplexer*		pcMultiplexer;
	CDecoder*			pcDecoder;
	CGroupPins*			pcPins;
	SGroupTestBus		sI0;
	SGroupTestBus		sI1;
	SGroupTestBus		sSelect;
	SGroupTestBus		sAddress;
	SGroupTestBus		sEnables;
	CTrace*				apcY[4];
	CProbe*				pcProbe;
	SGroupTestOutputs	sDecoded;
	size				i;

	cBoard.Init();
	uiRoles = CMultiplexer::GetRoles(asRoles, 1, false);
	pcMultiplexer = &OMalloc<CMultiplexer>(cBoard.mpcTop, "U1", OMalloc<CGroupPins>(4, asRoles, uiRoles, cBoard.mpcFamily), false);
	cBoard.Power(pcMultiplexer);
	pcPins = pcMultiplexer->GetGroupPins();
	AssertString("1I0", pcPins->GetPin(MR_Input, 0, 0)->GetName());
	AssertString("4I1", pcPins->GetPin(MR_Input, 3, 1)->GetName());
	AssertString("3Y", pcPins->GetPin(MR_Output, 2)->GetName());
	AssertString("S", pcPins->GetPin(MR_Select, 3)->GetName());
	AssertString("E", pcPins->GetPin(MR_Enable, 2)->GetName());

	sI0.Init(&cBoard, "I0", 0x3, 4);
	sI1.Init(&cBoard, "I1", 0xC, 4);
	sSelect.Init(&cBoard, "S", 0x2, 2);
	for (i = 0; i < 4; i++)
	{
		pcPins->GetPin(MR_Input, i, 0)->Connect(sI0.apcTraces[i]);
		pcPins->GetPin(MR_Input, i, 1)->Connect(sI1.apcTraces[i]);
		apcY[i] = cBoard.NewTrace();
		pcPins->GetPin(MR_Output, i)->Connect(apcY[i]);
	}
	pcPins->GetPin(MR_Select, 0)->Connect(sSelect.apcTraces[0]);
	pcPins->GetPin(MR_Enable, 0)->Connect(sSelect.apcTraces[1]);
	pcProbe = cBoard.Probe(apcY, 4);

	uiRoles = CDecoder::GetRoles(asRoles, 3, 3);
	pcDecoder = &OMalloc<CDecoder>(cBoard.mpcTop, "U2", OMalloc<CGroupPins>(1, asRoles, uiRoles, cBoard.mpcFamily), 2);
	cBoard.Power(pcDecoder);
	AssertString("E3", pcDecoder->GetGroupPins()->GetPin(DR_Enable + 2, 0)->GetName());
	AssertString("Y7", pcDecoder->GetGroupPins()->GetPin(DR_Output, 0, 7)->GetName());
	sAddress.Init(&cBoard, "A", 0x5, 3);
	sEnables.Init(&cBoard, "E", 0x4, 3);
	sAddress.Connect(pcDecoder->GetGroupPins(), DR_Address, 0);
	for (i = 0; i < 3; i++)
	{
		pcDecoder->GetGroupPins()->GetPin(DR_Enable + i, 0)->Connect(sEnables.apcTraces[i]);
	}
	sDecoded.Init(&cBoard, pcDecoder->GetGroupPins(), DR_Output, 0);
	cBoard.Start();

	//Disabled: low; the decoder enabled (E1 and E2 low, E3 high) drives Y5 low.
	SettleGroupTest(&cBoard);
	AssertLong(0x0, cBoard.Read(pcProbe));
	AssertLong(0xDF, sDecoded.Read(&cBoard));

	sSelect.Write(&cBoard, 0x0);
	SettleGroupTest(&cBoard);
	AssertLong(0x3, cBoard.Read(pcProbe));
	sSelect.Write(&cBoard, 0x1);
	SettleGroupTest(&cBoard);
	AssertLong(0xC, cBoard.Read(pcProbe));

	sAddress.Write(&cBoard, 0x0);
	SettleGroupTest(&cBoard);
	AssertLong(0xFE, sDecoded.Read(&cBoard));
	sEnables.Write(&cBoard, 0x5);
	SettleGroupTest(&cBoard);
	AssertLong(0xFF, sDecoded.Read(&cBoard));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 74LVC283 adds; a 74LVC521 drives P = Q low while they're equal.
//
//////////////////////////////////////////////////////////////////////////
void TestGroupAdderAndComparator(void)
{
	CTestBoard			cBoard;
	SGroupPinRole		asRoles[8];
	size				uiRoles;
	CAdder*				pcAdder;
	CComparator*		pcComparator;
	SGroupTestBus		sA;
	SGroupTestBus		sB;
	SGroupTestBus		sCarry;
	SGroupTestOutputs	sSum;
	SGroupTestOutputs	sCarryOut;
	SGroupTestBus		sEnable;
	SGroupTestOutputs	sEqual;

	cBoard.Init();
	uiRoles = CAdder::GetRoles(asRoles, 4);
	pcAdder = &OMalloc<CAdder>(cBoard.mpcTop, "U1", OMalloc<CGroupPins>(1, asRoles, uiRoles, cBoard.mpcFamily));
	cBoard.Power(pcAdder);
	AssertString("S3", pcAdder->GetGroupPins()->GetPin(AR_Sum, 0, 3)->GetName());
	AssertString("CO", pcAdder->GetGroupPins()->GetPin(AR_CarryOut, 0)->GetName());

	uiRoles = CComparator::GetRoles(asRoles, 4);
	pcComparator = &OMalloc<CComparator>(cBoard.mpcTop, "U2", OMalloc<CGroupPins>(1, asRoles, uiRoles, cBoard.mpcFamily));
	cBoard.Power(pcComparator);
	AssertString("EQ", pcComparator->GetGroupPins()->GetPin(CR_Equal, 0)->GetName());

	sA.Init(&cBoard, "A", 0x9, 4);
	sB.Init(&cBoard, "B", 0x8, 4);
	sCarry.Init(&cBoard, "C", 0x1, 1);
	sEnable.Init(&cBoard, "OE", 0x0, 1);
	sA.Connect(pcAdder->GetGroupPins(), AR_A, 0);
	sB.Connect(pcAdder->GetGroupPins(), AR_B, 0);
	sCarry.Connect(pcAdder->GetGroupPins(), AR_CarryIn, 0);
	sSum.Init(&cBoard, pcAdder->GetGroupPins(), AR_Sum, 0);
	sCarryOut.Init(&cBoard, pcAdder->GetGroupPins(), AR_CarryOut, 0);
	sA.Connect(pcComparator->GetGroupPins(), CR_P, 0);
	sB.Connect(pcComparator->GetGroupPins(), CR_Q, 0);
	sEnable.Connect(pcComparator->GetGroupPins(), CR_Enable, 0);
	sEqual.Init(&cBoard, pcComparator->GetGroupPins(), CR_Equal, 0);
	cBoard.Start();

	//9 + 8 + 1 is 18: 2 carry 1.
	SettleGroupTest(&cBoard);
	AssertLong(0x2, sSum.Read(&cBoard));
	AssertLong(0x1, sCarryOut.Read(&cBoard));
	AssertLong(0x1, sEqual.Read(&cBoard));

	sB.Write(&cBoard, 0x9);
	sCarry.Write(&cBoard, 0);
	SettleGroupTest(&cBoard);
	AssertLong(0x2, sSum.Read(&cBoard));
	AssertLong(0x1, sCarryOut.Read(&cBoard));
	AssertLong(0x0, sEqual.Read(&cBoard));

	sA.Write(&cBoard, 0x3);
	sB.Write(&cBoard, 0x4);
	SettleGroupTest(&cBoard);
	AssertLong(0x7, sSum.Read(&cBoard));
	AssertLong(0x0, sCarryOut.Read(&cBoard));

	sB.Write(&cBoard, 0x3);
	sEnable.Write(&cBoard, 1);
	SettleGroupTest(&cBoard);
	AssertLong(0x1, sEqual.Read(&cBoard));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 74LVC74's two flip flops are clocked, set and reset on their own.
//
//////////////////////////////////////////////////////////////////////////
void TestGroupFlipFlopArray(void)
{
	CTestBoard			cBoard;
	SGroupPinRole		asRoles[8];
	size				uiRoles;
	CFlipFlopArray*		pcFlipFlops;
	CGroupPins*			pcPins;
	SGroupTestBus		sClocks;
	SGroupTestBus		sData;
	SGroupTestBus		sSet;
	SGroupTestOutputs	sQ1;
	SGroupTestOutputs	sQ2;
	SGroupTestOutputs	sQB2;
	size				i;

	cBoard.Init();
	uiRoles = CFlipFlopArray::GetRoles(asRoles);
	pcFlipFlops = &OMalloc<CFlipFlopArray>(cBoard.mpcTop, "U1", OMalloc<CGroupPins>(2, asRoles, uiRoles, cBoard.mpcFamily));
	cBoard.Power(pcFlipFlops);
	pcPins = pcFlipFlops->GetGroupPins();
	AssertString("1CP", pcPins->GetPin(FFAR_Clock, 0)->GetName());
	AssertString("2QB", pcPins->GetPin(FFAR_QB, 1)->GetName());

	sClocks.Init(&cBoard, "CP", 0, 2);
	sData.Init(&cBoard, "D", 0x3, 2);
	sSet.Init(&cBoard, "SD", 0x3, 2);
	for (i = 0; i < 2; i++)
	{
		pcPins->GetPin(FFAR_Clock, i)->Connect(sClocks.apcTraces[i]);
		pcPins->GetPin(FFAR_Data, i)->Connect(sData.apcTraces[i]);
		pcPins->GetPin(FFAR_Set, i)->Connect(sSet.apcTraces[i]);
		pcPins->GetPin(FFAR_Reset, i)->Connect(sSet.apcTraces[1 - i]);
	}
	sQ1.Init(&cBoard, pcPins, FFAR_Q, 0);
	sQ2.Init(&cBoard, pcPins, FFAR_Q, 1);
	sQB2.Init(&cBoard, pcPins, FFAR_QB, 1);
	cBoard.Start();
	SettleGroupTest(&cBoard);
	AssertLong(0, sQ1.Read(&cBoard));
	AssertLong(1, sQB2.Read(&cBoard));

	//Only the first is clocked.
	sClocks.Write(&cBoard, 0x1);
	SettleGroupTest(&cBoard);
	AssertLong(1, sQ1.Read(&cBoard));
	AssertLong(0, sQ2.Read(&cBoard));

	//Set 1 low sets the first and resets the second.
	sSet.Write(&cBoard, 0x2);
	SettleGroupTest(&cBoard);
	AssertLong(1, sQ1.Read(&cBoard));
	AssertLong(0, sQ2.Read(&cBoard));
	AssertTrue(pcFlipFlops->GetState(0));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 74LVC161 loads on a rising clock, not at once.
//
//////////////////////////////////////////////////////////////////////////
void TestGroupSynchronousCounter(void)
{
	CTestBoard			cBoard;
	CCounter*			pcCounter;
	CCounterPins*		pcPins;
	SGroupTestBus		sClock;
	SGroupTestBus		sControls;
	SGroupTestBus		sData;
	CTrace*				apcCount[4];
	CProbe*				pcProbe;
	size				i;

	cBoard.Init();
	pcCounter = &OMalloc<CCounter>(cBoard.mpcTop, "U1", OMalloc<CCounterPins>(cBoard.mpcFamily, 4), 4, 15, true);
	pcPins = pcCounter->GetCounterPins();
	cBoard.Power(pcCounter);
	AssertTrue(pcCounter->IsSynchronousLoad());

	//Control bit 0 is reset, bit 1 load and bit 2 both enables.
	sClock.Init(&cBoard, "CP", 0, 1);
	sControls.Init(&cBoard, "C", 0x1, 3);
	sData.Init(&cBoard, "D", 0xB, 4);
	pcPins->GetClock()->Connect(sClock.apcTraces[0]);
	pcPins->GetReset()->Connect(sControls.apcTraces[0]);
	pcPins->GetLoad()->Connect(sControls.apcTraces[1]);
	pcPins->GetCountEnable()->Connect(sControls.apcTraces[2]);
	pcPins->GetCountEnableTerminal()->Connect(sControls.apcTraces[2]);
	for (i = 0; i < 4; i++)
	{
		pcPins->GetData()->GetPtr(i)->Connect(sData.apcTraces[i]);
		apcCount[i] = cBoard.NewTrace();
		pcPins->GetOutput()->GetPtr(i)->Connect(apcCount[i]);
	}
	pcProbe = cBoard.Probe(apcCount, 4);
	cBoard.Start();
	SettleGroupTest(&cBoard);

	//Load low doesn't load until the clock rises.
	AssertLong(0, cBoard.Read(pcProbe));
	sClock.Write(&cBoard, 1);
	SettleGroupTest(&cBoard);
	AssertLong(0xB, cBoard.Read(pcProbe));

	//Then it counts.
	sControls.Write(&cBoard, 0x7);
	sClock.Write(&cBoard, 0);
	SettleGroupTest(&cBoard);
	sClock.Write(&cBoard, 1);
	SettleGroupTest(&cBoard);
	AssertLong(0xC, cBoard.Read(pcProbe));

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGroupIntegratedCircuits(void)
{
	BeginTests();

	TestGroupGateArray();
	TestGroupRegister();
	TestGroupBusBuffer();
	TestGroupBusTransceiver();
	TestGroupMultiplexerAndDecoder();
	TestGroupAdderAndComparator();
	TestGroupFlipFlopArray();
	TestGroupSynchronousCounter();

	TestStatistics();
}

