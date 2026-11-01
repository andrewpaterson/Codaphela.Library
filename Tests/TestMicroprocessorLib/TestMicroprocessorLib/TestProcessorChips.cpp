#include <string.h>
#include "MicroprocessorLib/W65C816Chip.h"
#include "MicroprocessorLib/65C64Chip.h"
#include "MicroprocessorLib/Oscilloscope.h"
#include "MicroprocessorLib/W65C816Meta.h"
#include "MicroprocessorLib/65C64Meta.h"
#include "MicroprocessorLib/W65C816InstructionFactory.h"
#include "MicroprocessorLib/65C64InstructionFactory.h"
#include "TestCircuitBoard.h"
#include "TestW65C816Context.h"
#include "Test65C64Context.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// What a test RAM reads and writes.
//
//////////////////////////////////////////////////////////////////////////
class CTestRamMemory
{
public:
	virtual uint8	GetByte(uint32 uiAddress) =0;
	virtual void	SetByte(uint32 uiAddress, uint8 uiValue) =0;
};


class CTestW65C816Memory : public CTestRamMemory
{
public:
	CTestW65C816Context*	mpcContext;

	uint8	GetByte(uint32 uiAddress) { return mpcContext->GetByte(uiAddress); }
	void	SetByte(uint32 uiAddress, uint8 uiValue) { mpcContext->SetByte(uiAddress, uiValue); }
};


class CTest65C64Memory : public CTestRamMemory
{
public:
	CTest65C64Context*	mpcContext;

	uint8	GetByte(uint32 uiAddress) { return mpcContext->GetByte(uiAddress); }
	void	SetByte(uint32 uiAddress, uint8 uiValue) { mpcContext->SetByte(uiAddress, uiValue); }
};


//////////////////////////////////////////////////////////////////////////
//
// The pins of a test RAM for either processor's bus.  The 65C64's has
// VPB and the byte lanes; the W65C816's takes the bank off the data bus.
//
//////////////////////////////////////////////////////////////////////////
class CTestRamPins : public CPins
{
CONSTRUCTABLE(CTestRamPins);
public:
	CLogicPortBus	mcAddress;
	CLogicPortBus	mcData;
	CLogicPort*		mpcPHI2;
	CLogicPort*		mpcRWB;
	CLogicPort*		mpcVDA;
	CLogicPort*		mpcVPA;
	CLogicPort*		mpcVPB;
	CLogicPort*		mpcLowerByte;
	CLogicPort*		mpcUpperByte;

	CLogicPort* AddPort(EPortType eType, const char* szName, size i, CVoltageConfigurationSource* pcSource)
	{
		CLogicPort*	pcPort;
		CChars		sz;

		sz.Init(szName);
		if (i != (size)-1)
		{
			sz.Append(' ');
			sz.Append(i);
		}
		pcPort = NewMalloc<CLogicPort>();
		pcPort->Init(eType, this, sz.Text(), pcSource);
		sz.Kill();
		return pcPort;
	}

	void Init(CVoltageConfigurationSource* pcSource, size uiAddressBits, size uiDataBits, bool b65C64)
	{
		size	i;

		PreInit();

		CPins::Init();
		mcAddress.Init();
		for (i = 0; i < uiAddressBits; i++)
		{
			mcAddress.Add(AddPort(PT_Input, "Address", i, pcSource));
		}
		mcData.Init();
		for (i = 0; i < uiDataBits; i++)
		{
			mcData.Add(AddPort(PT_Bidirectional, "Data", i, pcSource));
		}
		mpcPHI2 = AddPort(PT_Input, "PHI2", -1, pcSource);
		mpcRWB = AddPort(PT_Input, "RWB", -1, pcSource);
		mpcVDA = AddPort(PT_Input, "VDA", -1, pcSource);
		mpcVPA = AddPort(PT_Input, "VPA", -1, pcSource);
		mpcVPB = NULL;
		mpcLowerByte = NULL;
		mpcUpperByte = NULL;
		if (b65C64)
		{
			mpcVPB = AddPort(PT_Input, "VPB", -1, pcSource);
			mpcLowerByte = AddPort(PT_Input, "Lower Byte", -1, pcSource);
			mpcUpperByte = AddPort(PT_Input, "Upper Byte", -1, pcSource);
		}

		PostInit();
	}

	void Class(void) override
	{
		CPins::Class();

		U_Data(CLogicPortBus, mcAddress);
		U_Data(CLogicPortBus, mcData);
		U_Pointer(mpcPHI2);
		U_Pointer(mpcRWB);
		U_Pointer(mpcVDA);
		U_Pointer(mpcVPA);
		U_Pointer(mpcVPB);
		U_Pointer(mpcLowerByte);
		U_Pointer(mpcUpperByte);
	}

