#ifndef __TILE_GRID_SOURCE_H__
#define __TILE_GRID_SOURCE_H__
#include "BaseLib/Chars.h"
#include "Image.h"


class CTileGridSource : public CObject
{
CONSTRUCTABLE(CTileGridSource);
DESTRUCTABLE(CTileGridSource);
protected:
	char*			mszConstantName;
	Ptr<CImage>		mpImage;

public:
	void	Init(char* szConstantName, Ptr<CImage> pImage);
	void	Free(void);
	void	Class(void);

	bool	Load(CObjectReader* pcFile);
	bool	Save(CObjectWriter* pcFile);

	bool	IsNamed(char* szConstantName);
	char*	GetConstantName(void);
};


#endif // __TILE_GRID_SOURCE_H__

