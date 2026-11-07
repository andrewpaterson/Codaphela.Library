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
#include "WindowTheme.h"
#include "DiffViewColours.h"


//////////////////////////////////////////////////////////////////////////
//
// The theme's diff colours; the shapes are filled in the lines' colours
// and outlined in the changes'.
//
//////////////////////////////////////////////////////////////////////////
void SDiffViewColours::Init(CWindowTheme* pcTheme)
{
	asLines[TDBT_Inserted] = pcTheme->GetColour(TC_DiffInsertedLines);
	asLines[TDBT_Deleted] = pcTheme->GetColour(TC_DiffDeletedLines);
	asLines[TDBT_Modified] = pcTheme->GetColour(TC_DiffModifiedLines);
	asChanges[TDBT_Inserted] = pcTheme->GetColour(TC_DiffInsertedChanges);
	asChanges[TDBT_Deleted] = pcTheme->GetColour(TC_DiffDeletedChanges);
	asChanges[TDBT_Modified] = pcTheme->GetColour(TC_DiffModifiedChanges);
	asShapes[TDBT_Inserted] = asLines[TDBT_Inserted];
	asShapes[TDBT_Deleted] = asLines[TDBT_Deleted];
	asShapes[TDBT_Modified] = asLines[TDBT_Modified];
	asOutlines[TDBT_Inserted] = asChanges[TDBT_Inserted];
	asOutlines[TDBT_Deleted] = asChanges[TDBT_Deleted];
	asOutlines[TDBT_Modified] = asChanges[TDBT_Modified];
	sDivider = pcTheme->GetColour(TC_DiffDivider);
}


ARGB32 SDiffViewColours::GetLines(ETextDiffBlockType eType) { return asLines[eType]; }
ARGB32 SDiffViewColours::GetChanges(ETextDiffBlockType eType) { return asChanges[eType]; }
ARGB32 SDiffViewColours::GetShape(ETextDiffBlockType eType) { return asShapes[eType]; }
ARGB32 SDiffViewColours::GetOutline(ETextDiffBlockType eType) { return asOutlines[eType]; }

