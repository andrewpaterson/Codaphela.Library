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
#ifndef __DEBUG_LAUNCH_H__
#define __DEBUG_LAUNCH_H__


//How a debuggable emulator is started, so the emulator and whatever starts it share one definition.  The emulator
//takes one channel argument; the debugger then connects to it (the emulator is always the server).
#define DEBUG_ARGUMENT_SHARED_MEMORY	"--debug-shm"		//Then a name: the emulator Creates a shared memory channel with it.
#define DEBUG_ARGUMENT_NETWORK			"--debug-tcp"		//Then a port (0 for any): the emulator listens on the loopback network.
#define DEBUG_ARGUMENT_WAIT				"--debug-wait"		//Hold before the first instruction until a debugger continues it.
#define DEBUG_ARGUMENT_EXIT_ON_DETACH	"--exit-on-detach"	//Close when the debugger detaches or goes.
#define DEBUG_ARGUMENT_CARD				"--card"			//Then a disk image file: the card the computer boots from.

//Printed on standard output, with the port, when the emulator listens: for a person attaching to a port it chose.
#define DEBUG_PORT_LINE					"Debug port "


//The value given after szArgument ("--card", "file.img"): NULL if szArgument isn't there, "" if nothing follows it.
const char* DebugLaunchValue(int argc, char* argv[], const char* szArgument);


#endif // __DEBUG_LAUNCH_H__
