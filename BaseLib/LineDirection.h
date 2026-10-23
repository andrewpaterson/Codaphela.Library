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
#ifndef __LINE_DIRECTION_H__
#define __LINE_DIRECTION_H__
#include "PrimitiveTypes.h"


//The direction a line runs from its start to its end, as Logicim's Rotation names them: increasing y is North and
//increasing x is West.  Cannot is a line that isn't horizontal or vertical, or has no length.
enum ELineDirection
{
	LD_Cannot,
	LD_North,
	LD_East,
	LD_South,
	LD_West,
};


ELineDirection	CalculateLineDirection(int32 iStartX, int32 iStartY, int32 iEndX, int32 iEndY);


#endif // __LINE_DIRECTION_H__

