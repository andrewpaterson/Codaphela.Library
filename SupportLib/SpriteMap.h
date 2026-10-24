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
#ifndef __SPRITE_MAP_H__
#define __SPRITE_MAP_H__
#include "BaseLib/Int32Vec2.h"
#include "BaseLib/IntVec2.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/MapPrimitiveTemplate.h"
#include "StandardLib/Array.h"
#include "ImageCel.h"
#include "ImageBlitter.h"
#include "BlockMap.h"


typedef void (*MapImageCelFunction)(Ptr<CImageCel> pcImageCel, int x, int y);


//A cel drawn with its top left at a position in the map.  The cel is the map's cel at uiCel.
struct SSprite
{
	SInt32Vec2	sPosition;
	size		uiCel;
};


typedef CArrayTemplate<SSprite>					CArraySprite;
typedef CMapPrimitiveTemplate<CImageCel*, size>	CMapImageCelIndex;


//A map of cels at positions, such as the glyphs of text.  The sprites are plain data (not objects) so a map can hold
//many; each cel they draw is held once, with one blitter made for it.
class CSpriteMap : public CBlockMap
{
CONSTRUCTABLE(CSpriteMap)
DESTRUCTABLE(CSpriteMap)
protected:
	CArraySprite				masSprites;
	CArrayImageCel				maCels;			//Each cel drawn, once.
	CArrayImageBlitter			maBlitters;		//The blitter for the cel at the same index in maCels.  NULL until made.
	CMapImageCelIndex			mcCelIndices;	//A cel's index in maCels.
	Ptr<CImageCelBlitterCache>	mpCache;
	Ptr<CImage>					mpViewport;

public:
	void						Init(void);
	void 						Free(void) override;
	void						Class(void) override;

	bool						Save(CObjectWriter* pcFile) override;
	bool						Load(CObjectReader* pcFile) override;

	void						SetBlitterCache(Ptr<CImageCelBlitterCache> pCache) override;
	void						SetViewport(Ptr<CImage> pViewport) override;
	Ptr<CImage>					GetViewport(void);

	void						Clear(void);

	bool						AddSprite(Ptr<CImageCel> pCel, int32 x, int32 y);
	size						NumSprites(void);
	SSprite*					GetSprite(size uiIndex);
	size						NumCels(void);
	Ptr<CImageCel>				GetCel(size uiCel);

	void						BeginChange(void);
	void						EndChange(void);

	void						FindImageCels(CInt32Rect* pcRectangle, MapImageCelFunction pSpriteFunction);
	
	bool						GetImageDestBounds(CInt32Rect* pcRect);
	bool						GetFullDestBounds(CInt32Rect* pcRect);

	bool						Blit(CInt32Rect* pcViewportRect, bool bEnsureBlitters) override;

	Ptr<CImage>					WriteToImage(void);
	Ptr<CImage>					CreateViewportImage(void);
	Ptr<CImage>					CreateViewportImage(CInt32Rect* pcBoundingRect);
	Ptr<CImageCelBlitterCache>	CreateBlitterCache(void);

	bool						CreateCelBlitters(void) override;
	void						ClearCelBlitters(void) override;
	bool						HasCelBlitters(void) override;
	bool						UsesCelBlitters(void) override;

	void						TileMapAbstract(void) override {}

protected:
	size						AddCel(Ptr<CImageCel> pCel);
	bool						CreateCelBlitter(size uiCel);
	bool						BlitSprites(int32 iLeft, int32 iTop, bool bEnsureBlitters);
};


typedef CArray<CSpriteMap>	CArraySpriteMap;


#endif // __SPRITE_MAP_H__

