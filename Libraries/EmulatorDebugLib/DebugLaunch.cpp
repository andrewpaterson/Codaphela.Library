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
#include <string.h>
#include "DebugLaunch.h"


//////////////////////////////////////////////////////////////////////////
//
// argv[0] is the program, so it is never an argument.
//
//////////////////////////////////////////////////////////////////////////
const char* DebugLaunchValue(int argc, char* argv[], const char* szArgument)
{
	int		i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], szArgument) == 0)
		{
			return (i + 1 < argc) ? argv[i + 1] : "";
		}
	}
	return NULL;
}

