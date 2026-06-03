#include "BaseLib/TypeNames.h"
#include "ImageRowBlitterByteAlignedOpaque.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterByteAlignedOpaque::Copy(CImageBlitterContext* pcContext, void* pvSource, void* pvDest, size iDestX, size iDestY, size iSourceXLeft, size iSourceXRight, size iSourceY)
{
	size	x;
	void*	pvAlphaDest;

	pvAlphaDest = pvDest;

	pvSource = RemapSinglePointer(pvSource, iSourceY * pcContext->miSourceRowStride + iSourceXLeft * pcContext->miSourcePixelStride + pcContext->muiSourceColourOffset);
	pvDest = RemapSinglePointer(pvDest, iDestY * pcContext->miDestRowStride + iDestX * pcContext->miDestPixelStride + pcContext->muiDestColourOffset);

	for (x = iSourceXLeft; x < iSourceXRight; x++)
	{
		ga_memcpy_fast[pcContext->muiColourWidth](pvDest, pvSource);

		pvSource = RemapSinglePointer(pvSource, pcContext->miSourcePixelStride);
		pvDest = RemapSinglePointer(pvDest, pcContext->miDestPixelStride);
	}

	if (pcContext->muiDestAlphaOffset != SIZE_MAX)
	{
		pvDest = RemapSinglePointer(pvAlphaDest, iDestY * pcContext->miDestRowStride + iDestX * pcContext->miDestPixelStride + pcContext->muiDestAlphaOffset);

		for (x = iSourceXLeft; x < iSourceXRight; x++)
		{
			*(uint8*)pvDest = 255;
			pvDest = RemapSinglePointer(pvDest, pcContext->miDestPixelStride);
		}
	}
}

