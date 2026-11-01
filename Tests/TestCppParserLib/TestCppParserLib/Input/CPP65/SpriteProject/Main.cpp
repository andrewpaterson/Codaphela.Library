#include "stdint.h"
#include "Image.h"
#include "Sprite.h"


//A container of two sprites: a 4 x 3 opaque one, then a transparent one of three rows of runs (run flags $8000
//start a row; the low bits are the pixels skipped before the run).  Each SSpriteNext's skip is from its own start.
static uint8_t gauiSprites[] =
{
	0xef, 0xe4, 2, 0,							//SSpriteContainer: SFT_TYPE_CONTAINER, 2 images.
	2 + 6 + 12, 0,								//SSpriteNext: to the next image.
	0x57, 0xe4, 4, 0, 3, 0,						//SSpriteHeader: SFT_TYPE_OPAQUE, 4 x 3.
	1, 2, 3, 4,  5, 6, 7, 8,  9, 10, 11, 12,
	0, 0,										//SSpriteNext: the last image.
	0xf9, 0xe4, 6, 0, 3, 0,						//SSpriteHeader: SFT_TYPE_TRANSPARENT, 6 x 3.
	0x01, 0x80, 2, 0,  20, 21,					//Row 1: skip 1, 2 pixels.
	0x02, 0x00, 1, 0,  22,						//         skip 2, 1 pixel.
	0x00, 0x80, 3, 0,  23, 24, 25,				//Row 2: skip 0, 3 pixels.
	0x03, 0x80, 2, 0,  26, 27,					//Row 3: skip 3, 2 pixels.
	0, 0, 0, 0									//The end.
};


//Draws the sprites at (10, 5), (30, 5) ... then returns the number of pixels set and writes a hash of the screen at
//0010:0000.
int main()
{
	struct SSpriteIterator	sIter;
	struct SSpriteHeader*	psSprite;
	uint16_t				uiCount;
	int16_t					x;
	uint8_t*				puiScreen;
	uint32_t				uiHash;
	int32_t					i;
	int						iSet;

	puiScreen = (uint8_t*)SCREEN_ADDRESS;
	for (i = 0; i < (int32_t)SCREEN_WIDTH * SCREEN_HEIGHT; i++)
	{
		puiScreen[i] = 0;
	}

	uiCount = StartSpritesIteration(gauiSprites, &sIter);
	x = 10;
	for (psSprite = SpritesIterate(&sIter); psSprite; psSprite = SpritesIterate(&sIter))
	{
		DrawSprite(x, 5, psSprite);
		x += 20;
	}

	uiHash = 0;
	iSet = 0;
	for (i = 0; i < (int32_t)SCREEN_WIDTH * SCREEN_HEIGHT; i++)
	{
		if (puiScreen[i] != 0)
		{
			uiHash = uiHash * 31 + (uint32_t)i * puiScreen[i];
			iSet++;
		}
	}
	*((uint32_t*)0x100000) = uiHash;
	*((uint32_t*)0x100004) = uiCount;
	return iSet;
}
