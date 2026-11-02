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
#include "Insets.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SInsets::Init(void)
{
	aiWidths[0] = 0;
	aiWidths[1] = 0;
	aiWidths[2] = 0;
	aiWidths[3] = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SInsets::Init(int* paiWidths)
{
	aiWidths[0] = paiWidths[0];
	aiWidths[1] = paiWidths[1];
	aiWidths[2] = paiWidths[2];
	aiWidths[3] = paiWidths[3];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int SInsets::GetLeftAndRight(void)
{
	return aiWidths[BEI_Left] + aiWidths[BEI_Right];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int SInsets::GetTopAndBottom(void)
{
	return aiWidths[BEI_Top] + aiWidths[BEI_Bottom];
}


//////////////////////////////////////////////////////////////////////////
//
// Moves an area in by the insets.  Its size never goes below zero.
//
//////////////////////////////////////////////////////////////////////////
void SInsets::Inset(SInt32Vec2* psPosition, SInt32Vec2* psSize)
{
	psPosition->x += aiWidths[BEI_Left];
	psPosition->y += aiWidths[BEI_Top];
	psSize->x -= GetLeftAndRight();
	psSize->y -= GetTopAndBottom();
	if (psSize->x < 0)
	{
		psSize->x = 0;
	}
	if (psSize->y < 0)
	{
		psSize->y = 0;
	}
}

