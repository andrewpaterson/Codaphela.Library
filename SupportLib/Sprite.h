#ifndef __SPRITE_H__
#define __SPRITE_H__
#include "BaseLib/Int32Vec2.h"
#include "StandardLib/Object.h"
#include "StandardLib/Array.h"
#include "ImageCel.h"


class CSprite : public CObject
{
CONSTRUCTABLE(CSprite);
DESTRUCTABLE(CSprite);
protected:
	SInt32Vec2		msPosition;
	Ptr<CImageCel>	mpCel;

public:
	void			Init(Ptr<CImageCel> pCel, int32 x, int32 y);
	void 			Free(void);
	void			Class(void);

	bool			Save(CObjectWriter* pcFile);
	bool			Load(CObjectReader* pcFile);

	Ptr<CImageCel>	GetCel(void);
	void			GetImageDestBounds(CRectangle* pcReturn);

	int32			GetX(void);
	int32			GetY(void);
};


typedef CArray<CSprite>		CArraySprite;


#endif // __SPRITE_H__

