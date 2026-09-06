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

public:
	void	Init(Ptr<CTileGridSource>, Ptr<CTileLayer>, int iCelType, CTileColourSource* pcSource);
	void	Free(void);

	void	Class(void);

	bool	Load(CObjectReader* pcFile);
	bool	Save(CObjectWriter* pcFile);
};


#endif // __TILE_CEL_TYPE_H__

