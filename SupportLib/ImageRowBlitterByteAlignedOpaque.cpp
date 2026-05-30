#include "BaseLib/TypeNames.h"
#include "ImageRowBlitterByteAlignedOpaque.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterByteAlignedOpaque::Init(Ptr<CImage> pSource, Ptr<CImage> pDest, CColourFormatHelper* pcSourceFormatHelper, CColourFormatHelper* pcDestFormatHelper)
{
	EPrimitiveType	eType;
	size			uiTypeSize;

	CBaseImageRowBlitter::Init(pSource, pDest);

	muiSourceColourOffset = pcSourceFormatHelper->GetFirstColourIndex();
	muiDestColourOffset = pcDestFormatHelper->GetFirstColourIndex();

	eType = pcSourceFormatHelper->GetColourType(0);
	uiTypeSize = gcTypeNames.GetByteSize(eType);

	muiSourceColourOffset *= uiTypeSize;
	muiDestColourOffset *= uiTypeSize;

	muiColourWidth = 3 * uiTypeSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterByteAlignedOpaque::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterByteAlignedOpaque::Copy(int32 iDestX, int32 iDestY, int32 iSourceXLeft, int32 iSourceXRight, int32 iSourceY)
{
	void*	pvSource;
	void*	pvDest;
	int32	x;

	pvSource = mpSource->GetData();
	pvSource = RemapSinglePointer(pvSource, iSourceY * miSourceRowStride + iSourceXLeft * miSourcePixelStride);

	pvDest = mpDest->GetData();
	pvDest = RemapSinglePointer(pvDest, iDestY * miDestRowStride + iDestX * miDestPixelStride);

	for (x = iSourceXLeft; x <= iSourceXRight; x++)
	{
		ga_memcpy_fast[muiColourWidth](pvDest, pvSource);

		pvSource = RemapSinglePointer(pvSource, miSourcePixelStride);
		pvDest = RemapSinglePointer(pvSource, miDestPixelStride);
	}
}

