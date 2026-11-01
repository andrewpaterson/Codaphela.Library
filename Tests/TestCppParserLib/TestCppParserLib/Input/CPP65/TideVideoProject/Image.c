#include "Image.h"


uint16_t	guiImageWidth;
uint16_t	guiImageHeight;
uint8_t* __restrict	guiImageMemory;
uint16_t	guiImageWidthMinusOne;
uint16_t	guiImageHeightMinusOne;


void SetImageParameters(uint8_t* uiImageMemory, uint16_t uiImageWidth, uint16_t uiImageHeight)
{
	guiImageMemory = uiImageMemory;
	guiImageWidth = uiImageWidth;
	guiImageWidthMinusOne = uiImageWidth-1;
	guiImageHeight = uiImageHeight;
	guiImageHeightMinusOne = uiImageHeight-1;
	return;
}


void DrawPixel(int16_t x, int16_t y, uint8_t uiColour)
{
	if ((x < 0) || (x > guiImageWidthMinusOne))
	{
		return;
	}
	if ((y < 0) || (y > guiImageHeightMinusOne))
	{
		return;
	}

	*(guiImageMemory + (y * guiImageWidth) + x) = uiColour;
}


uint8_t* GetImageMemory(void)
{
	return guiImageMemory;
}


uint16_t GetImageWidth(void)
{
	return guiImageWidth;
}


uint16_t GetImageHeight(void)
{
	return guiImageHeight;
}


