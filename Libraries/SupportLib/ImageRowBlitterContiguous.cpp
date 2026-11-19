#include "ImageRowBlitterFactory.h"
#include "ImageRowBlitterContiguous.h"
#include "BaseLib/Memory32.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterContiguous::Copy(CImageBlitterContext* pcContext, void* pvSource, void* pvDest, size iDestX, size iDestY, size iSourceXLeft, size iSourceXRight, size iSourceY)
{
	pvSource = RemapSinglePointer(pvSource, iSourceY * pcContext->miSourceRowStride + iSourceXLeft * pcContext->miSourcePixelStride);
	pvDest = RemapSinglePointer(pvDest, iDestY * pcContext->miDestRowStride + iDestX * pcContext->miDestPixelStride);
	memcpy32(pvDest, pvSource, (lsize)(iSourceXRight - iSourceXLeft) * pcContext->miSourcePixelStride);
}

