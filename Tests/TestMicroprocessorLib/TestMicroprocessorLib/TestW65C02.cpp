#include <string.h>
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "MicroprocessorLib/W65C02.h"
#include "MicroprocessorLib/W65C02InstructionFactory.h"
#include "MicroprocessorLib/W65C02Meta.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// W65C02S behaviour the single step tests do not cover: reset, hardware interrupts,
// WAI, STP, SOB and SYNC.  Programs run from reset in a flat 64KB memory.
//
//////////////////////////////////////////////////////////////////////////


class CTestW65C02Context
{
public:
	uint8	mauiMemory[0x10000];
	uint32	muiLastAddress;
	size	muiCycles;
	size	muiSyncCycles;
	size	muiIRQCycle;	//Pull IRQB low from this cycle until the vector is read.
	size	muiNMICycle;	//Pull NMIB low on this cycle until the vector is read.
	size	muiSOBCycle;	//Pull SOB low for one cycle.

	void Init(uint16 uiResetAddress)
	{
		memset(mauiMemory, 0xEA, sizeof(mauiMemory));
		mauiMemory[0xFFFC] = (uint8)uiResetAddress;
		mauiMemory[0xFFFD] = (uint8)(uiResetAddress >> 8);
		muiLastAddress = uiResetAddress - 1;
		muiCycles = 0;
		muiSyncCycles = 0;
		muiIRQCycle = 0;
		muiNMICycle = 0;
		muiSOBCycle = 0;
	}

	void SetByte(uint32 uiAddress, uint8 uiValue)
	{
		mauiMemory[uiAddress & 0xFFFF] = uiValue;
		muiLastAddress = uiAddress;
	}

	void SetByte(uint8 uiValue)
	{
		SetByte(muiLastAddress + 1, uiValue);
	}

