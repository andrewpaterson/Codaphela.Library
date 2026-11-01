#include "stdlib.h"
#include "inttypes.h"
#include "Image.h"
#include "Sprite.h"


//Tide/Video's MainSprites.c without the font, heap and IRQ: the test puts Graphics.sft (a container of the background
//and seven sprites of Maki standing and walking) at SFT_ADDRESS and reads the 320 x 200 screen at SCREEN_ADDRESS.
#define SFT_ADDRESS		0x080000
#define SCREEN_ADDRESS	0x110000
#define RESULTS_ADDRESS	0x100000


//Draws the background at 0, 0, Maki standing at 70, 84 (as MainSprites.c) and the walking sprites along the bottom,
//the first partly off the left edge and the last off the right.  Returns the number of sprites in the container
//(and writes it and a hash of the screen at RESULTS_ADDRESS).
int main()
{
	static const int16_t	aiWalkX[6] = { -20, 30, 80, 130, 180, 290 };
	struct SSpriteIterator	sIter;
	struct SSpriteHeader*	psSprite;
	struct SSpriteHeader*	apsSprites[8];
	uint16_t				uiCount;
	uint16_t				i;
	uint8_t*				puiScreen;
	uint32_t				uiHash;
	uint32_t				j;

	SetImageParameters((uint8_t*)SCREEN_ADDRESS, 320, 200);

	uiCount = StartSpritesIteration((void*)SFT_ADDRESS, &sIter);
	for (i = 0; i < 8; i++)
	{
		psSprite = SpritesIterate(&sIter);
		apsSprites[i] = psSprite;
		if (psSprite == NULL)
		{
			return -1;
		}
	}

	DrawSprite(0, 0, apsSprites[0]);
	DrawSprite(70, 84, apsSprites[1]);
	for (i = 0; i < 6; i++)
	{
		DrawSprite(aiWalkX[i], 105, apsSprites[i + 2]);
	}

	puiScreen = GetImageMemory();
	uiHash = 0;
	for (j = 0; j < (uint32_t)GetImageWidth() * GetImageHeight(); j++)
	{
		uiHash = uiHash * 31 + puiScreen[j];
	}
	*((uint32_t*)RESULTS_ADDRESS) = uiHash;
	*((uint32_t*)(RESULTS_ADDRESS + 4)) = uiCount;
	return uiCount;
}
