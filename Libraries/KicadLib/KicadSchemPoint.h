/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadLib

Codaphela KicadLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __KICAD_SCHEM_POINT_H__
#define __KICAD_SCHEM_POINT_H__
#include "BaseLib/Float64Vec2.h"


//KiCad's schematic coordinates are millimetres, held as SFloat64Vec2.  On a sheet Y increases downwards; inside a
//library symbol Y increases upwards (see CKicadSchemTransform).  Items are normally on a 1.27mm (50 mil) grid.  Two
//points closer than the tolerance are the same point (see SFloat64Vec2::CloselyEqual).
#define KICAD_SCHEM_POINT_TOLERANCE		0.0005


#endif // __KICAD_SCHEM_POINT_H__

