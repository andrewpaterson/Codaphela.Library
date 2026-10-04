#ifndef __TILE_CEL_GENERATOR_H__
#define __TILE_CEL_GENERATOR_H__
#include "BaseLib/Chars.h"
#include "Image.h"
#include "Image.h"
#include "TileLayer.h"
#include "TileGridSource.h"
#include "TileCelColourSource.h"


class CTileCelGenerator : public CObject
{
CONSTRUCTABLE(CTileCelGenerator);
DESTRUCTABLE(CTileCelGenerator);
protected:
	Ptr<CTileGridSource>	mpTileGridSource;
	Ptr<CTileLayer>			mpTileLayer;
	int						miCelType;
	CTileCelSource*			mpcTileCelSource;

public:
	void					Init(Ptr<CTileGridSource> pTileGridSource, Ptr<CTileLayer> pTileLayer, int iCelType, CTileCelSource* pcTileCelSource);
	void					Free(void);

	void					Class(void);

	bool					Load(CObjectReader* pcFile);
	bool					Save(CObjectWriter* pcFile);

	Ptr<CTileGridSource>	GetTileGridSource(void);
	Ptr<CTileLayer>			GetTileLayer(void);
	int						GetCelType(void);
	CTileCelSource*			GetTileColourSource(void);

	bool					Matches(int x, int y);
	bool					Matches(CTileCelSource* pcSource);
};


#endif // __TILE_CEL_TYPE_H__

