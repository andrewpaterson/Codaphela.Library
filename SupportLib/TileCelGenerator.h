#ifndef __TILE_CEL_GENERATOR_H__
#define __TILE_CEL_GENERATOR_H__
#include "BaseLib/Chars.h"
#include "Image.h"
#include "Image.h"
#include "TileLayer.h"
#include "TileGridSource.h"
#include "TileColourSource.h"


class CTileCelGenerator : public CObject
{
CONSTRUCTABLE(CTileCelGenerator);
DESTRUCTABLE(CTileCelGenerator);
protected:
	Ptr<CTileGridSource>	mpTileGridSource;
	Ptr<CTileLayer>			mpTileLayer;
	int						miCelType;
	CTileColourSource*		mpcTilePointSource;

public:
	void					Init(Ptr<CTileGridSource> pTileGridSource, Ptr<CTileLayer> pTileLayer, int iCelType, CTileColourSource* pcTilePointSource);
	void					Free(void);

	void					Class(void);

	bool					Load(CObjectReader* pcFile);
	bool					Save(CObjectWriter* pcFile);

	Ptr<CTileGridSource>	GetTileGridSource(void);
	Ptr<CTileLayer>			GetTileLayer(void);
	int						GetCelType(void);
	CTileColourSource*		GeTileColourSource(void);

	bool					Matches(int x, int y);
};


#endif // __TILE_CEL_TYPE_H__

