#ifndef __SPRITE_H__
#define __SPRITE_H__
#include "ImageCel.h"
#include "BaseSprite.h"


class CSprite : public CBaseSprite
{
CONSTRUCTABLE(CSprite);
DESTRUCTABLE(CSprite);
protected:
	Ptr<CImageCel>	mpCel;

public:
	void			Init(Ptr<CImageCel> pCel, int32 x, int32 y);
	void 			Free(void);
	void			Class(void);

	bool			Save(CObjectWriter* pcFile);
	bool			Load(CObjectReader* pcFile);

	Ptr<CImageCel>	GetCel(void);
	void			BaseSpriteAbstract(void);
};


#endif // __SPRITE_H__

