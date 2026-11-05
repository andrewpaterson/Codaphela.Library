#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BaseLib/ListenSocket.h"
#include "BaseLib/Chars.h"
#include "ThreadLib/NetworkConnection.h"
#include "ThreadLib/SharedMemoryChannel.h"
#include "EmulatorDebugLib/DebugServer.h"
#include "EmulatorDebugLib/DebugClient.h"
#include "EmulatorDebugLib/Flat65C816DebugTarget.h"
#include "EmulatorDebugLib/Flat65C64DebugTarget.h"
#include "EmulatorDebugLib/DebugFormat.h"
#include "TestLib/Assert.h"
#include "../../TestMicroprocessorLib/TestMicroprocessorLib/Test65C64Context.h"


#define TEST_DEBUG_TIMEOUT		5000	//Milliseconds to wait for the server's answer.
#define TEST_DEBUG_BUDGET		1000	//Instructions the target runs each Tick.
#define TEST_DEBUG_816_MEMORY	0x20000


enum ETestDebugTransport
{
	TDT_Network,
	TDT_SharedMemory,
};


//Both ends of one channel, in this process.
struct STestDebugLink
{
	CListenSocket			cListen;
	CNetworkConnection		cServerNetwork;
	CNetworkConnection		cClientNetwork;
	CSharedMemoryChannel	cServerShared;
	CSharedMemoryChannel	cClientShared;
	CProcessChannel*		pcServer;
	CProcessChannel*		pcClient;
	CDebugServer			cServer;
	CDebugClient			cClient;
};


static int	giTestDebugChannels = 0;


