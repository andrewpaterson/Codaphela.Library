#ifndef __TILE_GRID_SOURCE_H__
#define __TILE_GRID_SOURCE_H__
#include "BaseLib/SizeVec2.h"
#include "BaseLib/Chars.h"
#include "StandardLib/Object.h"


class CTileColourSource;
class CTileGridSource : public CObject
{
CONSTRUCTABLE(CTileGridSource);
DESTRUCTABLE(CTileGridSource);
protected:
	char*			mszConstantName;

public:
			void		Init(char* szConstantName);
			void		Free(void);
			void		Class(void);

			bool		Load(CObjectReader* pcFile);
			bool		Save(CObjectWriter* pcFile);

	virtual	bool		StartGeneration(void) =0;
	virtual	void		StopGeneration(void) =0;

			bool		IsNamed(char* szConstantName);
			char*		GetConstantName(void);

	virtual	SSizeVec2	GetSize(void) =0;
	virtual	bool		Matches(CTileColourSource* pcTilePointSource, int x, int y) =0;
};


#endif // __TILE_GRID_SOURCE_H__

