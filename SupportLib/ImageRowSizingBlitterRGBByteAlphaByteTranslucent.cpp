#include "BaseLib/TypeNames.h"
#include "ImageRowBlitterRGBByteAlphaByteTranslucent.h"
#include "ImageRowSizingBlitterRGBByteAlphaByteTranslucent.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowSizingBlitterRGBByteAlphaByteTranslucent::Copy(CImageBlitterContext* pcContext, void* pvSource, void* pvDest, size iDestXLeft, size iDestXRight, size iDestY, int32* paiSourceX, size iSourceY)
{
	size	x;
	void*	pvSourcePixel;

	pvSource = RemapSinglePointer(pvSource, iSourceY * pcContext->miSourceRowStride);
	pvDest = RemapSinglePointer(pvDest, iDestY * pcContext->miDestRowStride + iDestXLeft * pcContext->miDestPixelStride);

	for (x = iDestXLeft; x < iDestXRight; x++)
	{
		pvSourcePixel = RemapSinglePointer(pvSource, *paiSourceX * pcContext->miSourcePixelStride);
		BlendRGBByteAlphaByte(pcContext, pvSourcePixel, pvDest);

		paiSourceX++;
		pvDest = RemapSinglePointer(pvDest, pcContext->miDestPixelStride);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowSizingBlitterRGBByteAlphaByteTranslucentDestAlpha::Copy(CImageBlitterContext* pcContext, void* pvSource, void* pvDest, size iDestXLeft, size iDestXRight, size iDestY, int32* paiSourceX, size iSourceY)
{
	size	x;
	void*	pvSourcePixel;

	pvSource = RemapSinglePointer(pvSource, iSourceY * pcContext->miSourceRowStride);
	pvDest = RemapSinglePointer(pvDest, iDestY * pcContext->miDestRowStride + iDestXLeft * pcContext->miDestPixelStride);

	for (x = iDestXLeft; x < iDestXRight; x++)
	{
		pvSourcePixel = RemapSinglePointer(pvSource, *paiSourceX * pcContext->miSourcePixelStride);
		BlendRGBByteAlphaByte(pcContext, pvSourcePixel, pvDest);
		BlendAlphaByte(pcContext, pvSourcePixel, pvDest);

		paiSourceX++;
		pvDest = RemapSinglePointer(pvDest, pcContext->miDestPixelStride);
	}
}