//The few W65C816 instructions the tests' programs use.  The real decoder is with the W65C816's assembler, in the
//HCP65 repository; EmulatorDebugLib has none of its own.
class CTestDebug816Decoder : public CDebugDecoder
{
public:
	size Decode(CDebugTarget* pcTarget, uint32 uiAddress, size uiCount, SDebugInstruction* asDest) override
	{
		size				i;
		uint8				uiOpcode;
		SDebugInstruction*	psInstruction;

		for (i = 0; i < uiCount; i++)
		{
			if (pcTarget->PeekMemory(uiAddress, &uiOpcode, 1) != 1)
			{
				break;
			}
			psInstruction = &asDest[i];
			StartDebugInstruction(psInstruction, pcTarget, uiAddress, ((uiOpcode == 0x20) ? 3 : ((uiOpcode == 0x80) ? 2 : 1)));
			switch (uiOpcode)
			{
			case 0xE8:	strcpy(psInstruction->szText, "inx");	break;
			case 0xC8:	strcpy(psInstruction->szText, "iny");	break;
			case 0x48:	strcpy(psInstruction->szText, "pha");	break;
			case 0x68:	strcpy(psInstruction->szText, "pla");	break;
			case 0xDB:	strcpy(psInstruction->szText, "stp");	break;
			case 0xEA:	strcpy(psInstruction->szText, "nop");	break;
			case 0x60:
				strcpy(psInstruction->szText, "rts");
				psInstruction->uiKind = DI_Return;
				break;
			case 0x20:
				psInstruction->uiKind = DI_Call;
				psInstruction->uiTarget = (uiAddress & 0xFF0000) | psInstruction->auiBytes[1] | (psInstruction->auiBytes[2] << 8);
				psInstruction->uiHasTarget = true;
				sprintf(psInstruction->szText, "jsr $%04X", psInstruction->uiTarget & 0xFFFF);
				break;
			case 0x80:
				psInstruction->uiKind = DI_Jump;
				psInstruction->uiTarget = (uiAddress & 0xFF0000) | ((uiAddress + 2 + (int8)psInstruction->auiBytes[1]) & 0xFFFF);
				psInstruction->uiHasTarget = true;
				sprintf(psInstruction->szText, "bra $%04X", psInstruction->uiTarget & 0xFFFF);
				break;
			default:
				strcpy(psInstruction->szText, "?");
				break;
			}
			uiAddress += psInstruction->uiLength;
		}
		return i;
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool TestDebugLinkInit(STestDebugLink* psLink, ETestDebugTransport eTransport, CDebugTarget* pcTarget)
{
	CChars	szName;
	bool	bResult;

	psLink->cServerNetwork.Init();
	psLink->cClientNetwork.Init();
	psLink->cServerShared.Init();
	psLink->cClientShared.Init();

	if (eTransport == TDT_Network)
	{
		bResult = psLink->cListen.Init(NETWORK_LOOPBACK_ADDRESS, 0);
		bResult = bResult && psLink->cClientNetwork.Connect(NETWORK_LOOPBACK_ADDRESS, psLink->cListen.GetPort());
		bResult = bResult && psLink->cServerNetwork.Accept(&psLink->cListen, TEST_DEBUG_TIMEOUT);
		psLink->pcServer = &psLink->cServerNetwork;
		psLink->pcClient = &psLink->cClientNetwork;
	}
	else
	{
		giTestDebugChannels++;
		szName.Init("TestDebugChannel")->Append(giTestDebugChannels);
		bResult = psLink->cServerShared.Create(szName.Text());
		bResult = bResult && psLink->cClientShared.Connect(szName.Text());
		szName.Kill();
		psLink->pcServer = &psLink->cServerShared;
		psLink->pcClient = &psLink->cClientShared;
	}

	psLink->cServer.Init(psLink->pcServer, pcTarget);
	psLink->cClient.Init(psLink->pcClient);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestDebugLinkKill(STestDebugLink* psLink, ETestDebugTransport eTransport)
{
	psLink->cClient.Kill();
	psLink->cServer.Kill();
	psLink->cClientShared.Kill();
	psLink->cServerShared.Kill();
	psLink->cClientNetwork.Kill();
	psLink->cServerNetwork.Kill();
	if (eTransport == TDT_Network)
	{
		psLink->cListen.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Ticks the server and polls the client until the client has counted
// uiCount of what fCount counts.
//
//////////////////////////////////////////////////////////////////////////
static bool TestDebugWait(STestDebugLink* psLink, size (CDebugClient::*fCount)(void), size uiCount)
{
	int		i;

	for (i = 0; i < TEST_DEBUG_TIMEOUT; i++)
	{
		psLink->cServer.Tick(TEST_DEBUG_BUDGET);
		psLink->cClient.Poll(1);
		if ((psLink->cClient.*fCount)() >= uiCount)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The next stop, which must be at uiAddress for eReason.
//
//////////////////////////////////////////////////////////////////////////
static void TestDebugWaitForStop(STestDebugLink* psLink, EDebugStop eReason, uint32 uiAddress)
{
	size	uiStops;

	uiStops = psLink->cClient.NumStops() + 1;
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumStops, uiStops));
	AssertTrue(psLink->cClient.IsStopped());
	AssertInt(eReason, psLink->cClient.GetStopReason());
	AssertIntHex(uiAddress, psLink->cClient.GetStopAddress());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestDebugWaitForRunning(STestDebugLink* psLink)
{
	int		i;

	for (i = 0; (i < TEST_DEBUG_TIMEOUT) && !psLink->cClient.IsRunning(); i++)
	{
		psLink->cServer.Tick(TEST_DEBUG_BUDGET);
		psLink->cClient.Poll(1);
	}
	AssertTrue(psLink->cClient.IsRunning());
	AssertInt(DSS_Running, psLink->cServer.GetState());
}


//////////////////////////////////////////////////////////////////////////
//
// Everything sent so far has been handled: a DM_ReadRegisters sent after
// it has been answered.
//
//////////////////////////////////////////////////////////////////////////
static void TestDebugSync(STestDebugLink* psLink)
{
	size	uiReplies;

	uiReplies = psLink->cClient.NumRegisterReplies() + 1;
	AssertTrue(psLink->cClient.ReadRegisters());
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumRegisterReplies, uiReplies));
}


//////////////////////////////////////////////////////////////////////////
//
// A loop with a call, on a W65C816 in emulation mode:
//
//	$1000	INX
//	$1001	INX
//	$1002	JSR $2000
//	$1005	INY
//	$1006	BRA $1000
//
//	$2000	INX
//	$2001	PHA
//	$2002	PLA
//	$2003	RTS
//
//////////////////////////////////////////////////////////////////////////
static void TestDebugServer65C816(ETestDebugTransport eTransport)
{
	CFlat65C816DebugTarget	cTarget;
	CTestDebug816Decoder	cDecoder;
	STestDebugLink			sLink;
	STestDebugLink*			psLink;
	CDebugClient*			pcClient;
	SDebugBreakpoint		sBreakpoint;
	SDebugInstruction*		psInstruction;
	uint32					uiRequest;
	CChars					sz;
	SDebugRange				sLine;
	SDebugRange				sFunction;
	SDebugRegister*			psRegister;
	static uint8			auiMain[] = { 0xE8, 0xE8, 0x20, 0x00, 0x20, 0xC8, 0x80, 0xF8 };
	static uint8			auiFunction[] = { 0xE8, 0x48, 0x68, 0x60 };
	static uint8			auiText[] = { 'a', 'b', 'c' };
	static uint8			auiStop[] = { 0xDB };

	cTarget.Init(TEST_DEBUG_816_MEMORY);
	cTarget.SetDecoder(&cDecoder);
	psLink = &sLink;
	AssertTrue(TestDebugLinkInit(psLink, eTransport, &cTarget));
	pcClient = &psLink->cClient;

	//Welcomed, and stopped from the start.  The hello is a request answered by the state.
	uiRequest = pcClient->Hello();
	AssertInt(1, uiRequest);
	AssertTrue(pcClient->IsBusy());
	TestDebugWaitForStop(psLink, DS_Start, 0);
	AssertFalse(pcClient->IsBusy());
	AssertString("W65C816", pcClient->GetProcessorName());
	AssertSize(24, pcClient->NumAddressBits());
	AssertInt(DAF_Bank, pcClient->GetAddressFormat());
	AssertTrue(pcClient->CanDecode());
	AssertString("NVMXDIZC", pcClient->GetRegister(D816R_P)->szFlags);
	AssertString("", pcClient->GetRegister(D816R_C)->szFlags);
	AssertSize(D816R_NumRegisters, pcClient->NumRegisters());
	psRegister = pcClient->GetRegister(D816R_PC);
	AssertString("PC", psRegister->szName);
	AssertInt(16, psRegister->uiBits);
	AssertInt(DRK_Flags, pcClient->GetRegister(D816R_P)->uiKind);
	AssertNull(pcClient->GetRegister(D816R_NumRegisters));

	//Loaded and started at $1000.
	AssertTrue(pcClient->Load(0x1000, auiMain, sizeof(auiMain)));
	AssertTrue(pcClient->Load(0x2000, auiFunction, sizeof(auiFunction)));
	AssertTrue(pcClient->SetProgramAddress(0x1000));
	TestDebugSync(psLink);
	AssertLongLongIntHex(0x1000, pcClient->GetRegisterValue(D816R_PC));
	AssertTrue(pcClient->ReadMemory(0x1000, sizeof(auiMain)));
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumMemoryReplies, 1));
	AssertIntHex(0x1000, pcClient->GetMemoryAddress());
	AssertSize(sizeof(auiMain), pcClient->GetMemorySize());
	AssertMemory(auiMain, pcClient->GetMemory(), sizeof(auiMain));

	//Continue to the breakpoint at INY, and round the loop to it again.
	AssertTrue(pcClient->AddBreakpoint(0x1005, true));
	AssertTrue(pcClient->Continue());
	TestDebugWaitForStop(psLink, DS_Breakpoint, 0x1005);
	AssertLongLongIntHex(3, pcClient->GetRegisterValue(D816R_X));
	AssertTrue(pcClient->Continue());
	TestDebugWaitForStop(psLink, DS_Breakpoint, 0x1005);
	AssertLongLongIntHex(6, pcClient->GetRegisterValue(D816R_X));
	AssertLongLongIntHex(1, pcClient->GetRegisterValue(D816R_Y));

	//One instruction, then Continue goes on from there.
	AssertTrue(pcClient->Step(1));
	TestDebugWaitForStop(psLink, DS_Step, 0x1006);
	AssertLongLongIntHex(2, pcClient->GetRegisterValue(D816R_Y));
	AssertTrue(pcClient->Continue());
	TestDebugWaitForStop(psLink, DS_Breakpoint, 0x1005);
	AssertLongLongIntHex(9, pcClient->GetRegisterValue(D816R_X));

	//Step over the line $1000 to $1004 (the call is stepped over), stopping at INY where the breakpoint is.
	sLine.uiStart = 0x1000;
	sLine.uiEnd = 0x1005;
	AssertTrue(pcClient->Step(2));
	TestDebugWaitForStop(psLink, DS_Step, 0x1000);
	AssertTrue(pcClient->StepRange(&sLine, 1, true));
	TestDebugWaitForStop(psLink, DS_Step, 0x1005);
	AssertLongLongIntHex(12, pcClient->GetRegisterValue(D816R_X));

	//Step into it: the call stops at the function.
	AssertTrue(pcClient->Step(2));
	TestDebugWaitForStop(psLink, DS_Step, 0x1000);
	AssertTrue(pcClient->StepRange(&sLine, 1, false));
	TestDebugWaitForStop(psLink, DS_Step, 0x2000);

	//Step out of the function: the whole of it stepped over.
	sFunction.uiStart = 0x2000;
	sFunction.uiEnd = 0x2004;
	AssertTrue(pcClient->StepRange(&sFunction, 1, true));
	TestDebugWaitForStop(psLink, DS_Step, 0x1005);
	AssertLongLongIntHex(15, pcClient->GetRegisterValue(D816R_X));

	//Step over the JSR: the instruction after it.
	AssertTrue(pcClient->Step(4));
	TestDebugWaitForStop(psLink, DS_Step, 0x1002);
	AssertTrue(pcClient->StepOver());
	TestDebugWaitForStop(psLink, DS_Step, 0x1005);
	AssertLongLongIntHex(18, pcClient->GetRegisterValue(D816R_X));

	//Into it; over an INX and a PHA (one instruction each, though PHA moves the stack), then out of it.
	AssertTrue(pcClient->Step(5));
	TestDebugWaitForStop(psLink, DS_Step, 0x2000);
	AssertTrue(pcClient->StepOver());
	TestDebugWaitForStop(psLink, DS_Step, 0x2001);
	AssertTrue(pcClient->StepOver());
	TestDebugWaitForStop(psLink, DS_Step, 0x2002);
	AssertTrue(pcClient->StepOut());
	TestDebugWaitForStop(psLink, DS_Step, 0x1005);
	AssertLongLongIntHex(21, pcClient->GetRegisterValue(D816R_X));

	//The emulator's decoder disassembles: the call's and the branch's targets, and the flow they make.
	uiRequest = pcClient->Disassemble(0x1000, 5);
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumDisassemblies, 1));
	AssertInt(uiRequest, pcClient->GetDisassemblyRequest());
	AssertSize(5, pcClient->NumInstructions());
	psInstruction = pcClient->GetInstruction(2);
	AssertIntHex(0x1002, psInstruction->uiAddress);
	AssertInt(3, psInstruction->uiLength);
	AssertInt(DI_Call, psInstruction->uiKind);
	AssertIntHex(0x2000, psInstruction->uiTarget);
	AssertString("jsr $2000", psInstruction->szText);
	AssertIntHex(0x20, psInstruction->auiBytes[0]);
	AssertIntHex(0x1006, pcClient->GetInstruction(4)->uiAddress);
	AssertString("bra $1000", pcClient->GetInstruction(4)->szText);
	AssertInt(DI_Jump, pcClient->GetInstruction(4)->uiKind);
	AssertNull(pcClient->GetInstruction(5));

