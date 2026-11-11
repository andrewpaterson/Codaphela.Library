#include "Image.h"


void DrawPixel(int16_t x, int16_t y, uint8_t uiColour)
{
	uint8_t*	puiScreen;

	if ((x < 0) || (x >= SCREEN_WIDTH) || (y < 0) || (y >= SCREEN_HEIGHT))
	{
		return;
	}
	puiScreen = (uint8_t*)SCREEN_ADDRESS;
	puiScreen[(int32_t)y * SCREEN_WIDTH + x] = uiColour;
}
