#include "ImageRowBlitterContiguous.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterContiguous::Init(Ptr<CImage> pSource, Ptr<CImage> pDest)
{
	CImageRowBlitter::Init(pSource, pDest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterContiguous::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterContiguous::Copy(int32 iDestX, int32 iDestY, int32 iSourceXLeft, int32 iSourceXRight, int32 iSourceY)
{
	void*	pvSource;
	void*	pvDest;

	pvSource = mpSource->GetData();
	pvSource = RemapSinglePointer(pvSource, iSourceY * miSourceRowStride + iSourceXLeft * miSourcePixelStride);

	pvDest = mpDest->GetData();
	pvDest = RemapSinglePointer(pvDest, iDestY * miDestRowStride + iDestX * miDestPixelStride);
	memcpy_fast(pvDest, pvSource, (iSourceXRight - iSourceXLeft) * miSourcePixelStride);
}

