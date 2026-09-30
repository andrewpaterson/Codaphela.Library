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
	ARGB32					muiColour;

public:
	void					Init(Ptr<CTileGridSource> pTileGridSource, Ptr<CTileLayer> pTileLayer, int iCelType, CTileColourSource* pcSource);
	void					Free(void);

	void					Class(void);

	bool					Load(CObjectReader* pcFile);
	bool					Save(CObjectWriter* pcFile);

	Ptr<CTileGridSource>	GetTileGridSource(void);
	Ptr<CTileLayer>			GetTileLayer(void);
	int						GetCelType(void);
	bool					IsColour(ARGB32 uiColour);
};


#endif // __TILE_CEL_TYPE_H__

