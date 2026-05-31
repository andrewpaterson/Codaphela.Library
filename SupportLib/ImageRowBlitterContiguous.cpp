#include "ImageRowBlitterContiguous.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterContiguous::Copy(CImageBlitterContext* pcContext, void* pvSource, void* pvDest, size iDestX, size iDestY, size iSourceXLeft, size iSourceXRight, size iSourceY)
{
	pvSource = RemapSinglePointer(pvSource, iSourceY * pcContext->miSourceRowStride + iSourceXLeft * pcContext->miSourcePixelStride);
	pvDest = RemapSinglePointer(pvDest, iDestY * pcContext->miDestRowStride + iDestX * pcContext->miDestPixelStride);
	memcpy_fast(pvDest, pvSource, (iSourceXRight - iSourceXLeft) * pcContext->miSourcePixelStride);
}

