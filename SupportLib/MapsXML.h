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
#ifndef __TILE_MAP_XML_H__
#define __TILE_MAP_XML_H__
#include "BaseLib/Chars.h"
#include "ImageCelsSource.h"
#include "MovableBlocks.h"
#include "Maps.h"

class CMarkupTag;
class CTileMapXML
{
protected:
	CChars				mszMapName;
	CChars				mszTexturePath;
	CMovableBlocks*		mpcMovableBlocks;
	CMaps*				mpcTileMaps;

public:
	void Init(char* szMapName, char* szTexturePath);
	void Kill(void);

	bool Import(CMovableBlocks* pcTileWorld);

protected:
	bool ImportBrushSources(CMarkupTag* pcTag);
	bool ImportObjectSources(CMarkupTag* pcTag);
	bool ImportMaps(CMarkupTag* pcTag);
	bool ImportMap(CMarkupTag* pcTag);
	bool ImportMap(CMarkupTag* pcTag, CTileMap* pcMap);
	bool ImportLayers(CMarkupTag* pcTag, CTileMap* pcMap);
	bool ImportLayer(CMarkupTag* pcTag, CTileMap* pcMap);
	bool ImportTiles(CMarkupTag* pcTag, CTileLayer* pcLayer);
};


#endif // __TILE_MAP_XML_H__

