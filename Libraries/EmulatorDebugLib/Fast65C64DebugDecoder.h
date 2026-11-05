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
#ifndef __FAST_65C64_DEBUG_DECODER_H__
#define __FAST_65C64_DEBUG_DECODER_H__
#include "DebugTarget.h"


//A 65C64's instructions from MicroprocessorLib's decoding of them (CFast65C64): their lengths, calls and returns.
//There is no 65C64 disassembler yet, so the text is the instruction's words.
class CFast65C64DebugDecoder : public CDebugDecoder
{
public:
	size	Decode(CDebugTarget* pcTarget, uint32 uiAddress, size uiCount, SDebugInstruction* asDest) override;
};


#endif // __FAST_65C64_DEBUG_DECODER_H__
