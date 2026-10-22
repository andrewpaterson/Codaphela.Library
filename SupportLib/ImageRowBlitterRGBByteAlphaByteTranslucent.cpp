#include "BaseLib/TypeNames.h"
#include "ColourBlend.h"
#include "ImageRowBlitterFactory.h"
#include "ImageRowBlitterRGBByteAlphaByteTranslucent.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void BlendRGBByteAlphaByte(CImageBlitterContext* pcContext, void* pvSourcePixel, void* pvDestPixel)
{
	uint8*	puiAlpha;
	uint8*	puiSourceColour;
	uint8*	puiDestColour;
	size	uiSourceColourOffset;
	size	uiDestColourOffset;
	uint32	uiAlpha;
	size	uiColourIndex;

	puiAlpha = (uint8*)RemapSinglePointer(pvSourcePixel, pcContext->muiSourceAlphaOffset);
	uiAlpha = *puiAlpha;

	uiSourceColourOffset = pcContext->muiSourceColourOffset;
	uiDestColourOffset = pcContext->muiDestColourOffset;
	for (uiColourIndex = 0; uiColourIndex < 3; uiColourIndex++)
	{
		puiSourceColour = (uint8*)RemapSinglePointer(pvSourcePixel, uiSourceColourOffset);
		puiDestColour = (uint8*)RemapSinglePointer(pvDestPixel, uiDestColourOffset);
		*puiDestColour = BlendLerpByte(*puiDestColour, *puiSourceColour, uiAlpha);

		uiSourceColourOffset++;
		uiDestColourOffset++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void BlendAlphaByte(CImageBlitterContext* pcContext, void* pvSourcePixel, void* pvDestPixel)
{
	uint8*	puiSourceAlpha;
	uint8*	puiDestAlpha;
	uint32	uiSourceAlpha;
	uint32	uiDestAlpha;

	//Source over destination: as + ad * (1 - as).
	//The colour is still blended as if the destination were opaque (see BlendRGBByteAlphaByte).
	puiSourceAlpha = (uint8*)RemapSinglePointer(pvSourcePixel, pcContext->muiSourceAlphaOffset);
	puiDestAlpha = (uint8*)RemapSinglePointer(pvDestPixel, pcContext->muiDestAlphaOffset);

	uiSourceAlpha = *puiSourceAlpha;
	uiDestAlpha = *puiDestAlpha;
	*puiDestAlpha = BlendOverByte(uiDestAlpha, uiSourceAlpha);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterRGBByteAlphaByteTranslucent::Copy(CImageBlitterContext* pcContext, void* pvSource, void* pvDest, size iDestX, size iDestY, size iSourceXLeft, size iSourceXRight, size iSourceY)
{
	size	x;

	pvSource = RemapSinglePointer(pvSource, iSourceY * pcContext->miSourceRowStride + iSourceXLeft * pcContext->miSourcePixelStride);
	pvDest = RemapSinglePointer(pvDest, iDestY * pcContext->miDestRowStride + iDestX * pcContext->miDestPixelStride);

	for (x = iSourceXLeft; x < iSourceXRight; x++)
	{
		BlendRGBByteAlphaByte(pcContext, pvSource, pvDest);

		pvSource = RemapSinglePointer(pvSource, pcContext->miSourcePixelStride);
		pvDest = RemapSinglePointer(pvDest, pcContext->miDestPixelStride);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha::Copy(CImageBlitterContext* pcContext, void* pvSource, void* pvDest, size iDestX, size iDestY, size iSourceXLeft, size iSourceXRight, size iSourceY)
{
	size	x;

	pvSource = RemapSinglePointer(pvSource, iSourceY * pcContext->miSourceRowStride + iSourceXLeft * pcContext->miSourcePixelStride);
	pvDest = RemapSinglePointer(pvDest, iDestY * pcContext->miDestRowStride + iDestX * pcContext->miDestPixelStride);

	for (x = iSourceXLeft; x < iSourceXRight; x++)
	{
		BlendRGBByteAlphaByte(pcContext, pvSource, pvDest);
		BlendAlphaByte(pcContext, pvSource, pvDest);

		pvSource = RemapSinglePointer(pvSource, pcContext->miSourcePixelStride);
		pvDest = RemapSinglePointer(pvDest, pcContext->miDestPixelStride);
	}
}

