#ifndef __TILE_GRID_IMAGE_SOURCE_H__
#define __TILE_GRID_IMAGE_SOURCE_H__
#include "BaseLib/SizeVec2.h"
#include "BaseLib/Chars.h"
#include "TileGridSource.h"
#include "TileCelSource.h"
#include "Image.h"


class CTileGridImageSource : public CTileGridSource
{
CONSTRUCTABLE(CTileGridImageSource);
DESTRUCTABLE(CTileGridImageSource);
protected:
	Ptr<CImage>			mpImage;
	CImageAccessor*		mpcAccessor;

public:
	void			Init(char* szConstantName, Ptr<CImage> pImage);
	void			Free(void) override;
	void			Class(void) override;

	bool			StartGeneration(void) override;
	void			StopGeneration(void) override;

	bool			Load(CObjectReader* pcFile) override;
	bool			Save(CObjectWriter* pcFile) override;

	Ptr<CImage>		GetImage(void) const;
	ARGB32			GetColour(int x, int y);

	bool			Matches(CTileCelSource* pcTileCelSource, int x, int y) override;
	SSizeVec2		GetSize(void) override;
};


#endif // __TILE_GRID_IMAGE_SOURCE_H__

