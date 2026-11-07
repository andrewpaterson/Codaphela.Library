/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#ifndef __INSETS_H__
#define __INSETS_H__
#include "BaseLib/Int32Vec2.h"


//BORDER_EDGE_INDEX
#define BEI_Top 0
#define BEI_Right 1
#define BEI_Bottom 2
#define BEI_Left 3


struct SInsets
{
	int aiWidths[4];

	void Init(void);
	void Init(int* paiWidths);
	void Init(int iTop, int iRight, int iBottom, int iLeft);
	void Init(int iHorizontal, int iVertical);

	int	GetLeftAndRight(void);
	int	GetTopAndBottom(void);

	void Inset(SInt32Vec2* psPosition, SInt32Vec2* psSize);
};


#endif // __INSETS_H__