	//From the middle of the jsr the instructions are wrong; landing on $1005 starts them there instead.
	AssertTrue(pcClient->Disassemble(0x1003, 3, 0x1005));
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumDisassemblies, 2));
	AssertIntHex(0x1005, pcClient->GetInstruction(0)->uiAddress);
	AssertString("iny", pcClient->GetInstruction(0)->szText);

	//Instructions that end before the address landed on start there too; ones that include it are kept.
	AssertTrue(pcClient->Disassemble(0x1000, 2, 0x2000));
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumDisassemblies, 3));
	AssertIntHex(0x2000, pcClient->GetInstruction(0)->uiAddress);
	AssertTrue(pcClient->Disassemble(0x1000, 4, 0x1002));
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumDisassemblies, 4));
	AssertIntHex(0x1000, pcClient->GetInstruction(0)->uiAddress);

	//Pause answers even when it is already stopped; a step's answer clears busy.
	uiRequest = pcClient->Pause();
	AssertTrue(pcClient->IsBusy());
	TestDebugWaitForStop(psLink, DS_Pause, 0x1005);
	AssertFalse(pcClient->IsBusy());
	AssertTrue(pcClient->Step(1));
	AssertTrue(pcClient->IsBusy());
	TestDebugWaitForStop(psLink, DS_Step, 0x1006);
	AssertFalse(pcClient->IsBusy());

	//Addresses and registers written as the welcome describes them.
	sz.Init();
	pcClient->AppendAddress(&sz, 0x016590);
	AssertString("$01:6590", sz.Text());
	sz.Kill();
	sz.Init();
	AppendDebugRegisterValue(&sz, pcClient->GetRegister(D816R_P), 0x34);
	AssertString("$34 nvMXdIzc", sz.Text());
	sz.Kill();
	AssertTrue(pcClient->Step(8));
	TestDebugWaitForStop(psLink, DS_Step, 0x1005);

	//A breakpoint in a function being stepped over stops it.
	AssertTrue(pcClient->AddBreakpoint(0x2001, true));
	AssertTrue(pcClient->Step(2));
	TestDebugWaitForStop(psLink, DS_Step, 0x1000);
	AssertTrue(pcClient->StepRange(&sLine, 1, true));
	TestDebugWaitForStop(psLink, DS_Breakpoint, 0x2001);

	//Disabled breakpoints are kept but passed; Pause stops it anywhere.
	AssertTrue(pcClient->EnableBreakpoint(0x2001, false));
	AssertTrue(pcClient->EnableBreakpoint(0x1005, false));
	AssertTrue(pcClient->Continue());
	TestDebugWaitForRunning(psLink);
	psLink->cServer.Tick(TEST_DEBUG_BUDGET);
	AssertTrue(pcClient->Pause());
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumStops, pcClient->NumStops() + 1));
	AssertInt(DS_Pause, pcClient->GetStopReason());
	AssertSize(2, cTarget.GetBreakpoints()->NumBreakpoints());
	AssertSize(0, cTarget.GetBreakpoints()->NumEnabled());
	AssertTrue(pcClient->EnableAllBreakpoints(true));
	TestDebugSync(psLink);
	AssertSize(2, cTarget.GetBreakpoints()->NumEnabled());

	//Registers and memory written.
	AssertTrue(pcClient->WriteRegister(D816R_X, 0x55));
	TestDebugSync(psLink);
	AssertLongLongIntHex(0x55, pcClient->GetRegisterValue(D816R_X));
	AssertTrue(pcClient->WriteMemory(0x3000, auiText, sizeof(auiText)));
	AssertTrue(pcClient->ReadMemory(0x2FFF, 5));
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumMemoryReplies, 2));
	AssertSize(5, pcClient->GetMemorySize());
	AssertMemory(auiText, &pcClient->GetMemory()[1], sizeof(auiText));

	//Past the memory there is nothing; a missing breakpoint is an error.
	AssertTrue(pcClient->ReadMemory(TEST_DEBUG_816_MEMORY - 2, 16));
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumMemoryReplies, 3));
	AssertSize(2, pcClient->GetMemorySize());
	uiRequest = pcClient->RemoveBreakpoint(0x9999);
	AssertTrue(uiRequest != 0);
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumErrors, 1));
	AssertInt(DM_RemoveBreakpoint, pcClient->GetErrorType());
	AssertInt(uiRequest, pcClient->GetErrorRequest());
	AssertString("No breakpoint there.", pcClient->GetError());

	//A breakpoint for a process, on a processor with none.
	uiRequest = pcClient->AddBreakpoint(0x1000, true, DEBUG_SPACE_ANY, 3);
	AssertTrue(uiRequest != 0);
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumErrors, 2));
	AssertInt(DM_AddBreakpoint, pcClient->GetErrorType());
	AssertString("No breakpoints for a process there.", pcClient->GetError());
	AssertFalse(pcClient->HasProcesses());
	AssertSize(0, pcClient->NumProcesses());

	//The breakpoints replaced by one at $1000.
	sBreakpoint.uiAddress = 0x1000;
	sBreakpoint.uiEnabled = true;
	sBreakpoint.uiSpace = DEBUG_SPACE_ANY;
	sBreakpoint.uiProcess = DEBUG_PROCESS_ANY;
	AssertTrue(pcClient->SetBreakpoints(&sBreakpoint, 1));
	AssertTrue(pcClient->Continue());
	TestDebugWaitForStop(psLink, DS_Breakpoint, 0x1000);
	AssertSize(1, cTarget.GetBreakpoints()->NumBreakpoints());

	//STP stops the processor; reset starts it again.
	AssertTrue(pcClient->WriteMemory(0x1000, auiStop, sizeof(auiStop)));
	AssertTrue(pcClient->SetBreakpoints(NULL, 0));
	AssertTrue(pcClient->Continue());
	TestDebugWaitForStop(psLink, DS_Stopped, 0x1001);
	AssertTrue(cTarget.IsStopped());
	AssertTrue(pcClient->Continue());
	TestDebugWaitForStop(psLink, DS_Stopped, 0x1001);
	AssertTrue(pcClient->Reset(false));
	TestDebugWaitForStop(psLink, DS_Reset, 0xEAEA);
	AssertFalse(cTarget.IsStopped());

	//Reset and run on: answered as running.
	uiRequest = pcClient->Reset(true);
	TestDebugWaitForRunning(psLink);
	AssertFalse(pcClient->IsBusy());
	AssertFalse(pcClient->IsStepping());
	AssertTrue(pcClient->Pause());
	TestDebugWait(psLink, &CDebugClient::NumStops, pcClient->NumStops() + 1);
	AssertInt(DS_Pause, pcClient->GetStopReason());

	//Detached: the breakpoints go and it runs on (round the loop, without the STP).
	AssertTrue(pcClient->Load(0x1000, auiMain, sizeof(auiMain)));
	AssertTrue(pcClient->SetProgramAddress(0x1000));
	AssertTrue(pcClient->AddBreakpoint(0x1000, true));
	AssertTrue(pcClient->Detach());
	AssertInt(DCS_Closed, pcClient->GetState());
	TestDebugWait(psLink, &CDebugClient::NumErrors, 1000);
	AssertTrue(psLink->cServer.IsClosed());
	AssertFalse(psLink->cServer.IsConnected());
	AssertInt(DSS_Running, psLink->cServer.GetState());
	AssertSize(0, cTarget.GetBreakpoints()->NumBreakpoints());

	TestDebugLinkKill(psLink, eTransport);
	cTarget.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The same debugger on a 65C64 with 32 bit addresses: a loop calling a
