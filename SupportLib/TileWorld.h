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
#ifndef __TILE_WORLD_H__
#define __TILE_WORLD_H__
#include "StandardLib/Unknown.h"
#include "StandardLib/Pointer.h"
#include "Image.h"
#include "ImageCelGroup.h"
#include "TileMap.h"
#include "TileType.h"


class CWorld;
class CTileWorld : public CUnknown
{
CONSTRUCTABLE(CTileWorld);
public:
	CArrayTileMap			macMaps;
	CArrayTileType			macTileTypes;
	CTileType*				mpcImageType;
	CArray<CImage>			macImages;
	CArray<CImageCelGroup>	macGroups;
	
	void 						Init(void);
	void 						Kill(void);

	CTileType*					AddType(char* szTypeName);
	CTileMap*					AddMap(char* szName, int iCelWidth, int iCelHeight);
	CTileType*					GetType(char* szTypeName);

	void						AddImages(Ptr<CArray<CImage>> pacImages);
	void						AddGroup(Ptr<CImageCelGroup> pcGroup);
	Ptr<CImageCelGroup>	GetGroup(char* szName);
};


#endif // __TILE_WORLD_H__

