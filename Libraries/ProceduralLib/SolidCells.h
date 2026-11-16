/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ProceduralLib

Codaphela ProceduralLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ProceduralLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ProceduralLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __SOLID_CELLS_H__
#define __SOLID_CELLS_H__
#include "SupportLib/SolidTexture.h"


//Which of cellular noise's numbers a CSolidCells gives.
enum ESolidCells
{
	SC_Nearest,	//How far the nearest point is: dark at each cell's middle, lighter to its edge.  Pebbles, scales.
	SC_Edge,	//How much further the second nearest is: 0 along the cells' edges, rising inside.  Cracks, mortar.
	SC_Cell,	//The cell's own number, 0 to 1, the same all over it: a colour for each stone.
};


//Cellular noise as a value (TexturePlan.txt, Operators; BaseLib's Noise.h): space cut into cells about one of the texture's
//units across, one round a point somewhere in each lattice cube.  Cells finer than the texel are their mean.
class CSolidCells : public CSolidTexture
{
CONSTRUCTABLE(CSolidCells);
DESTRUCTABLE(CSolidCells);
public:
	ESolidCells	meKind;
	uint32		muiSeed;

	void	Init(ESolidCells eKind, uint32 uiSeed);
	void	Class(void) override;

protected:
	void	TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample) override;
};


#endif // __SOLID_CELLS_H__