// function, stepped over, into and out of.
//
//////////////////////////////////////////////////////////////////////////
static void TestDebugServer65C64(ETestDebugTransport eTransport)
{
	CFlat65C64DebugTarget	cTarget;
	CTest65C64Context*		pcProgram;
	STestDebugLink			sLink;
	STestDebugLink*			psLink;
	CDebugClient*			pcClient;
	SDebugRange				sLine;
	SDebugRange				sFunction;
	uint32					uiAddY;
	uint32					uiFunctionEnd;
	CChars					sz;

	pcProgram = (CTest65C64Context*)malloc(sizeof(CTest65C64Context));
	pcProgram->Init(0x1000);
	pcProgram->Op(Op65RE(C65_ADD, R65_X, 1));
	pcProgram->OpLong(Op65Jump(R65_S, OPERAND_MODE_IMMEDIATE, 4, false, true, false), 0x1100);
	uiAddY = pcProgram->Here();
	pcProgram->Op(Op65RE(C65_ADD, R65_Y, 1));
	pcProgram->OpLong(Op65Jump(R65_S, OPERAND_MODE_IMMEDIATE, 4, false, false, false), 0x1000);
	pcProgram->Org(0x1100);
	pcProgram->Op(Op65RE(C65_ADD, R65_X, 2));
	pcProgram->Op(Op65Return(R65_S, false, true, false));
	uiFunctionEnd = pcProgram->Here();

	cTarget.Init(TEST_65C64_LOW_SIZE);
	psLink = &sLink;
	AssertTrue(TestDebugLinkInit(psLink, eTransport, &cTarget));
	pcClient = &psLink->cClient;

	AssertTrue(pcClient->Hello());
	TestDebugWaitForStop(psLink, DS_Start, 0);
	AssertString("65C64", pcClient->GetProcessorName());
	AssertSize(32, pcClient->NumAddressBits());
	AssertInt(DAF_Flat, pcClient->GetAddressFormat());
	AssertTrue(pcClient->CanDecode());
	AssertString("C", pcClient->GetRegister(F64R_C)->szName);
	AssertString("ELFGNVZC", pcClient->GetRegister(F64R_P)->szFlags);
	sz.Init();
	pcClient->AppendAddress(&sz, 0x1000);
	AssertString("$00001000", sz.Text());
	sz.Kill();

	AssertTrue(pcClient->Load(0, pcProgram->mauiLow, TEST_65C64_LOW_SIZE));
	AssertTrue(pcClient->SetProgramAddress(0x1000));

	//MicroprocessorLib's decoding: the jsr is a call of its words' length, the function's ret a return.
	AssertTrue(pcClient->Disassemble(0x1000, 2));
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumDisassemblies, 1));
	AssertSize(2, pcClient->NumInstructions());
	AssertInt(2, pcClient->GetInstruction(0)->uiLength);
	AssertIntHex(0x1002, pcClient->GetInstruction(1)->uiAddress);
	AssertInt(DI_Call, pcClient->GetInstruction(1)->uiKind);
	AssertIntHex(uiAddY, (uint32)(0x1002 + pcClient->GetInstruction(1)->uiLength));
	AssertTrue(strncmp(pcClient->GetInstruction(1)->szText, ".word $", 7) == 0);
	AssertTrue(pcClient->Disassemble(0x1102, 1));
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumDisassemblies, 2));
	AssertInt(DI_Return, pcClient->GetInstruction(0)->uiKind);
	AssertTrue(pcClient->AddBreakpoint(uiAddY, true));
	AssertTrue(pcClient->Continue());
	TestDebugWaitForStop(psLink, DS_Breakpoint, uiAddY);
	AssertLongLongIntHex(3, pcClient->GetRegisterValue(F64R_X));
	AssertLongLongIntHex(0x1FFFF, pcClient->GetRegisterValue(F64R_S));

	//Over the line with the call.
	sLine.uiStart = 0x1000;
	sLine.uiEnd = uiAddY;
	AssertTrue(pcClient->Step(2));
	TestDebugWaitForStop(psLink, DS_Step, 0x1000);
	AssertTrue(pcClient->StepRange(&sLine, 1, true));
	TestDebugWaitForStop(psLink, DS_Step, uiAddY);
	AssertLongLongIntHex(6, pcClient->GetRegisterValue(F64R_X));

	//Into it, then out of the function.
	AssertTrue(pcClient->Step(2));
	TestDebugWaitForStop(psLink, DS_Step, 0x1000);
	AssertTrue(pcClient->StepRange(&sLine, 1, false));
	TestDebugWaitForStop(psLink, DS_Step, 0x1100);
	AssertTrue(pcClient->GetRegisterValue(F64R_S) < 0x1FFFF);
	sFunction.uiStart = 0x1100;
	sFunction.uiEnd = uiFunctionEnd;
	AssertTrue(pcClient->StepRange(&sFunction, 1, true));
	TestDebugWaitForStop(psLink, DS_Step, uiAddY);
	AssertLongLongIntHex(9, pcClient->GetRegisterValue(F64R_X));
	AssertLongLongIntHex(0x1FFFF, pcClient->GetRegisterValue(F64R_S));

	//Over the jsr, then into it and out.
	AssertTrue(pcClient->Step(3));
	TestDebugWaitForStop(psLink, DS_Step, 0x1002);
	AssertTrue(pcClient->StepOver());
	TestDebugWaitForStop(psLink, DS_Step, uiAddY);
	AssertLongLongIntHex(12, pcClient->GetRegisterValue(F64R_X));
	AssertTrue(pcClient->Step(4));
	TestDebugWaitForStop(psLink, DS_Step, 0x1100);
	AssertTrue(pcClient->StepOut());
	TestDebugWaitForStop(psLink, DS_Step, uiAddY);
	AssertLongLongIntHex(15, pcClient->GetRegisterValue(F64R_X));
	AssertLongLongIntHex(0x1FFFF, pcClient->GetRegisterValue(F64R_S));

	//A breakpoint at a 32 bit address past the memory is kept and never hit.
	AssertTrue(pcClient->AddBreakpoint(0xFFFF0000, true));
	AssertTrue(pcClient->Continue());
	TestDebugWaitForStop(psLink, DS_Breakpoint, uiAddY);
	AssertTrue(cTarget.GetBreakpoints()->Contains(0xFFFF0000));
	AssertSize(0, pcClient->NumErrors());

	AssertTrue(pcClient->Quit());
	TestDebugWait(psLink, &CDebugClient::NumErrors, 1000);
	AssertTrue(psLink->cServer.IsQuitting());

	TestDebugLinkKill(psLink, eTransport);
	cTarget.Kill();
	free(pcProgram);
}


