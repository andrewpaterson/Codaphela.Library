#ifndef __SPRITE_H__
#define __SPRITE_H__
#include "inttypes.h"

#define SFT_TYPE_OPAQUE			(uint16_t)0xe457
#define SFT_TYPE_TRANSPARENT	(uint16_t)0xe4f9
#define SFT_TYPE_CONTAINER		(uint16_t)0xe4ef


struct SSpriteHeader
{
	uint16_t	uiType;
	uint16_t	uiImageWidth;
	uint16_t	uiImageHeight;
};


struct SSpriteRowRun
{
	uint16_t	uiFlags;  //0x8000: Start of run flag.  //0x7fff: run skip mask.  or how many pixels to add before the current run starts drawing
	uint16_t	uiRunLength;
};


struct SSpriteContainer
{
	uint16_t	uiType;
	uint16_t	uiCount;  //Count of contained images.
};


struct SSpriteNext
{
	uint16_t	uiSkipToNext;  //How many bytes to seek forward to the next contained (or zero if no more images contained).  Relative to the end of this struct.
};  //end of struct == start of SSFT image type.


struct SSpriteIterator
{
	void*	pvData;
	
};


uint16_t StartSpritesIteration(void* pvData, struct SSpriteIterator* psIter);
struct SSpriteHeader* SpritesIterate(struct SSpriteIterator* psIter);
void DrawSprite(int16_t x, int16_t y, struct SSpriteHeader* psSprite);


#endif // __SPRITE_H__

