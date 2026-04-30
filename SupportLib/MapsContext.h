#ifndef __MAPS_CONTEXT_H__
#define __MAPS_CONTEXT_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/ObjectWriter.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ClassDefines.h"
#include "Image.h"
#include "ImageCelGroup.h"
#include "TileMap.h"
#include "SpriteType.h"
#include "MovableBlockType.h"


class CMapsContext : public CObject
{
CONSTRUCTABLE(CMapsContext);
DESTRUCTABLE(CMapsContext);
protected:
	CArrayMovableBlockType	maBlockTypes;  
	CArraySpriteType		maSpriteTypes;
	CArrayImage				maImages;
	CArrayImageCelGroup		maGroups;
	
public:
	void 							Init(void);
	void 							Free(void);
	void							Class(void);

	bool							Save(CObjectWriter* pcFile);
	bool							Load(CObjectReader* pcFile);

	Ptr<CMovableBlockType>			AddBlockType(char* szTypeName = NULL);
	Ptr<CArrayMovableBlockType>		GetBlockTypes(void);

	Ptr<CSpriteType>				AddSpriteType(char* szTypeName = NULL);

	void							AddImages(Ptr<CArrayImage> paImages);
	void							AddGroup(Ptr<CImageCelGroup> pGroup);
	Ptr<CImageCelGroup>				GetGroup(char* szName);
};


#endif // __MAPS_CONTEXT_H__

