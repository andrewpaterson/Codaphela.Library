/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include <string.h>
#include "Validation.h"


static bool			gbBreakSwitchRead = false;
static EBreakSwitch	geBreakSwitch = BSW_AsBuilt;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EBreakSwitch ParseBreakSwitch(const char* szValue)
{
	if (szValue != NULL)
	{
		if (strcmp(szValue, "0") == 0)
		{
			return BSW_Off;
		}
		if (strcmp(szValue, "1") == 0)
		{
			return BSW_On;
		}
	}
	return BSW_AsBuilt;
}


//////////////////////////////////////////////////////////////////////////
//
// BASELIB_BREAK, read the first time it is asked for.  A program for the
// HCP65 has no environment: its breaks are as built.
//
//////////////////////////////////////////////////////////////////////////
EBreakSwitch GetBreakSwitch(void)
{
	if (!gbBreakSwitchRead)
	{
#ifndef __LIBC65__
		geBreakSwitch = ParseBreakSwitch(getenv("BASELIB_BREAK"));
#endif // __LIBC65__
		gbBreakSwitchRead = true;
	}
	return geBreakSwitch;
}

