#ifndef __SPRITE_H__
#define __SPRITE_H__
#include "BaseLib/Int2.h"
#include "StandardLib/Unknown.h"
#include "SpriteType.h"


class CSprite : public CUnknown
{
CONSTRUCTABLE(CSprite);
protected:
	CSpriteType*	mpcType;
	SInt2			msPosition;

public:
	void			Init(CSpriteType* pcType, int32 x, int32 y);
	void			Kill(void);

	CSpriteType*	GetType(void);
	SInt2*			GetPosition(void);
};


typedef CArrayUnknownTemplate<CSprite>	CArraySprite;


#endif // __SPRITE_H__

