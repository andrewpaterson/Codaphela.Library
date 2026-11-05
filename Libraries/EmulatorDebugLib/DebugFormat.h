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
#ifndef __DEBUG_FORMAT_H__
#define __DEBUG_FORMAT_H__
#include "BaseLib/Chars.h"
#include "DebugTarget.h"


//Text for what an emulator describes, the same in every debugger: an address as its welcome says to write it
//("$12:3456" or "$00001234"), and a register's value with its flags' letters ("$34 nvMXdIzc").
void AppendDebugAddress(CChars* psz, uint32 uiAddress, EDebugAddressFormat eFormat, size uiAddressBits);
void AppendDebugRegisterValue(CChars* psz, SDebugRegister* psRegister, uint64 uiValue);


#endif // __DEBUG_FORMAT_H__
