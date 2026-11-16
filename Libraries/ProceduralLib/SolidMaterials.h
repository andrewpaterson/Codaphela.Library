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
#ifndef __SOLID_MATERIALS_H__
#define __SOLID_MATERIALS_H__
#include "StandardLib/Pointer.h"
#include "SupportLib/SolidTexture.h"


//Materials: graphs of solid textures (TexturePlan.txt, Operators), each made by a function and given back as its top texture.
//A material is ordinary objects, to be placed, changed, saved and loaded as any texture is; nothing here is a class.  Each is
//sized for things a unit or so across (Place its top to change that) and differs with its seed.


Ptr<CSolidTexture>	MakeSolidMarble(uint32 uiSeed);  //Pale polished stone, bands along x bent into veins.
Ptr<CSolidTexture>	MakeSolidWood(uint32 uiSeed);  //Rings round the z axis, wobbled: the grain runs along z.
Ptr<CSolidTexture>	MakeSolidStone(uint32 uiSeed);  //Rough mottled rock in patches, cracked.
Ptr<CSolidTexture>	MakeSolidRust(uint32 uiSeed);  //Bright steel with dull, raised rust spreading over it.


#endif // __SOLID_MATERIALS_H__

