#include "BaseLib/TypeNames.h"
#include "ImageRowBlitterByteAlignedOpaque.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterByteAlignedOpaque::Copy(CImageBlitterContext* pcContext, size iDestX, size iDestY, size iSourceXLeft, size iSourceXRight, size iSourceY)
{
	void*	pvSource;
	void*	pvDest;
	size	x;

	pvSource = RemapSinglePointer(pcContext->mpvSource, iSourceY * pcContext->miSourceRowStride + iSourceXLeft * pcContext->miSourcePixelStride + pcContext->muiSourceColourOffset);
	pvDest = RemapSinglePointer(pcContext->mpvDest, iDestY * pcContext->miDestRowStride + iDestX * pcContext->miDestPixelStride + pcContext->muiDestColourOffset);

	for (x = iSourceXLeft; x < iSourceXRight; x++)
	{
		ga_memcpy_fast[pcContext->muiColourWidth](pvDest, pvSource);

		pvSource = RemapSinglePointer(pvSource, pcContext->miSourcePixelStride);
		pvDest = RemapSinglePointer(pvDest, pcContext->miDestPixelStride);
	}
}

