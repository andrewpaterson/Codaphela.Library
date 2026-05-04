#ifndef __BASE_SPRITE_H__
#define __BASE_SPRITE_H__
#include "BaseLib/Int32Vec2.h"
#include "StandardLib/Object.h"
#include "StandardLib/Array.h"
#include "CompoundSpriteType.h"


class CBaseSprite : public CObject
{
CONSTRUCTABLE(CBaseSprite);
DESTRUCTABLE(CBaseSprite);
protected:
	SInt32Vec2	msPosition;

public:
			void	Init(int32 x, int32 y);
			void 	Free(void);
			void	Class(void);

			bool	Save(CObjectWriter* pcFile);
			bool	Load(CObjectReader* pcFile);

			SInt32Vec2*	GetPosition(void);
	virtual	void	BaseSpriteAbstract(void) =0;
};


typedef CArray<CBaseSprite>	CArraySprite;


#endif // __BASE_SPRITE_H__

