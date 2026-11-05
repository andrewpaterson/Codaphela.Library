/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "DiffViewColours.h"


//////////////////////////////////////////////////////////////////////////
//
// IntelliJ IDEA's dark theme: inserted green, deleted grey, modified blue.
//
//////////////////////////////////////////////////////////////////////////
void SDiffViewColours::Init(void)
{
	asLines[TDBT_Inserted] = Set32BitColour((uint8)0x29, (uint8)0x44, (uint8)0x36);
	asLines[TDBT_Deleted] = Set32BitColour((uint8)0x48, (uint8)0x4A, (uint8)0x4A);
	asLines[TDBT_Modified] = Set32BitColour((uint8)0x38, (uint8)0x55, (uint8)0x70);
	asChanges[TDBT_Inserted] = Set32BitColour((uint8)0x44, (uint8)0x71, (uint8)0x52);
	asChanges[TDBT_Deleted] = Set32BitColour((uint8)0x65, (uint8)0x6E, (uint8)0x76);
	asChanges[TDBT_Modified] = Set32BitColour((uint8)0x43, (uint8)0x69, (uint8)0x8D);
	asShapes[TDBT_Inserted] = asLines[TDBT_Inserted];
	asShapes[TDBT_Deleted] = asLines[TDBT_Deleted];
	asShapes[TDBT_Modified] = asLines[TDBT_Modified];
	asOutlines[TDBT_Inserted] = asChanges[TDBT_Inserted];
	asOutlines[TDBT_Deleted] = asChanges[TDBT_Deleted];
	asOutlines[TDBT_Modified] = asChanges[TDBT_Modified];
	sDivider = Set32BitColour((uint8)0x1E, (uint8)0x1F, (uint8)0x22);
}


ARGB32 SDiffViewColours::GetLines(ETextDiffBlockType eType) { return asLines[eType]; }
ARGB32 SDiffViewColours::GetChanges(ETextDiffBlockType eType) { return asChanges[eType]; }
ARGB32 SDiffViewColours::GetShape(ETextDiffBlockType eType) { return asShapes[eType]; }
ARGB32 SDiffViewColours::GetOutline(ETextDiffBlockType eType) { return asOutlines[eType]; }