	void Free(void) override
	{
		mcAddress.Kill();
		mcData.Kill();
		CPins::Free();
	}
};


//////////////////////////////////////////////////////////////////////////
//
// Memory on the bus.  When PHI2 rises it takes the address (and the
// W65C816's bank off the data bus) and drives the data for a valid read.
// When PHI2 falls it stores the data for a valid write, and stops driving.
//
//////////////////////////////////////////////////////////////////////////
class CTestRam : public CIntegratedCircuit
{
CONSTRUCTABLE(CTestRam);
public:
	CTestRamMemory*	mpcMemory;
	bool			mb65C64;
	uint32			muiAddress;
	bool			mbValid;
	bool			mbWrite;
	bool			mbLower;
	bool			mbUpper;
	bool			mbDriving;
	size			muiReads;
	size			muiWrites;

	void Init(CSubcircuitSimulation* pcSimulation, Ptr<CTestRamPins> pPins, CTestRamMemory* pcMemory, bool b65C64)
	{
		PreInit();

		CIntegratedCircuit::Init(pcSimulation, "RAM", pPins);
		mpcMemory = pcMemory;
		mb65C64 = b65C64;
		mbValid = false;
		mbDriving = false;
		muiReads = 0;
		muiWrites = 0;

		PostInit();
	}

	void Class(void) override
	{
		CIntegratedCircuit::Class();

		U_Pointer(mpcMemory);
		U_Bool(mb65C64);
		U_UInt32(muiAddress);
		U_Bool(mbValid);
		U_Bool(mbWrite);
		U_Bool(mbLower);
		U_Bool(mbUpper);
		U_Bool(mbDriving);
		U_Size(muiReads);
		U_Size(muiWrites);
	}

	CTestRamPins* GetRamPins(void) { return (CTestRamPins*)&mpPins; }
	const char* GetType(void) { return "Test RAM"; }
	void SimulationStarted(CTimeline* pcTimeline) {}