//////////////////////////////////////////////////////////////////////////
//
// A message before DM_Hello, and a version the server doesn't speak.
//
//////////////////////////////////////////////////////////////////////////
static void TestDebugServerHello(void)
{
	CFlat65C816DebugTarget	cTarget;
	STestDebugLink			sLink;
	STestDebugLink*			psLink;
	CDebugClient*			pcClient;
	SDebugHello				sHello;
	uint32					uiRequest;
	struct
	{
		SDebugHeader	sHeader;
		SDebugHello		sHello;
	}						sOldHello;

	cTarget.Init(TEST_DEBUG_816_MEMORY);
	psLink = &sLink;
	AssertTrue(TestDebugLinkInit(psLink, TDT_SharedMemory, &cTarget));
	pcClient = &psLink->cClient;

	uiRequest = pcClient->Continue();
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumErrors, 1));
	AssertInt(DM_Continue, pcClient->GetErrorType());
	AssertInt(uiRequest, pcClient->GetErrorRequest());
	AssertFalse(pcClient->IsBusy());
	AssertInt(DSS_Stopped, psLink->cServer.GetState());

	sHello.uiVersion = DEBUG_PROTOCOL_VERSION + 1;
	sOldHello.sHeader.uiRequest = 99;
	sOldHello.sHeader.uiReserved = 0;
	sOldHello.sHello.uiVersion = 1;
	AssertTrue(psLink->pcClient->Send(DM_Hello, &sOldHello, sizeof(sOldHello)));
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumErrors, 2));
	AssertInt(DM_Hello, pcClient->GetErrorType());
	AssertInt(99, pcClient->GetErrorRequest());
	AssertTrue(psLink->pcClient->Send(DM_Hello, &sHello, sizeof(sHello)));
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumErrors, 3));
	AssertString("Message too short.", pcClient->GetError());
	AssertFalse(psLink->cServer.IsConnected());

	AssertTrue(pcClient->Hello());
	TestDebugWaitForStop(psLink, DS_Start, 0);
	AssertTrue(psLink->cServer.IsConnected());

	TestDebugLinkKill(psLink, TDT_SharedMemory);
	cTarget.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A server that starts running, with no debugger: it runs the target until
