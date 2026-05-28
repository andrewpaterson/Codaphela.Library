#include "ImageRowBlitter.h"

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitter::Init(Ptr<CImage> pSource, Ptr<CImage> pDest)
{
	mpSource = pSource;
	mpDest = pDest;

	miSourcePixelStride = (size)pSource->GetPixelByteStride();
	miDestPixelStride = (size)pDest->GetPixelByteStride();
	miSourceWidth = pSource->GetWidth();
	miDestWidth = pDest->GetWidth();
	miSourceRowStride = miSourceWidth * miSourcePixelStride;
	miDestRowStride = miDestWidth * miDestPixelStride;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitter::Kill(void)
{
	mpSource = NULL;
	mpDest = NULL;
}

