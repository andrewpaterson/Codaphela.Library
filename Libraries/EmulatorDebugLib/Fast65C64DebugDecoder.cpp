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
#include <string.h>
#include "MicroprocessorLib/Fast65C64.h"
#include "Fast65C64DebugDecoder.h"


//////////////////////////////////////////////////////////////////////////
//
// ".word $1234, $5678": the instruction's words, little endian.
//
//////////////////////////////////////////////////////////////////////////
size CFast65C64DebugDecoder::Decode(CDebugTarget* pcTarget, uint32 uiAddress, size uiCount, SDebugInstruction* asDest)
{
	size				i;
	size				j;
	uint16				uiOpcode;
	size				uiLength;
	SDebugInstruction*	psInstruction;
	char				szWord[16];

	for (i = 0; i < uiCount; i++)
	{
		if (pcTarget->PeekMemory(uiAddress, &uiOpcode, 2) != 2)
		{
			break;
		}
		uiLength = CFast65C64::InstructionLength(uiOpcode);
		psInstruction = &asDest[i];
		StartDebugInstruction(psInstruction, pcTarget, uiAddress, uiLength);
		if (CFast65C64::CallLength(uiOpcode) != 0)
		{
			psInstruction->uiKind = DI_Call;
		}
		else if (CFast65C64::IsReturn(uiOpcode))
		{
			psInstruction->uiKind = DI_Return;
		}

		strcpy(psInstruction->szText, ".word ");
		for (j = 0; (j + 1 < psInstruction->uiLength) && (strlen(psInstruction->szText) + 8 < DEBUG_INSTRUCTION_TEXT); j += 2)
		{
			sprintf(szWord, "%s$%02X%02X", (j == 0) ? "" : ", ", psInstruction->auiBytes[j + 1], psInstruction->auiBytes[j]);
			strcat(psInstruction->szText, szWord);
		}
		uiAddress += (uint32)uiLength;
	}
	return i;
}
