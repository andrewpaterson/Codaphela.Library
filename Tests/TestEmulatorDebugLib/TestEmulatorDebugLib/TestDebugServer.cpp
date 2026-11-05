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
	STestDebugLink			sLink;
	STestDebugLink*			psLink;
	CDebugClient*			pcClient;
	SDebugBreakpoint		sBreakpoint;
	SDebugRange				sLine;
	SDebugRange				sFunction;
	SDebugRegister*			psRegister;
	static uint8			auiMain[] = { 0xE8, 0xE8, 0x20, 0x00, 0x20, 0xC8, 0x80, 0xF8 };
	static uint8			auiFunction[] = { 0xE8, 0x48, 0x68, 0x60 };
	static uint8			auiText[] = { 'a', 'b', 'c' };
	static uint8			auiStop[] = { 0xDB };

	cTarget.Init(TEST_DEBUG_816_MEMORY);
	psLink = &sLink;
	AssertTrue(TestDebugLinkInit(psLink, eTransport, &cTarget));
	pcClient = &psLink->cClient;

	//Welcomed, and stopped from the start.
	AssertTrue(pcClient->Hello());
	TestDebugWaitForStop(psLink, DS_Start, 0);
	AssertString("W65C816", pcClient->GetProcessorName());
	AssertSize(24, pcClient->NumAddressBits());
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
	AssertTrue(pcClient->RemoveBreakpoint(0x9999));
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumErrors, 1));
	AssertInt(DM_RemoveBreakpoint, pcClient->GetErrorRequest());
	AssertString("No breakpoint there.", pcClient->GetError());

	//The breakpoints replaced by one at $1000.
	sBreakpoint.uiAddress = 0x1000;
	sBreakpoint.uiEnabled = true;
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
	AssertTrue(pcClient->Reset());
	TestDebugWaitForStop(psLink, DS_Reset, 0xEAEA);
	AssertFalse(cTarget.IsStopped());

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
	AssertString("C", pcClient->GetRegister(F64R_C)->szName);

	AssertTrue(pcClient->Load(0, pcProgram->mauiLow, TEST_65C64_LOW_SIZE));
	AssertTrue(pcClient->SetProgramAddress(0x1000));
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

	cTarget.Init(TEST_DEBUG_816_MEMORY);
	psLink = &sLink;
	AssertTrue(TestDebugLinkInit(psLink, TDT_SharedMemory, &cTarget));
	pcClient = &psLink->cClient;

	AssertTrue(pcClient->Continue());
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumErrors, 1));
	AssertInt(DM_Continue, pcClient->GetErrorRequest());
	AssertInt(DSS_Stopped, psLink->cServer.GetState());

	sHello.uiVersion = DEBUG_PROTOCOL_VERSION + 1;
	AssertTrue(psLink->pcClient->Send(DM_Hello, &sHello, sizeof(sHello)));
	AssertTrue(TestDebugWait(psLink, &CDebugClient::NumErrors, 2));
	AssertInt(DM_Hello, pcClient->GetErrorRequest());
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
