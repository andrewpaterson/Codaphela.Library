/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __MAPS_CONTEXT_H__
#define __MAPS_CONTEXT_H__
#include "StandardLib/Unknown.h"
#include "StandardLib/Pointer.h"
#include "Image.h"
#include "ImageCelGroup.h"
#include "TileMap.h"
#include "SpriteType.h"
#include "MovableBlockType.h"


class CMapsContext : public CUnknown
{
CONSTRUCTABLE(CMapsContext);
protected:
	CArrayMovableBlockType	macBlockTypes;  
	CArraySpriteType		macSpriteTypes;
	CArrayImage				macImages;
	CArray<CImageCelGroup>	macGroups;
	
public:
	void 						Init(void);
	void 						Kill(void);

	CMovableBlockType*			AddBlockType(char* szTypeName);
	CMovableBlockType*			GetBlockType(char* szTypeName);
	CArrayMovableBlockType*		GetBlockTypes(void);

	CSpriteType*				AddSpriteType(char* szTypeName);
	CSpriteType*				GetSpriteType(char* szTypeName);

	void						AddImages(Ptr<CArrayImage> pacImages);
	void						AddGroup(Ptr<CImageCelGroup> pcGroup);
	Ptr<CImageCelGroup>			GetGroup(char* szName);
};


#endif // __MAPS_CONTEXT_H__