	void InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
	{
		CTestRamPins*	pcPins;
		STraceValue		sClock;
		STraceValue		sRWB;
		uint32			uiData;

		pcPins = GetRamPins();
		if (pcPort != pcPins->mpcPHI2)
		{
			return;
		}

		sClock = pcPort->Read(pcTimeline);
		if (sClock.IsHigh())
		{
			muiAddress = pcPins->mcAddress.Read(pcTimeline);
			sRWB = pcPins->mpcRWB->Read(pcTimeline);
			mbWrite = sRWB.IsLow();
			if (mb65C64)
			{
				mbValid = pcPins->mpcVPA->Read(pcTimeline).IsHigh() || pcPins->mpcVDA->Read(pcTimeline).IsHigh() || pcPins->mpcVPB->Read(pcTimeline).IsLow();
				mbLower = pcPins->mpcLowerByte->Read(pcTimeline).IsHigh();
				mbUpper = pcPins->mpcUpperByte->Read(pcTimeline).IsHigh();
			}
			else
			{
				muiAddress |= (pcPins->mcData.Read(pcTimeline) & 0xFF) << 16;
				mbValid = pcPins->mpcVPA->Read(pcTimeline).IsHigh() || pcPins->mpcVDA->Read(pcTimeline).IsHigh();
			}
			mbValid = mbValid && (sRWB.IsHigh() || sRWB.IsLow());

			if (mbValid && !mbWrite)
			{
				uiData = mpcMemory->GetByte(muiAddress);
				if (mb65C64)
				{
					uiData |= (uint32)mpcMemory->GetByte(muiAddress + 1) << 8;
				}
				pcPins->mcData.Write(pcTimeline, uiData);
				mbDriving = true;
				muiReads++;
			}
		}
		else if (sClock.IsLow())
		{
			if (mbValid && mbWrite)
			{
				uiData = pcPins->mcData.Read(pcTimeline);
				if (!mb65C64)
				{
					mpcMemory->SetByte(muiAddress, (uint8)uiData);
				}
				else
				{
					if (mbLower)
					{
						mpcMemory->SetByte(muiAddress, (uint8)uiData);
					}
					if (mbUpper)
					{
						mpcMemory->SetByte(muiAddress + 1, (uint8)(uiData >> 8));
					}
				}
				muiWrites++;
			}
			if (mbDriving)
			{
				pcPins->mcData.WriteImpedance(pcTimeline);
				mbDriving = false;
			}
			mbValid = false;
		}
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A clock of fFrequency on a new trace.
//
//////////////////////////////////////////////////////////////////////////
CTrace* AddChipClock(CTestBoard* pcBoard, float32 fFrequency, CClockOscillator** ppcClock)
{
	CClockOscillatorPins*	pcPins;
	CClockOscillator*		pcClock;
	CTrace*					pcTrace;

	pcClock = &OMalloc<CClockOscillator>(pcBoard->mpcTop, "Clock", OMalloc<CClockOscillatorPins>(pcBoard->mpcFamily, false), fFrequency);
	pcPins = pcClock->GetClockOscillatorPins();
	pcBoard->Power(pcClock);
	pcTrace = pcBoard->NewTrace();
	pcPins->GetOutput()->Connect(pcTrace);
	if (ppcClock)
	{
		*ppcClock = pcClock;
	}
	return pcTrace;
}


//////////////////////////////////////////////////////////////////////////
//
// Connects each port of the two buses to a shared new trace.
//
//////////////////////////////////////////////////////////////////////////
void ConnectBuses(CTestBoard* pcBoard, CLogicPortBus* pcFirst, CLogicPortBus* pcSecond)
{
	size	i;
	CTrace*	pcTrace;

	for (i = 0; i < pcFirst->GetWidth(); i++)
	{
		pcTrace = pcBoard->NewTrace();
		pcFirst->GetPort(i)->Connect(pcTrace);
		pcSecond->GetPort(i)->Connect(pcTrace);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A trace shared by two ports.
//
//////////////////////////////////////////////////////////////////////////
void ConnectPorts(CTestBoard* pcBoard, CLogicPort* pcFirst, CLogicPort* pcSecond)
{
	CTrace*	pcTrace;

	pcTrace = pcBoard->NewTrace();
	pcFirst->Connect(pcTrace);
	if (pcSecond)
	{
		pcSecond->Connect(pcTrace);
	}
}


#define CHIP_CLOCK_FREQUENCY	(1 * FREQUENCY_MHZ)
#define CHIP_CLOCK_PERIOD		NanosecondsToTime((int64)1000)
#define CHIP_MAX_CYCLES			200


//////////////////////////////////////////////////////////////////////////
//
// LDA #$42, STA $0300, INC $0300, LDA $0300, CLC, ADC #$10, STA $0301,
// STP at $0200, in emulation mode.
//
//////////////////////////////////////////////////////////////////////////
void LoadW65C816Program(CTestW65C816Context* pcContext)
{
	pcContext->Init(0x10000, 0, 0x0200);
	pcContext->SetOpcd(0x0200, 0xA9);
	pcContext->SetByte(0x42);
	pcContext->SetOpcd(0x8D);
	pcContext->SetShort(0x0300);
	pcContext->SetOpcd(0xEE);
	pcContext->SetShort(0x0300);
	pcContext->SetOpcd(0xAD);
	pcContext->SetShort(0x0300);
	pcContext->SetOpcd(0x18);
	pcContext->SetOpcd(0x69);
	pcContext->SetByte(0x10);
	pcContext->SetOpcd(0x8D);
	pcContext->SetShort(0x0301);
	pcContext->SetOpcd(0xDB);
}


//////////////////////////////////////////////////////////////////////////
//
// The program on the ideal meta processor, then on the chip with the test
// RAM, at 1MHz on LVC traces.  Reset is held for three cycles.
//
//////////////////////////////////////////////////////////////////////////
void TestProcessorChipsW65C816(void)
{
	CTestW65C816Context	cMetaContext;
	CW65C816Meta		cMeta;
	CTestW65C816Context	cChipContext;
	CTestW65C816Memory	cMemory;
	CTestBoard			cBoard;
	CW65C816ChipPins*	pcPins;
	CW65C816Chip*		pcChip;
	CTestRamPins*		pcRamPins;
	CTestRam*			pcRam;
	CConstant*			pcReset;
	CTrace*				pcResetTrace;
	CTrace*				pcHigh;
	CTrace*				pcClock;
	size				uiCycles;

	CW65C816InstructionFactory::GetInstance()->Init();
	LoadW65C816Program(&cMetaContext);
	cMetaContext.mbEnablePrint = false;
	cMetaContext.muiMaxInstructions = 100;
	cMeta.Init(TestW65C81ContextTickHigh, TestW65C81ContextTickLow, &cMetaContext);
	cMetaContext.Run(&cMeta);
	AssertTrue(cMeta.GetMPU()->IsStopped());
	AssertIntHex(0x53, cMeta.GetMPU()->GetA() & 0xFF);
	AssertIntHex(0x43, cMetaContext.GetByte(0x0300));
	AssertIntHex(0x53, cMetaContext.GetByte(0x0301));

	LoadW65C816Program(&cChipContext);
	cMemory.mpcContext = &cChipContext;
	cBoard.Init();
	pcClock = AddChipClock(&cBoard, CHIP_CLOCK_FREQUENCY, NULL);
	pcReset = cBoard.Input("RESB", 0, 1, &pcResetTrace);
	pcHigh = cBoard.Input("High", true);

	pcChip = &OMalloc<CW65C816Chip>(cBoard.mpcTop, "U1", OMalloc<CW65C816ChipPins>(cBoard.mpcFamily));
	pcPins = pcChip->GetChipPins();
	cBoard.Power(pcChip);
	AssertString(W65C816_CHIP_TYPE, pcChip->GetType());
	AssertString("Address 15", pcPins->GetAddress()->GetPort(15)->GetName());

	pcRam = &OMalloc<CTestRam>(cBoard.mpcTop, OMalloc<CTestRamPins>(cBoard.mpcFamily, 16, 8, false), &cMemory, false);
	pcRamPins = pcRam->GetRamPins();
	cBoard.Power(pcRam);

	ConnectBuses(&cBoard, pcPins->GetAddress(), &pcRamPins->mcAddress);
	ConnectBuses(&cBoard, pcPins->GetData(), &pcRamPins->mcData);
	pcPins->GetPHI2()->Connect(pcClock);
	pcRamPins->mpcPHI2->Connect(pcClock);
	ConnectPorts(&cBoard, pcPins->GetRWB(), pcRamPins->mpcRWB);
	ConnectPorts(&cBoard, pcPins->GetVDA(), pcRamPins->mpcVDA);
	ConnectPorts(&cBoard, pcPins->GetVPA(), pcRamPins->mpcVPA);
	pcPins->GetRESB()->Connect(pcResetTrace);
	pcPins->GetBE()->Connect(pcHigh);
	pcPins->GetIRQB()->Connect(pcHigh);
	pcPins->GetNMIB()->Connect(pcHigh);
	pcPins->GetABORTB()->Connect(pcHigh);

	cBoard.Start();
	cBoard.mpcTimeline->RunToTime(3 * CHIP_CLOCK_PERIOD);

	cBoard.Write(pcReset, true);
	for (uiCycles = 0; uiCycles < CHIP_MAX_CYCLES; uiCycles++)
	{
		cBoard.mpcTimeline->RunToTime(CHIP_CLOCK_PERIOD);
		if (pcChip->GetMPU()->IsStopped())
		{
			break;
		}
	}

	AssertTrue(pcChip->GetMPU()->IsStopped());
	AssertIntHex(cMeta.GetMPU()->GetA(), pcChip->GetMPU()->GetA());
	AssertIntHex(cMetaContext.GetByte(0x0300), cChipContext.GetByte(0x0300));
	AssertIntHex(cMetaContext.GetByte(0x0301), cChipContext.GetByte(0x0301));
	AssertSize(3, pcRam->muiWrites);
	AssertTrue(pcRam->muiReads > 10);

	cBoard.Kill();
	cMeta.Kill();
	cMetaContext.Kill();
	cChipContext.Kill();
	CW65C816InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Loads a long from $2000, adds 1, stores it at $4001 (odd, so the byte
// lanes split it), adds 1 to the word at $4010 and stops.
//
//////////////////////////////////////////////////////////////////////////
void Load65C64Program(CTest65C64Context* pcContext)
{
	size	i;

	pcContext->Init(0x1000);
	for (i = 0; i < 9; i++)
	{
		pcContext->SetByte((uint32)(0x2000 + i), (uint8)(0x11 * (i + 1)));
	}
	pcContext->SetWord(0x4010, 0x1234);
	pcContext->Op(Op65Wth(R65_A, 4));
	pcContext->OpLong(Op65M(C65_CPY, false, R65_A, OPERAND_MODE_ABSOLUTE, R65_Zero), 0x2000);
	pcContext->Op(Op65RE(C65_ADD, R65_A, 1));
	pcContext->OpLong(Op65M(C65_CPY, true, R65_A, OPERAND_MODE_ABSOLUTE, R65_Zero), 0x4001);
	pcContext->OpLong(Op65ME(C65_ADD, 1, OPERAND_MODE_ABSOLUTE, R65_Zero), 0x4010);
	pcContext->Op(Op65Stp());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestProcessorChips65C64(void)
{
	CTest65C64Context*	pcMetaContext;
	C65C64Meta			cMeta;
	CTest65C64Context*	pcChipContext;
	CTest65C64Memory	cMemory;
	CTestBoard			cBoard;
	C65C64ChipPins*		pcPins;
	C65C64Chip*			pcChip;
	CTestRamPins*		pcRamPins;
	CTestRam*			pcRam;
	CConstant*			pcReset;
	CTrace*				pcResetTrace;
	CTrace*				pcHigh;
	CTrace*				pcClock;
	size				uiCycles;
	size				i;

	C65C64InstructionFactory::GetInstance()->Init();
	pcMetaContext = (CTest65C64Context*)malloc(sizeof(CTest65C64Context));
	Load65C64Program(pcMetaContext);
	cMeta.Init(Test65C64TickHigh, Test65C64TickLow, pcMetaContext);
	Test65C64Run(&cMeta, pcMetaContext, 100);
	AssertTrue(cMeta.GetMPU()->IsStopped());
	AssertLongLongIntHex(0x44332212LL, cMeta.GetMPU()->GetA());
	AssertIntHex(0x12, pcMetaContext->GetByte(0x4001));
	AssertIntHex(0x44, pcMetaContext->GetByte(0x4004));
	AssertIntHex(0x1235, pcMetaContext->GetWord(0x4010));

	pcChipContext = (CTest65C64Context*)malloc(sizeof(CTest65C64Context));
	Load65C64Program(pcChipContext);
	cMemory.mpcContext = pcChipContext;
	cBoard.Init();
	pcClock = AddChipClock(&cBoard, CHIP_CLOCK_FREQUENCY, NULL);
	pcReset = cBoard.Input("RESB", 0, 1, &pcResetTrace);
	pcHigh = cBoard.Input("High", true);

	pcChip = &OMalloc<C65C64Chip>(cBoard.mpcTop, "U1", OMalloc<C65C64ChipPins>(cBoard.mpcFamily));
	pcPins = pcChip->GetChipPins();
	cBoard.Power(pcChip);
	AssertString(C65C64_CHIP_TYPE, pcChip->GetType());
	AssertString("Address 31", pcPins->GetAddress()->GetPort(31)->GetName());

	pcRam = &OMalloc<CTestRam>(cBoard.mpcTop, OMalloc<CTestRamPins>(cBoard.mpcFamily, 32, 16, true), &cMemory, true);
	pcRamPins = pcRam->GetRamPins();
	cBoard.Power(pcRam);

	ConnectBuses(&cBoard, pcPins->GetAddress(), &pcRamPins->mcAddress);
	ConnectBuses(&cBoard, pcPins->GetData(), &pcRamPins->mcData);
	pcPins->GetPHI2()->Connect(pcClock);
	pcRamPins->mpcPHI2->Connect(pcClock);
	ConnectPorts(&cBoard, pcPins->GetRWB(), pcRamPins->mpcRWB);
	ConnectPorts(&cBoard, pcPins->GetVDA(), pcRamPins->mpcVDA);
	ConnectPorts(&cBoard, pcPins->GetVPA(), pcRamPins->mpcVPA);
	ConnectPorts(&cBoard, pcPins->GetVPB(), pcRamPins->mpcVPB);
	ConnectPorts(&cBoard, pcPins->GetLowerByte(), pcRamPins->mpcLowerByte);
	ConnectPorts(&cBoard, pcPins->GetUpperByte(), pcRamPins->mpcUpperByte);
	pcPins->GetRESB()->Connect(pcResetTrace);
	for (i = 0; i < NUM_65C64_EDGE_INTERRUPTS; i++)
	{
		pcPins->GetEIRQB(i)->Connect(pcHigh);
	}
	pcPins->GetLIRQB()->Connect(pcHigh);

	cBoard.Start();
	cBoard.mpcTimeline->RunToTime(3 * CHIP_CLOCK_PERIOD);

	cBoard.Write(pcReset, true);
	for (uiCycles = 0; uiCycles < CHIP_MAX_CYCLES; uiCycles++)
	{
		cBoard.mpcTimeline->RunToTime(CHIP_CLOCK_PERIOD);
		if (pcChip->GetMPU()->IsStopped())
		{
			break;
		}
	}

	AssertTrue(pcChip->GetMPU()->IsStopped());
	AssertLongLongIntHex(cMeta.GetMPU()->GetA(), pcChip->GetMPU()->GetA());
	for (i = 0; i < 0x20; i++)
	{
		AssertIntHex(pcMetaContext->GetByte(0x4000 + (uint32)i), pcChipContext->GetByte(0x4000 + (uint32)i));
	}
	AssertSize(0, pcChipContext->muiBadAccesses);

	cBoard.Kill();
	cMeta.Kill();
	free(pcMetaContext);
	free(pcChipContext);
	C65C64InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 10MHz clock sampled at 100MHz, 10 samples a cycle: high for five,
// low for five.  An unconnected input samples 0V.
//
//////////////////////////////////////////////////////////////////////////
void TestProcessorChipsOscilloscope(void)
{
	CTestBoard			cBoard;
	COscilloscopePins*	pcPins;
	COscilloscope*		pcScope;
	CTrace*				pcClock;
	size				i;
	size				uiHigh;
	size				uiLow;

	cBoard.Init();
	pcClock = AddChipClock(&cBoard, 10 * FREQUENCY_MHZ, NULL);
	pcScope = &OMalloc<COscilloscope>(cBoard.mpcTop, "Scope", OMalloc<COscilloscopePins>(2), 100 * FREQUENCY_MHZ, 10, 10);
	pcPins = pcScope->GetOscilloscopePins();
	pcPins->GetInput(0)->Connect(pcClock);
	AssertString(OSCILLOSCOPE_TYPE, pcScope->GetType());
	AssertString("Input 1", pcPins->GetInput(1)->GetName());
	AssertLong(NanosecondsToTime((int64)10), pcScope->GetSampleTime());

	cBoard.Start();
	AssertSize(2, pcScope->GetInputCount());
	AssertSize(100, pcScope->GetSampleCount());

	//A microsecond is 100 samples, round to the start of the ring.
	cBoard.mpcTimeline->RunToTime(NanosecondsToTime((int64)1000));
	AssertSize(0, pcScope->GetTickPosition());

	uiHigh = 0;
	uiLow = 0;
	for (i = 0; i < 100; i++)
	{
		if (pcScope->GetMaxVoltage(0, i) > 3.0f)
		{
			uiHigh++;
		}
		else if (pcScope->GetMaxVoltage(0, i) < 0.3f)
		{
			uiLow++;
		}
		AssertFloat(pcScope->GetMaxVoltage(0, i), pcScope->GetMinVoltage(0, i), 3);
		AssertFloat(0.0f, pcScope->GetMaxVoltage(1, i), 3);
	}
	AssertTrue(uiHigh >= 40);
	AssertTrue(uiLow >= 40);

	//The logic port with no voltage configuration doesn't read a level.
	AssertInt(TV_Undriven, pcPins->GetInput(0)->Read(cBoard.mpcTimeline).eValue);

	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim's Simulation: removing an event through the circuit simulation.
//
//////////////////////////////////////////////////////////////////////////
void TestProcessorChipsRemoveEvent(void)
{
	CTestBoard			cBoard;
	CClockOscillator*	pcClock;

	cBoard.Init();
	AddChipClock(&cBoard, 10 * FREQUENCY_MHZ, &pcClock);
	cBoard.Start();
	AssertSize(1, pcClock->NumEvents());
	AssertTrue(cBoard.mcSimulation.RemoveEvent(pcClock->GetEvent(0)));
	AssertSize(0, pcClock->NumEvents());
	AssertFalse(cBoard.mcSimulation.RunSimultaneous());
	AssertLong(0, cBoard.mcSimulation.GetTime());
	cBoard.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A W65C816 chip running LoadW65C816Program from a test RAM, reset
// released and ready to clock.
//
//////////////////////////////////////////////////////////////////////////
struct SW65C816Computer
{
	CTestW65C816Context	cContext;
	CTestW65C816Memory	cMemory;
	CTestBoard			cBoard;
	CW65C816Chip*		pcChip;
	CConstant*			pcReset;

	void Init(void)
	{
		CW65C816ChipPins*	pcPins;
		CTestRamPins*		pcRamPins;
		CTestRam*			pcRam;
		CTrace*				pcResetTrace;
		CTrace*				pcHigh;
		CTrace*				pcClock;

		LoadW65C816Program(&cContext);
		cMemory.mpcContext = &cContext;
		cBoard.Init();
		pcClock = AddChipClock(&cBoard, CHIP_CLOCK_FREQUENCY, NULL);
		pcReset = cBoard.Input("RESB", 0, 1, &pcResetTrace);
		pcHigh = cBoard.Input("High", true);
		pcChip = &OMalloc<CW65C816Chip>(cBoard.mpcTop, "U1", OMalloc<CW65C816ChipPins>(cBoard.mpcFamily));
		pcPins = pcChip->GetChipPins();
		cBoard.Power(pcChip);
		pcRam = &OMalloc<CTestRam>(cBoard.mpcTop, OMalloc<CTestRamPins>(cBoard.mpcFamily, 16, 8, false), &cMemory, false);
		pcRamPins = pcRam->GetRamPins();
		cBoard.Power(pcRam);
		ConnectBuses(&cBoard, pcPins->GetAddress(), &pcRamPins->mcAddress);
		ConnectBuses(&cBoard, pcPins->GetData(), &pcRamPins->mcData);
		pcPins->GetPHI2()->Connect(pcClock);
		pcRamPins->mpcPHI2->Connect(pcClock);
		ConnectPorts(&cBoard, pcPins->GetRWB(), pcRamPins->mpcRWB);
		ConnectPorts(&cBoard, pcPins->GetVDA(), pcRamPins->mpcVDA);
		ConnectPorts(&cBoard, pcPins->GetVPA(), pcRamPins->mpcVPA);
		pcPins->GetRESB()->Connect(pcResetTrace);
		pcPins->GetBE()->Connect(pcHigh);
		pcPins->GetIRQB()->Connect(pcHigh);
		pcPins->GetNMIB()->Connect(pcHigh);
		pcPins->GetABORTB()->Connect(pcHigh);
	}

	void Start(void)
	{
		cBoard.Start();
		cBoard.mpcTimeline->RunToTime(3 * CHIP_CLOCK_PERIOD);
		cBoard.Write(pcReset, true);
	}

	bool Cycle(void)
	{
		cBoard.mpcTimeline->RunToTime(CHIP_CLOCK_PERIOD);
		return pcChip->GetMPU()->IsStopped();
	}

	//Padding is cleared so states can be compared whole.
	void GetState(SW65CCoreState* psState)
	{
		memset(psState, 0, sizeof(SW65CCoreState));
		pcChip->GetMPU()->GetState(psState);
	}

	void Kill(void)
	{
		cBoard.Kill();
		cContext.Kill();
	}
};


//////////////////////////////////////////////////////////////////////////
//
// The same with a 65C64 running Load65C64Program.
//
//////////////////////////////////////////////////////////////////////////
struct S65C64Computer
{
	CTest65C64Context*	pcContext;
	CTest65C64Memory	cMemory;
	CTestBoard			cBoard;
	C65C64Chip*			pcChip;
	CConstant*			pcReset;

	void Init(void)
	{
		C65C64ChipPins*		pcPins;
		CTestRamPins*		pcRamPins;
		CTestRam*			pcRam;
		CTrace*				pcResetTrace;
		CTrace*				pcHigh;
		CTrace*				pcClock;
		size				i;

		pcContext = (CTest65C64Context*)malloc(sizeof(CTest65C64Context));
		Load65C64Program(pcContext);
		cMemory.mpcContext = pcContext;
		cBoard.Init();
		pcClock = AddChipClock(&cBoard, CHIP_CLOCK_FREQUENCY, NULL);
		pcReset = cBoard.Input("RESB", 0, 1, &pcResetTrace);
		pcHigh = cBoard.Input("High", true);
		pcChip = &OMalloc<C65C64Chip>(cBoard.mpcTop, "U1", OMalloc<C65C64ChipPins>(cBoard.mpcFamily));
		pcPins = pcChip->GetChipPins();
		cBoard.Power(pcChip);
		pcRam = &OMalloc<CTestRam>(cBoard.mpcTop, OMalloc<CTestRamPins>(cBoard.mpcFamily, 32, 16, true), &cMemory, true);
		pcRamPins = pcRam->GetRamPins();
		cBoard.Power(pcRam);
		ConnectBuses(&cBoard, pcPins->GetAddress(), &pcRamPins->mcAddress);
		ConnectBuses(&cBoard, pcPins->GetData(), &pcRamPins->mcData);
		pcPins->GetPHI2()->Connect(pcClock);
		pcRamPins->mpcPHI2->Connect(pcClock);
		ConnectPorts(&cBoard, pcPins->GetRWB(), pcRamPins->mpcRWB);
		ConnectPorts(&cBoard, pcPins->GetVDA(), pcRamPins->mpcVDA);
		ConnectPorts(&cBoard, pcPins->GetVPA(), pcRamPins->mpcVPA);
		ConnectPorts(&cBoard, pcPins->GetVPB(), pcRamPins->mpcVPB);
		ConnectPorts(&cBoard, pcPins->GetLowerByte(), pcRamPins->mpcLowerByte);
		ConnectPorts(&cBoard, pcPins->GetUpperByte(), pcRamPins->mpcUpperByte);
		pcPins->GetRESB()->Connect(pcResetTrace);
		for (i = 0; i < NUM_65C64_EDGE_INTERRUPTS; i++)
		{
			pcPins->GetEIRQB(i)->Connect(pcHigh);
		}
		pcPins->GetLIRQB()->Connect(pcHigh);
	}

	void Start(void)
	{
		cBoard.Start();
		cBoard.mpcTimeline->RunToTime(3 * CHIP_CLOCK_PERIOD);
		cBoard.Write(pcReset, true);
	}

	bool Cycle(void)
	{
		cBoard.mpcTimeline->RunToTime(CHIP_CLOCK_PERIOD);
		return pcChip->GetMPU()->IsStopped();
	}

	void GetState(S65C64CoreState* psState)
	{
		memset(psState, 0, sizeof(S65C64CoreState));
		pcChip->GetMPU()->GetState(psState);
	}

	void Kill(void)
	{
		cBoard.Kill();
		free(pcContext);
	}
};


//////////////////////////////////////////////////////////////////////////
//
// Registers and latches changed so a core that isn't given a whole state
// goes wrong.
//
//////////////////////////////////////////////////////////////////////////
void ScrambleCoreState(SW65CCoreState* psState)
{
	psState->uiAccumulator ^= 0x5A5A;
	psState->uiXIndex ^= 0xA5A5;
	psState->uiYIndex ^= 0x1234;
	psState->uiStackPointer ^= 0x00F0;
	psState->cProgramCounter.Init(0x12, 0x3456);
	psState->cAddress.Init(0x00, 0x0123);
	psState->uiData ^= 0x5A;
	psState->iCycle = 0;
	psState->iNextCycle = 1;
	psState->bCarryFlag = !psState->bCarryFlag;
	psState->bPreviousClockLow = !psState->bPreviousClockLow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ScrambleCoreState(S65C64CoreState* psState)
{
	psState->uiA ^= 0x5A5A5A5A5A5A5A5AULL;
	psState->uiX ^= 0xA5A5;
	psState->uiC ^= 0x1000;
	psState->uiS ^= 0x0100;
	psState->uiDI ^= 0x77;
	psState->uiNumTransfers = 0;
	psState->eDecoded = DK65_None;
	psState->eInstruction = IK65_FetchNext;
	psState->iCycle = 0;
	psState->iNextCycle = 1;
	psState->uiIOData ^= 0x5A5A;
	psState->bPreviousClockLow = !psState->bPreviousClockLow;
}


//////////////////////////////////////////////////////////////////////////
//
// Two computers run the same program.  After uiCycles the second's core is
// scrambled and then given the first's state.  Both must stop together
// with the same core and memory.  Returns false once the first stopped
// before uiCycles, past the end of the program.
//
//////////////////////////////////////////////////////////////////////////
template<class COMPUTER, class STATE>
bool RunChipStateLockstep(size uiCycles, uint32 uiFirstAddress, uint32 uiLastAddress)
{
	COMPUTER	cFirst;
	COMPUTER	cSecond;
	STATE		sState;
	STATE		sSecondState;
	size		i;
	bool		bFirstStopped;
	bool		bSecondStopped;
	uint32		uiAddress;
	bool		bRunning;

	cFirst.Init();
	cSecond.Init();
	cFirst.Start();
	cSecond.Start();
	bRunning = true;
	for (i = 0; i < uiCycles; i++)
	{
		if (cFirst.Cycle())
		{
			bRunning = false;
			break;
		}
		cSecond.Cycle();
	}

	if (bRunning)
	{
		cSecond.GetState(&sState);
		ScrambleCoreState(&sState);
		cSecond.pcChip->GetMPU()->SetState(&sState);
		cFirst.GetState(&sState);
		cSecond.pcChip->GetMPU()->SetState(&sState);
		cSecond.GetState(&sSecondState);
		AssertMemory(&sState, &sSecondState, sizeof(STATE));

		bFirstStopped = false;
		bSecondStopped = false;
		for (i = 0; (i < CHIP_MAX_CYCLES) && !bFirstStopped; i++)
		{
			bFirstStopped = cFirst.Cycle();
			bSecondStopped = cSecond.Cycle();
			AssertBool(bFirstStopped, bSecondStopped);
		}
		AssertTrue(bFirstStopped);
		cFirst.GetState(&sState);
		cSecond.GetState(&sSecondState);
		AssertMemory(&sState, &sSecondState, sizeof(STATE));
		for (uiAddress = uiFirstAddress; uiAddress <= uiLastAddress; uiAddress++)
		{
			AssertIntHex(cFirst.cMemory.mpcContext->GetByte(uiAddress), cSecond.cMemory.mpcContext->GetByte(uiAddress));
		}
	}

	cSecond.Kill();
	cFirst.Kill();
	return bRunning;
}


//////////////////////////////////////////////////////////////////////////
//
// A core's state taken at any cycle of a program and given to another
// core carries on exactly as the first.
//
//////////////////////////////////////////////////////////////////////////
void TestProcessorChipsCoreState(void)
{
	size	uiCycles;

	CW65C816InstructionFactory::GetInstance()->Init();
	for (uiCycles = 1; uiCycles < CHIP_MAX_CYCLES; uiCycles++)
	{
		if (!RunChipStateLockstep<SW65C816Computer, SW65CCoreState>(uiCycles, 0x0300, 0x0301))
		{
			break;
		}
	}
	AssertTrue(uiCycles > 10);
	CW65C816InstructionFactory::GetInstance()->Kill();

	C65C64InstructionFactory::GetInstance()->Init();
	for (uiCycles = 1; uiCycles < CHIP_MAX_CYCLES; uiCycles++)
	{
		if (!RunChipStateLockstep<S65C64Computer, S65C64CoreState>(uiCycles, 0x4000, 0x401F))
		{
			break;
		}
	}
	AssertTrue(uiCycles > 10);
	C65C64InstructionFactory::GetInstance()->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestProcessorChips(void)
{
	BeginTests();

	TestProcessorChipsW65C816();
	TestProcessorChips65C64();
	TestProcessorChipsOscilloscope();
	TestProcessorChipsRemoveEvent();
	TestProcessorChipsCoreState();

	TestStatistics();
}

