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
#ifndef __EMULATOR_DEBUGGER_H__
#define __EMULATOR_DEBUGGER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ListenSocket.h"
#include "ThreadLib/NetworkConnection.h"
#include "ThreadLib/SharedMemoryChannel.h"
#include "DebugServer.h"
#include "DebugLaunch.h"


enum EEmulatorDebugTransport
{
	EDT_None,
	EDT_SharedMemory,
	EDT_Network,
};


//An emulator's end of a debugger's connection: the channel (made from the command line's DebugLaunch.h arguments, shared
//with whatever starts the emulator) and the server, for whatever the emulator debugs (a CDebugTarget, Start's).  The
//target runs from the start and a debugger can attach whenever it likes, or with DEBUG_ARGUMENT_WAIT it holds stopped
//before its first instruction until a debugger attaches and continues it.  On the network another debugger can attach
//after one detaches.
class CEmulatorDebugger
{
protected:
	EEmulatorDebugTransport	meTransport;
	CChars					mszName;
	int						miPort;
	bool					mbExitOnDetach;
	bool					mbWait;
	CListenSocket			mcListen;
	CNetworkConnection		mcNetwork;
	CSharedMemoryChannel	mcShared;
	bool					mbStarted;
	bool					mbChannelOpen;		//Created (shared memory) or accepted (network): the server can read it.
	CDebugServer			mcServer;

public:
	void	Init(void);
	void	Kill(void);

	bool	ParseArguments(int argc, char* argv[]);
	bool	IsEnabled(void) const;
	bool	IsWaiting(void) const;
	bool	Start(CDebugTarget* pcTarget);
	bool	Tick(uint64 uiBudget);
	bool	IsRunning(void) const;
	bool	IsAttached(void) const;
	void	Reset(void);

protected:
	void	Accept(void);
};


#endif // __EMULATOR_DEBUGGER_H__

