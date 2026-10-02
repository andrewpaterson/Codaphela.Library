#ifndef __TILE_COLOUR_SOURCE_H__
#define __TILE_COLOUR_SOURCE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/LinkedListTemplate.h"
#include "ColourARGB32.h"


class CTileColourSource
{
protected:
	ARGB32	muiARGB;

public:
	void	Init(uint8 iRed, uint8 iGreen, uint8 iBlue);
	void	Kill(void);

	ARGB32	GetColour(void);

	bool	Matches(void* pvData);
};


typedef CArrayTemplate<CTileColourSource>		CArrayTileColourSource;
typedef CLinkedListTemplate<CTileColourSource>	CLinkedListTileColourSource;


#endif // __TILE_COLOUR_SOURCE_H__

