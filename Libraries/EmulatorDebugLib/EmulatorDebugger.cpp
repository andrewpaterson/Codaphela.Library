/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela EmulatorDebugLib

Codaphela EmulatorDebugLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela EmulatorDebugLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela EmulatorDebugLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BaseLib/Logger.h"
#include "BaseLib/Socket.h"
#include "EmulatorDebugger.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmulatorDebugger::Init(void)
{
	meTransport = EDT_None;
	mszName.Init();
	miPort = 0;
	mbExitOnDetach = false;
	mbWait = false;
	mcNetwork.Init();
	mcShared.Init();
	mbStarted = false;
	mbChannelOpen = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmulatorDebugger::Kill(void)
{
	if (mbStarted)
	{
		mcServer.Kill();
		if (meTransport == EDT_Network)
		{
			mcListen.Kill();
			SocketsKill();
		}
	}
	mcShared.Kill();
	mcNetwork.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The debugging arguments (DEBUG_ARGUMENT_*); others are left for the rest of
// the emulator.  False if one is malformed.
//
//////////////////////////////////////////////////////////////////////////
bool CEmulatorDebugger::ParseArguments(int argc, char* argv[])
{
	int		i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], DEBUG_ARGUMENT_SHARED_MEMORY) == 0)
		{
			if ((i + 1 >= argc) || (meTransport != EDT_None))
			{
				gcLogger.Error2(__METHOD__, " " DEBUG_ARGUMENT_SHARED_MEMORY " needs a name, and only one debug channel can be given.", NULL);
				return false;
			}
			i++;
			meTransport = EDT_SharedMemory;
			mszName.Kill();
			mszName.Init(argv[i]);
		}
		else if (strcmp(argv[i], DEBUG_ARGUMENT_NETWORK) == 0)
		{
			if ((i + 1 >= argc) || (meTransport != EDT_None))
			{
				gcLogger.Error2(__METHOD__, " " DEBUG_ARGUMENT_NETWORK " needs a port, and only one debug channel can be given.", NULL);
				return false;
			}
			i++;
			meTransport = EDT_Network;
			miPort = atoi(argv[i]);
		}
		else if (strcmp(argv[i], DEBUG_ARGUMENT_EXIT_ON_DETACH) == 0)
		{
			mbExitOnDetach = true;
		}
		else if (strcmp(argv[i], DEBUG_ARGUMENT_WAIT) == 0)
		{
			mbWait = true;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmulatorDebugger::IsEnabled(void) const
{
	return meTransport != EDT_None;
}


//////////////////////////////////////////////////////////////////////////
//
// A debugger is to continue it (--debug-wait), so it may load the
// program first.
//
//////////////////////////////////////////////////////////////////////////
bool CEmulatorDebugger::IsWaiting(void) const
{
	return IsEnabled() && mbWait;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the channel, debugging pcTarget (the caller's, Init'ed).  A
// shared memory channel is created now; the network listens now and
// accepts the debugger in a later Tick.  The server holds the target
// stopped (DEBUG_ARGUMENT_WAIT) or runs it until then.
//
//////////////////////////////////////////////////////////////////////////
bool CEmulatorDebugger::Start(CDebugTarget* pcTarget)
{
	CProcessChannel*	pcChannel;

	if (meTransport == EDT_SharedMemory)
	{
		if (!mcShared.Create(mszName.Text()))
		{
			gcLogger.Error2(__METHOD__, " Could not create the debug channel [", mszName.Text(), "].", NULL);
			return false;
		}
		mbChannelOpen = true;
		pcChannel = &mcShared;
	}
	else
	{
		SocketsInit();
		if (!mcListen.Init(NETWORK_LOOPBACK_ADDRESS, miPort))
		{
			gcLogger.Error2(__METHOD__, " Could not listen for a debugger on port [", IntToString(miPort), "].", NULL);
			SocketsKill();
			return false;
		}
		printf(DEBUG_PORT_LINE "%d\n", mcListen.GetPort());
		fflush(stdout);
		pcChannel = NULL;
	}

	mcServer.Init(pcChannel, pcTarget, mbWait);
	mbStarted = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Each tick of the emulator, with the budget the target is due (cycles or
// instructions, the target's Run's; 0 while it isn't running).  False when the emulator should close: the debugger sent
// DM_Quit, or it went and DEBUG_ARGUMENT_EXIT_ON_DETACH was given.  Without
// a debugger the server runs the target itself; on the network the next
// debugger is accepted once the last has gone.
//
//////////////////////////////////////////////////////////////////////////
bool CEmulatorDebugger::Tick(uint64 uiBudget)
{
	if (!mbChannelOpen)
	{
		Accept();
	}

	mcServer.Tick(uiBudget);
	if (mcServer.IsQuitting())
	{
		return false;
	}
	if (mbChannelOpen && mcServer.IsClosed())
	{
		if (mbExitOnDetach)
		{
			return false;
		}
		if (meTransport == EDT_Network)
		{
			mcNetwork.Kill();
			mcNetwork.Init();
			mbChannelOpen = false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the target's time goes on: not while the debugger has it stopped
// or is stepping it, nor while it waits for a debugger (DEBUG_ARGUMENT_WAIT).
//
//////////////////////////////////////////////////////////////////////////
bool CEmulatorDebugger::IsRunning(void) const
{
	return mcServer.GetState() == DSS_Running;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a debugger has the channel: the target is its to stop, not the
// emulator's to quit when it stops itself.
//
//////////////////////////////////////////////////////////////////////////
bool CEmulatorDebugger::IsAttached(void) const
{
	return mbChannelOpen && !mcServer.IsClosed();
}


//////////////////////////////////////////////////////////////////////////
//
// The emulator's user reset the target (File, Reset), not the debugger.
//
//////////////////////////////////////////////////////////////////////////
void CEmulatorDebugger::Reset(void)
{
	mcServer.ResetTarget();
}


//////////////////////////////////////////////////////////////////////////
//
// Without waiting.
//
//////////////////////////////////////////////////////////////////////////
void CEmulatorDebugger::Accept(void)
{
	if (mcNetwork.Accept(&mcListen, 0))
	{
		mbChannelOpen = true;
		mcServer.Attach(&mcNetwork);
	}
}
