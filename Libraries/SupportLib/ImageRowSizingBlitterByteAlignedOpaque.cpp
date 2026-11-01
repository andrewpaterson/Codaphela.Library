#include "BaseLib/TypeNames.h"
#include "ImageRowSizingBlitterByteAlignedOpaque.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowSizingBlitterByteAlignedOpaque::Copy(CImageBlitterContext* pcContext, void* pvSource, void* pvDest, size iDestXLeft, size iDestXRight, size iDestY, int32* paiSourceX, size iSourceY)
{
	size	x;
	void*	pvAlphaDest;
	void*	pvSourcePixel;

	pvAlphaDest = pvDest;

	pvSource = RemapSinglePointer(pvSource, iSourceY * pcContext->miSourceRowStride + pcContext->muiSourceColourOffset);
	pvDest = RemapSinglePointer(pvDest, iDestY * pcContext->miDestRowStride + iDestXLeft * pcContext->miDestPixelStride + pcContext->muiDestColourOffset);

	for (x = iDestXLeft; x < iDestXRight; x++)
	{
		pvSourcePixel = RemapSinglePointer(pvSource, *paiSourceX * pcContext->miSourcePixelStride);
		memcpy(pvDest, pvSourcePixel, pcContext->muiColourWidth);

		paiSourceX++;
		pvDest = RemapSinglePointer(pvDest, pcContext->miDestPixelStride);
	}

	if (pcContext->muiDestAlphaOffset != MAX_SIZE)
	{
		pvDest = RemapSinglePointer(pvAlphaDest, iDestY * pcContext->miDestRowStride + iDestXLeft * pcContext->miDestPixelStride + pcContext->muiDestAlphaOffset);

		for (x = iDestXLeft; x < iDestXRight; x++)
		{
			*(uint8*)pvDest = 255;
			pvDest = RemapSinglePointer(pvDest, pcContext->miDestPixelStride);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowSizingBlitterByteAlignedOpaqueDestAlpha::Copy(CImageBlitterContext* pcContext, void* pvSource, void* pvDest, size iDestXLeft, size iDestXRight, size iDestY, int32* paiSourceX, size iSourceY)
{
	size	x;
	void*	pvAlphaDest;
	void*	pvSourcePixel;

	pvAlphaDest = pvDest;

	pvSource = RemapSinglePointer(pvSource, iSourceY * pcContext->miSourceRowStride + pcContext->muiSourceColourOffset);
	pvDest = RemapSinglePointer(pvDest, iDestY * pcContext->miDestRowStride + iDestXLeft * pcContext->miDestPixelStride + pcContext->muiDestColourOffset);
	pvAlphaDest = RemapSinglePointer(pvAlphaDest, iDestY * pcContext->miDestRowStride + iDestXLeft * pcContext->miDestPixelStride + pcContext->muiDestAlphaOffset);

	for (x = iDestXLeft; x < iDestXRight; x++)
	{
		pvSourcePixel = RemapSinglePointer(pvSource, *paiSourceX * pcContext->miSourcePixelStride);
		memcpy(pvDest, pvSourcePixel, pcContext->muiColourWidth);
		*(uint8*)pvAlphaDest = 255;

		paiSourceX++;
		pvDest = RemapSinglePointer(pvDest, pcContext->miDestPixelStride);
		pvAlphaDest = RemapSinglePointer(pvAlphaDest, pcContext->miDestPixelStride);
	}
}

