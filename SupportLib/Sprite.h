#ifndef __SPRITE_H__
#define __SPRITE_H__
#include "BaseLib/Int2.h"
#include "StandardLib/Object.h"
#include "StandardLib/Array.h"
#include "SpriteType.h"


class CSprite : public CObject
{
CONSTRUCTABLE(CSprite);
DESTRUCTABLE(CSprite);
protected:
	Ptr<CSpriteType>	mpType;
	SInt2				msPosition;

public:
	void				Init(Ptr<CSpriteType> pType, int32 x, int32 y);
	void 				Free(void);
	void				Class(void);

	bool				Save(CObjectWriter* pcFile);
	bool				Load(CObjectReader* pcFile);

	Ptr<CSpriteType>	GetType(void);
	SInt2*				GetPosition(void);
};


typedef CArray<CSprite>	CArraySprite;


#endif // __SPRITE_H__