	void SetShort(uint32 uiAddress, uint16 uiValue)
	{
		SetByte(uiAddress, (uint8)uiValue);
		SetByte((uint8)(uiValue >> 8));
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C02TickHigh(CW65C02Meta* pcMPU, void* pvContext)
{
	CTestW65C02Context*	pcContext;
	uint16				uiAddress;

	pcContext = (CTestW65C02Context*)pvContext;
	pcContext->muiCycles++;

	uiAddress = pcMPU->GetAddress()->Get();
	if (pcMPU->GetRWB()->IsHigh())
	{
		pcMPU->GetData()->Set(pcContext->mauiMemory[uiAddress]);
	}
	else if (pcMPU->GetRWB()->IsLow())
	{
		pcContext->mauiMemory[uiAddress] = pcMPU->GetData()->Get();
	}

	if (pcMPU->GetSYNC()->IsHigh())
	{
		pcContext->muiSyncCycles++;
	}

	if ((pcContext->muiIRQCycle != 0) && (pcContext->muiCycles == pcContext->muiIRQCycle))
	{
		pcMPU->GetIRQB()->Set(false);
	}
	if ((pcContext->muiNMICycle != 0) && (pcContext->muiCycles == pcContext->muiNMICycle))
	{
		pcMPU->GetNMIB()->Set(false);
	}
	if (pcMPU->GetVPB()->IsLow())
	{
		pcMPU->GetIRQB()->Set(true);
		pcMPU->GetNMIB()->Set(true);
	}
	pcMPU->GetSOB()->Set((pcContext->muiSOBCycle == 0) || (pcContext->muiCycles != pcContext->muiSOBCycle));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C02TickLow(CW65C02Meta* pcMPU, void* pvContext)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size TestW65C02Run(CW65C02Meta* pcMPU, CTestW65C02Context* pcContext)
{
	//Runs from reset until STP.  Returns the number of instructions.
	size	uiInstructions;

	pcMPU->Init(TestW65C02TickHigh, TestW65C02TickLow, pcContext);
	uiInstructions = 0;
	while (pcMPU->TickInstruction())
	{
		uiInstructions++;
		if (uiInstructions > 10000)
		{
			break;
		}
	}
	return uiInstructions;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C02Reset(void)
{
	//Reset: emulation mode, I set, D clear, the program starts at the reset vector.
	CTestW65C02Context	cContext;
	CW65C02Meta			cMPU;
	CW65C02*			pcCPU;

	cContext.Init(0x0200);
	cContext.SetByte(0x0200, W65C02_SED_Implied);
	cContext.SetByte(W65C02_LDA_Immediate);
	cContext.SetByte(0x42);
	cContext.SetByte(W65C02_STA_ZeroPage);
	cContext.SetByte(0x10);
	cContext.SetByte(W65C02_STP_Implied);

	AssertSize(4, TestW65C02Run(&cMPU, &cContext));
	pcCPU = cMPU.GetMPU();
	AssertTrue(pcCPU->IsStopped());
	AssertByteHex((uint8)0x42, cContext.mauiMemory[0x10]);
	AssertShortHex((uint16)0x0042, pcCPU->GetC());
	AssertShortHex((uint16)0x0206, pcCPU->GetProgramCounter()->GetOffset());
	AssertByteHex((uint8)0x2C, pcCPU->GetProcessorRegisterValue());
	AssertTrue(pcCPU->IsEmulation());

	//SYNC is high once per instruction, on its opcode fetch.
	AssertSize(4, cContext.muiSyncCycles);

	cMPU.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C02IRQ(void)
{
	//IRQ pushes PC and P with B clear, sets I and clears D, and uses vector FFFE.
	CTestW65C02Context	cContext;
	CW65C02Meta			cMPU;
	CW65C02*			pcCPU;

	cContext.Init(0x0200);
	cContext.muiIRQCycle = 20;
	cContext.SetByte(0x0200, W65C02_LDX_Immediate);
	cContext.SetByte(0xFF);
	cContext.SetByte(W65C02_TXS_Implied);
	cContext.SetByte(W65C02_SED_Implied);
	cContext.SetByte(W65C02_CLI_Implied);
	cContext.SetByte(0x0300, W65C02_STP_Implied);
	cContext.SetShort(0xFFFE, 0x0300);

	TestW65C02Run(&cMPU, &cContext);
	pcCPU = cMPU.GetMPU();
	AssertShortHex((uint16)0x0301, pcCPU->GetProgramCounter()->GetOffset());
	AssertShortHex((uint16)0x01FC, pcCPU->GetStackPointer16Bit());
	AssertByteHex((uint8)0xA8, cContext.mauiMemory[0x01FD]);  //N from LDX #$FF, D, bit 5, B clear.
	AssertByteHex((uint8)0x02, cContext.mauiMemory[0x01FF]);
	AssertTrue(pcCPU->IsInterruptDisable());
	AssertFalse(pcCPU->IsDecimal());

	cMPU.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C02NMI(void)
{
	//NMI is taken with I set and uses vector FFFA.
	CTestW65C02Context	cContext;
	CW65C02Meta			cMPU;
	CW65C02*			pcCPU;

	cContext.Init(0x0200);
	cContext.muiNMICycle = 12;
	cContext.SetByte(0x0200, W65C02_SEI_Implied);
	cContext.SetByte(0x0400, W65C02_STP_Implied);
	cContext.SetShort(0xFFFA, 0x0400);
	cContext.SetShort(0xFFFE, 0x0300);

	TestW65C02Run(&cMPU, &cContext);
	pcCPU = cMPU.GetMPU();
	AssertShortHex((uint16)0x0401, pcCPU->GetProgramCounter()->GetOffset());

	cMPU.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C02BRKAndRTI(void)
{
	//BRK pushes the address after its signature byte and P with B set.  RTI returns there.
	CTestW65C02Context	cContext;
	CW65C02Meta			cMPU;
	CW65C02*			pcCPU;

	cContext.Init(0x0200);
	cContext.SetByte(0x0200, W65C02_LDX_Immediate);
	cContext.SetByte(0xFF);
	cContext.SetByte(W65C02_TXS_Implied);
	cContext.SetByte(W65C02_BRK_Interrupt);
	cContext.SetByte(0x99);
	cContext.SetByte(W65C02_LDA_Immediate);
	cContext.SetByte(0x55);
	cContext.SetByte(W65C02_STP_Implied);
	cContext.SetByte(0x0300, W65C02_RTI_Stack);
	cContext.SetShort(0xFFFE, 0x0300);

	TestW65C02Run(&cMPU, &cContext);
	pcCPU = cMPU.GetMPU();
	AssertShortHex((uint16)0x0055, pcCPU->GetC());
	AssertShortHex((uint16)0x0208, pcCPU->GetProgramCounter()->GetOffset());
	AssertByteHex((uint8)0x02, cContext.mauiMemory[0x01FF]);
	AssertByteHex((uint8)0x05, cContext.mauiMemory[0x01FE]);
	AssertByteHex((uint8)0xB4, cContext.mauiMemory[0x01FD]);  //N from LDX #$FF, bit 5, B set, I.

	cMPU.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C02WAIWithInterruptsDisabled(void)
{
	//Datasheet: with I set an IRQ ends WAI and execution continues after it, without the handler.
	CTestW65C02Context	cContext;
	CW65C02Meta			cMPU;
	CW65C02*			pcCPU;

	cContext.Init(0x0200);
	cContext.muiIRQCycle = 30;
	cContext.SetByte(0x0200, W65C02_SEI_Implied);
	cContext.SetByte(W65C02_WAI_Implied);
	cContext.SetByte(W65C02_LDA_Immediate);
	cContext.SetByte(0x77);
	cContext.SetByte(W65C02_STP_Implied);
	cContext.SetShort(0xFFFE, 0x0300);
	cContext.SetByte(0x0300, W65C02_STP_Implied);

	TestW65C02Run(&cMPU, &cContext);
	pcCPU = cMPU.GetMPU();
	AssertShortHex((uint16)0x0077, pcCPU->GetC());
	AssertShortHex((uint16)0x0205, pcCPU->GetProgramCounter()->GetOffset());
	AssertTrue(cContext.muiCycles > 30);

	cMPU.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C02SetOverflow(void)
{
	//A falling edge on SOB sets V.
	CTestW65C02Context	cContext;
	CW65C02Meta			cMPU;
	CW65C02*			pcCPU;

	cContext.Init(0x0200);
	cContext.muiSOBCycle = 12;
	cContext.SetByte(0x0200, W65C02_CLV_Implied);
	cContext.SetByte(W65C02_NOP_Implied);
	cContext.SetByte(W65C02_NOP_Implied);
	cContext.SetByte(W65C02_NOP_Implied);
	cContext.SetByte(W65C02_NOP_Implied);
	cContext.SetByte(W65C02_STP_Implied);

	TestW65C02Run(&cMPU, &cContext);
	pcCPU = cMPU.GetMPU();
	AssertTrue(pcCPU->IsOverflowFlag());

	cMPU.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestW65C02(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	CW65C02InstructionFactory::GetInstance()->Init();

	TestW65C02Reset();
	TestW65C02IRQ();
	TestW65C02NMI();
	TestW65C02BRKAndRTI();
	TestW65C02WAIWithInterruptsDisabled();
	TestW65C02SetOverflow();

	CW65C02InstructionFactory::GetInstance()->Kill();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

