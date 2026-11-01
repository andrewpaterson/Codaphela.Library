#ifndef __IMAGE_H__
#define __IMAGE_H__
#include "inttypes.h"

extern uint16_t		guiImageWidth;
extern uint16_t		guiImageHeight;
extern uint8_t* __restrict	guiImageMemory;
extern uint16_t		guiImageWidthMinusOne;
extern uint16_t		guiImageHeightMinusOne;


void 		SetImageParameters(uint8_t* uiImageMemory, uint16_t guiImageWidth, uint16_t uiImageHeight);
void 		DrawPixel(int16_t x, int16_t y, uint8_t uiColour);
uint8_t* 	GetImageMemory(void);
uint16_t	GetImageWidth(void);
uint16_t	GetImageHeight(void);


#endif // __IMAGE_H__

