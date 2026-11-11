#include "stdlib.h"
#include "stdbool.h"
#include "Image.h"
#include "Sprite.h"


uint16_t StartSpritesIteration(void* pvData, struct SSpriteIterator* psIter)
{
	uint16_t					uiType;
	uint16_t					uiCount;
	uint32_t					uiSkipToNext;
	struct SSpriteContainer*	psSpriteContainer;

	uiType = *((uint16_t*)pvData);

	if (uiType == SFT_TYPE_CONTAINER)
	{
		psSpriteContainer = (struct SSpriteContainer*)pvData;
		uiCount = psSpriteContainer->uiCount;

		psIter->pvData = (void*)((uint32_t)pvData + sizeof(struct SSpriteContainer));
		
		return uiCount;
	}
	else
	{
		psIter->pvData = NULL;
		return 0;
	}
}


struct SSpriteHeader* SpritesIterate(struct SSpriteIterator* psIter)
{
	uint8_t*					pvData;
	uint8_t*					pvNext;
	uint16_t					uiType;
	uint32_t					uiSkipToNext;
	struct SSpriteNext*			psSpriteNext;

	pvData = psIter->pvData;
	if (pvData == NULL)
	{
		return NULL;
	}
	
	uiType = *((uint16_t*)pvData);
	uiSkipToNext = 0;

	psSpriteNext = (struct SSpriteNext*)pvData;
	uiSkipToNext = psSpriteNext->uiSkipToNext;
	
	if (uiSkipToNext != 0)
	{
		psIter->pvData = (void*)((uint32_t)pvData + uiSkipToNext);
	}
	else
	{
		psIter->pvData = NULL;
	}
	
	pvData += sizeof(struct SSpriteNext);
	uiType = *((uint16_t*)pvData);
	
	switch (uiType)
	{
		case SFT_TYPE_OPAQUE:
			return (struct SSpriteHeader*)pvData;
		case SFT_TYPE_TRANSPARENT:
			return (struct SSpriteHeader*)pvData;
		default:
			return NULL;
	}
}


void DrawSprite(int16_t x, int16_t y, struct SSpriteHeader* psSprite)
{
	uint16_t				xd;
	uint16_t				yd;
	uint16_t				uiWidth;
	uint16_t				uiHeight;
	uint8_t*				puiData;
	uint8_t					uiColour;
	struct SSpriteRowRun*	psRun;
	uint16_t				xs;
	uint16_t				uiSkipLength;
	uint16_t				uiRunLength;
	
	uiWidth = psSprite->uiImageWidth;
	uiHeight = psSprite->uiImageHeight;
	
	if (psSprite->uiType == SFT_TYPE_OPAQUE)
	{
		puiData = (void*)((uint32_t)psSprite + (uint32_t)(sizeof(struct SSpriteHeader)));
		
		for (yd = 0; yd < uiHeight; yd++)
		{
			for (xd = 0; xd < uiWidth; xd++)
			{
				DrawPixel(x, y, *puiData);
				puiData++;
				x++;
			}
			x -= uiWidth;
			y++;
		}
	}
	else if (psSprite->uiType == SFT_TYPE_TRANSPARENT)
	{
		puiData = (void*)((uint32_t)psSprite + (uint32_t)(sizeof(struct SSpriteHeader)));
		psRun = (struct SSpriteRowRun*)puiData;
		puiData = (void*)((uint32_t)puiData + (uint32_t)(sizeof(struct SSpriteRowRun)));
		
		xs = x;
		while ((psRun->uiFlags != 0) && (psRun->uiRunLength != 0))
		{
			if ((psRun->uiFlags & 0x8000) != 0)
			{
				x = xs;
				y++;
			}

			uiSkipLength = psRun->uiFlags & 0x7fff;
			uiRunLength = psRun->uiRunLength;
			x += uiSkipLength;
			for (xd = 0; xd <  uiRunLength; xd++)
			{
				DrawPixel(x, y, *puiData);
				puiData++;
				x++;
			}
	
			psRun = (struct SSpriteRowRun*)puiData;		
			puiData = (void*)((uint32_t)puiData + (uint32_t)(sizeof(struct SSpriteRowRun)));
		}
	}
}

