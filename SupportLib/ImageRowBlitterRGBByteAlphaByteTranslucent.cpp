#include "BaseLib/TypeNames.h"
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
	uint16	uiAlpha;
	uint16	uiInverseAlpha;
	uint16	uiSourceColour;
	uint16	uiDestColour;
	size	uiColourIndex;

	puiAlpha = (uint8*)RemapSinglePointer(pvSourcePixel, pcContext->muiSourceAlphaOffset);
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
		puiSourceColour = (uint8*)RemapSinglePointer(pvSourcePixel, uiSourceColourOffset);
		puiDestColour = (uint8*)RemapSinglePointer(pvDestPixel, uiDestColourOffset);

		uiSourceColour = (uint16)*puiSourceColour;
		uiDestColour = (uint16)*puiDestColour;

		uiDestColour = (uiSourceColour * uiAlpha) + (uiDestColour * uiInverseAlpha);
		uiDestColour >>= 8;
		*puiDestColour = (uint8)uiDestColour;

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

	//Source over destination: as + ad * (1 - as).  Dividing by 255 (rounded) rather than shifting keeps an opaque
	//destination opaque and leaves a transparent source's destination unchanged.
	//The colour is still blended as if the destination were opaque (see BlendRGBByteAlphaByte).
	puiSourceAlpha = (uint8*)RemapSinglePointer(pvSourcePixel, pcContext->muiSourceAlphaOffset);
	puiDestAlpha = (uint8*)RemapSinglePointer(pvDestPixel, pcContext->muiDestAlphaOffset);

	uiSourceAlpha = *puiSourceAlpha;
	uiDestAlpha = *puiDestAlpha;
	*puiDestAlpha = (uint8)(uiSourceAlpha + (uiDestAlpha * (255 - uiSourceAlpha) + 127) / 255);
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
CBaseImageRowSizingBlitter* CImageRowBlitterRGBByteAlphaByteTranslucent::GetSizingBlitter(CImageRowBlitterFactory* pcFactory)
{
	return pcFactory->CreateImageRowSizingBlitterRGBByteAlphaByteTranslucent();
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseImageRowSizingBlitter* CImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha::GetSizingBlitter(CImageRowBlitterFactory* pcFactory)
{
	return pcFactory->CreateImageRowSizingBlitterRGBByteAlphaByteTranslucentDestAlpha();
}