// one is attached, which is told it is running.  After it detaches another
// can attach.
//
//////////////////////////////////////////////////////////////////////////
static void TestDebugServerAttach(void)
{
	CFlat65C816DebugTarget	cTarget;
	STestDebugLink			sLink;
	STestDebugLink*			psLink;
	CSharedMemoryChannel	cServerShared;
	CSharedMemoryChannel	cClientShared;
	uint64					uiX;
	int						i;
	static uint8			auiMain[] = { 0xE8, 0xE8, 0x20, 0x00, 0x20, 0xC8, 0x80, 0xF8 };
	static uint8			auiFunction[] = { 0xE8, 0x48, 0x68, 0x60 };

	cTarget.Init(TEST_DEBUG_816_MEMORY);
	memcpy(&cTarget.GetMemory()[0x1000], auiMain, sizeof(auiMain));
	memcpy(&cTarget.GetMemory()[0x2000], auiFunction, sizeof(auiFunction));
	cTarget.SetProgramAddress(0x1000);
	psLink = &sLink;
	AssertTrue(TestDebugLinkInit(psLink, TDT_SharedMemory, &cTarget));

	//Running, with no channel.
	psLink->cServer.Kill();
	psLink->cServer.Init(NULL, &cTarget, false);
	for (i = 0; i < 10; i++)
	{
		psLink->cServer.Tick(TEST_DEBUG_BUDGET);
	}
	AssertInt(DSS_Running, psLink->cServer.GetState());
	uiX = cTarget.GetMPU()->NumCycles();
	AssertTrue(uiX > 1000);
	AssertFalse(psLink->cServer.IsConnected());

	//Attached while it runs, then paused somewhere in the loop.
	psLink->cServer.Attach(psLink->pcServer);
	AssertTrue(psLink->cClient.Hello());
	TestDebugWaitForRunning(psLink);
	AssertString("W65C816", psLink->cClient.GetProcessorName());
	AssertTrue(cTarget.GetMPU()->NumCycles() > uiX);
	AssertTrue(psLink->cClient.Pause());
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumStops, 1));
	AssertInt(DS_Pause, psLink->cClient.GetStopReason());

	//Detached, it runs on.
	AssertTrue(psLink->cClient.Detach());
	for (i = 0; (i < TEST_DEBUG_TIMEOUT) && !psLink->cServer.IsClosed(); i++)
	{
		psLink->cServer.Tick(TEST_DEBUG_BUDGET);
	}
	AssertTrue(psLink->cServer.IsClosed());
	AssertInt(DSS_Running, psLink->cServer.GetState());

	//Another debugger attaches on a new channel.
	psLink->cClient.Kill();
	cServerShared.Init();
	cClientShared.Init();
	AssertTrue(cServerShared.Create("TestDebugAttach"));
	AssertTrue(cClientShared.Connect("TestDebugAttach"));
	psLink->cServer.Attach(&cServerShared);
	psLink->cClient.Init(&cClientShared);
	AssertFalse(psLink->cServer.IsClosed());
	AssertTrue(psLink->cClient.Hello());
	TestDebugWaitForRunning(psLink);
	AssertTrue(psLink->cClient.Pause());
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumStops, 1));
	AssertInt(DS_Pause, psLink->cClient.GetStopReason());
	AssertTrue(psLink->cServer.IsConnected());

	psLink->cClient.Kill();
	psLink->cClient.Init(psLink->pcClient);
	TestDebugLinkKill(psLink, TDT_SharedMemory);
	cClientShared.Kill();
	cServerShared.Kill();
	cTarget.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDebugServer(void)
{
	BeginTests();

	TestDebugServerHello();
	TestDebugServer65C816(TDT_SharedMemory);
	TestDebugServer65C816(TDT_Network);
	TestDebugServer65C64(TDT_SharedMemory);
	TestDebugServer65C64(TDT_Network);
	TestDebugServerAttach();

	TestStatistics();
}
