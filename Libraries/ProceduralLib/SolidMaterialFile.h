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
#ifndef __SOLID_MATERIAL_FILE_H__
#define __SOLID_MATERIAL_FILE_H__
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "SupportLib/SolidTexture.h"


#define SOLID_MATERIAL_NAME		"Material"	//What the object a material's file holds is called.


//A material as a file holds it: the solid texture at the top of its graph, and so all the graph.  A file's objects are
//found by name, and a texture has none, so this is the named one.
class CSolidMaterial : public CObject
{
CONSTRUCTABLE(CSolidMaterial);
DESTRUCTABLE(CSolidMaterial);
public:
	Ptr<CSolidTexture>	mpSurface;

	void	Init(Ptr<CSolidTexture> pSurface);
	void	Class(void) override;
	void	Free(void) override;
};


//A material as a configuration (TexturePlan.txt, step 39): the graph of solid textures pSurface is the top of, written to
//a file and read back as other textures that sample as they did.  The file is a chunk file of the objects, szFilename
//with the object files' extension (DRG) whether it's given or not.  Read gives NULL if there's no such file or it isn't a
//material's.  AddSolidTextureConstructors tells the objects how to make each class of solid texture a file may hold;
//Read calls it.
bool				WriteSolidMaterial(Ptr<CSolidTexture> pSurface, const char* szFilename);
Ptr<CSolidTexture>	ReadSolidMaterial(const char* szFilename);
void				AddSolidTextureConstructors(void);


#endif // __SOLID_MATERIAL_FILE_H__

