#ifndef __SPRITE_H__
#define __SPRITE_H__
#include "BaseLib/Int32Vec2.h"
#include "StandardLib/Object.h"
#include "StandardLib/Array.h"
#include "ImageBlitter.h"
#include "ImageCelBlitterCache.h"
#include "ImageCel.h"


class CSprite : public CObject
{
CONSTRUCTABLE(CSprite);
DESTRUCTABLE(CSprite);
protected:
	SInt32Vec2			msPosition;
	Ptr<CImageCel>		mpCel;
	Ptr<CImageBlitter>	mpBlitter;

public:
	void			Init(Ptr<CImageCel> pCel, int32 x, int32 y);
	void 			Free(void) override;
	void			Class(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	Ptr<CImageCel>	GetCel(void);
	void			GetImageDestBounds(CInt32Rect* pcReturn);

	int32			GetX(void);
	int32			GetY(void);

	bool			CreateBlitter(Ptr<CImageCelBlitterCache> pBlitterCache);
	void			ClearBlitter(void);
	bool			Blit(int32 x, int32 y);
	bool			HasBlitter(void);
};


typedef CArray<CSprite>		CArraySprite;


#endif // __SPRITE_H__

