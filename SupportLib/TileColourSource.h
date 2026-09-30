#ifndef __TILE_COLOUR_SOURCE_H__
#define __TILE_COLOUR_SOURCE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayTemplate.h"
#include "ColourARGB32.h"


class CTileColourSource
{
protected:
	ARGB32	mARGB;

public:
	void	Init(uint8 iRed, uint8 iGreen, uint8 iBlue);
	void	Kill(void);

	ARGB32	GetColour(void);
};


typedef CArrayTemplate<CTileColourSource>	CArrayTileColourSource;


#endif // __TILE_COLOUR_SOURCE_H__

