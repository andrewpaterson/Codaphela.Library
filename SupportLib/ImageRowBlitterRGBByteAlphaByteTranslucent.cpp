#include "BaseLib/TypeNames.h"
#include "ImageRowBlitterRGBByteAlphaByteTranslucent.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterRGBByteAlphaByteTranslucent::Copy(CImageBlitterContext* pcContext, void* pvSource, void* pvDest, size iDestX, size iDestY, size iSourceXLeft, size iSourceXRight, size iSourceY)
{
	uint8*	puiAlpha;
	uint8*	puiSourceColour;
	uint8*	puiDestColour;
	size	uiSourceColourOffset;
	size	uiDestColourOffset;
	uint16	uiAlpha;
	uint16	uiInverseAlpha;
	uint16	uiSourceColour;
	uint16	uiDestColour;
	size	uiColourIndex;
	size	x;

	pvSource = RemapSinglePointer(pvSource, iSourceY * pcContext->miSourceRowStride + iSourceXLeft * pcContext->miSourcePixelStride);
	pvDest = RemapSinglePointer(pvDest, iDestY * pcContext->miDestRowStride + iDestX * pcContext->miDestPixelStride);

	for (x = iSourceXLeft; x < iSourceXRight; x++)
	{
		puiAlpha = (uint8*)RemapSinglePointer(pvSource, pcContext->muiSourceAlphaOffset);
		uiAlpha = (uint16)*puiAlpha;
		if (uiAlpha > 128)
		{
			uiAlpha++;
		}
		uiInverseAlpha = 256 - uiAlpha;

		uiSourceColourOffset = pcContext->muiSourceColourOffset;
		uiDestColourOffset = pcContext->muiDestColourOffset;
		for (uiColourIndex = 0; uiColourIndex < 3; uiColourIndex++)
		{
			puiSourceColour = (uint8*)RemapSinglePointer(pvSource, uiSourceColourOffset);
			puiDestColour = (uint8*)RemapSinglePointer(pvDest, uiDestColourOffset);

			uiSourceColour = (uint16)*puiSourceColour;
			uiDestColour = (uint16)*puiDestColour;

			uiDestColour = (uiSourceColour * uiAlpha) + (uiDestColour * uiInverseAlpha);
			uiDestColour >>= 8;
			*puiDestColour = (uint8)uiDestColour;

			uiSourceColourOffset++;
			uiDestColourOffset++;
		}

		pvSource = RemapSinglePointer(pvSource, pcContext->miSourcePixelStride);
		pvDest = RemapSinglePointer(pvDest, pcContext->miDestPixelStride);
	}

	if (pcContext->muiDestAlphaOffset != SIZE_MAX)
	{
		pvSource = RemapSinglePointer(pvSource, iSourceY * pcContext->miSourceRowStride + iSourceXLeft * pcContext->miSourcePixelStride);
		pvDest = RemapSinglePointer(pvDest, iDestY * pcContext->miDestRowStride + iDestX * pcContext->miDestPixelStride);

		for (x = iSourceXLeft; x < iSourceXRight; x++)
		{
			puiAlpha = (uint8*)RemapSinglePointer(pvSource, pcContext->muiSourceAlphaOffset);
			uiAlpha = (uint16)*puiAlpha;

			if (uiAlpha > 0)
			{
				puiDestColour = (uint8*)RemapSinglePointer(pvDest, pcContext->muiDestAlphaOffset);
				*puiDestColour = 255;
			}
		}

		pvSource = RemapSinglePointer(pvSource, pcContext->miSourcePixelStride);
		pvDest = RemapSinglePointer(pvDest, pcContext->miDestPixelStride);
	}
}

