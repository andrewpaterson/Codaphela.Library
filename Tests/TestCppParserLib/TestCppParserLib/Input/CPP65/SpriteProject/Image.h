#ifndef __IMAGE_H__
#define __IMAGE_H__
#include "stdint.h"

//A 320 x 200 byte a pixel screen (the test's stand in for the real Image.h).
#define SCREEN_WIDTH	320
#define SCREEN_HEIGHT	200
#define SCREEN_ADDRESS	0x110000


void DrawPixel(int16_t x, int16_t y, uint8_t uiColour);


#endif // __IMAGE_H__
