#ifndef __COMPOUND_SPRITE_H__
#define __COMPOUND_SPRITE_H__
#include "BaseSprite.h"


class CCompoundSprite : public CBaseSprite
{
CONSTRUCTABLE(CCompoundSprite);
DESTRUCTABLE(CCompoundSprite);
protected:
	Ptr<CCompoundSpriteType>	mpType;

public:
	void						Init(Ptr<CCompoundSpriteType> pType, int32 x, int32 y);
	void 						Free(void);
	void						Class(void);

	bool						Save(CObjectWriter* pcFile);
	bool						Load(CObjectReader* pcFile);

	Ptr<CCompoundSpriteType>	GetType(void);
	void						BaseSpriteAbstract(void);
};


#endif // __COMPOUND_SPRITE_H__

