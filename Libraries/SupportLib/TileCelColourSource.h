#ifndef __TILE_CEL_COLOUR_SOURCE_H__
#define __TILE_CEL_COLOUR_SOURCE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/LinkedListTemplate.h"
#include "BaseLib/Chars.h"
#include "ColourARGB32.h"
#include "TileCelSource.h"


class CTileCelColourSource : public CTileCelSource
{
protected:
	ARGB32	muiARGB;

public:
	void	Init(uint8 iRed, uint8 iGreen, uint8 iBlue);
	void	Kill(void) override;
	void	Print(CChars* psz) override;

	ARGB32	GetColour(void);

	bool	Matches(void* pvData) override;
};


typedef CLinkedListTemplate<CTileCelColourSource>	CLinkedListTileColourSource;


#endif // __TILE_CEL_COLOUR_SOURCE_H__

