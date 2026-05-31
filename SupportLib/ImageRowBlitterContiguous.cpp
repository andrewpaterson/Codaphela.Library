#include "ImageRowBlitterContiguous.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterContiguous::Copy(CImageBlitterContext* pcContext, size iDestX, size iDestY, size iSourceXLeft, size iSourceXRight, size iSourceY)
{
	void*	pvSource;
	void*	pvDest;

	pvSource = RemapSinglePointer(pcContext->mpvSource, iSourceY * pcContext->miSourceRowStride + iSourceXLeft * pcContext->miSourcePixelStride);
	pvDest = RemapSinglePointer(pcContext->mpvDest, iDestY * pcContext->miDestRowStride + iDestX * pcContext->miDestPixelStride);
	memcpy_fast(pvDest, pvSource, (iSourceXRight - iSourceXLeft) * pcContext->miSourcePixelStride);
}

