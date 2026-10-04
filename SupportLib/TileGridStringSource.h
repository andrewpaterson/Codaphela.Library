#ifndef __TILE_GRID_STRING_SOURCE_H__
#define __TILE_GRID_STRING_SOURCE_H__
#include "BaseLib/SizeVec2.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "TileGridSource.h"


class CTileGridStringSource : public CTileGridSource
{
CONSTRUCTABLE(CTileGridStringSource);
DESTRUCTABLE(CTileGridStringSource);
protected:
	CArrayChars		maszStrings;
	SSizeVec2		msSize;

public:
	void			Init(char* szConstantName, CArrayChars* paszStrings);
	void			Free(void);
	void			Class(void);

	bool			StartGeneration(void);
	void			StopGeneration(void);

	bool			Load(CObjectReader* pcFile);
	bool			Save(CObjectWriter* pcFile);

	CArrayChars*	GetStrings(void);

	bool			Matches(CTileCelSource* pcTileCelSource, int x, int y);
	SSizeVec2		GetSize(void);
};


#endif // __TILE_GRID_STRING_SOURCE_H__

