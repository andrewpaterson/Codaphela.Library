#include "BaseLib/TypeNames.h"
#include "ImageBlitterContext.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBlitterContext::Init(void)
{
	memset(this, 0, sizeof(*this));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBlitterContext::Init(Ptr<CImage> pSource, Ptr<CImage> pDest)
{
	ValidatePtr(pSource);
	ValidatePtr(pDest);

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
void CImageBlitterContext::Init(Ptr<CImage> pSource, Ptr<CImage> pDest, CColourFormatHelper* pcSourceFormatHelper, CColourFormatHelper* pcDestFormatHelper)
{
	EPrimitiveType	eType;
	size			uiTypeSize;

	Init(pSource, pDest);

	muiSourceColourOffset = pcSourceFormatHelper->GetFirstColourIndex();
	muiDestColourOffset = pcDestFormatHelper->GetFirstColourIndex();

	eType = pcSourceFormatHelper->GetColourType(0);
	uiTypeSize = gcTypeNames.GetByteSize(eType);

	muiSourceColourOffset *= uiTypeSize;
	muiDestColourOffset *= uiTypeSize;
	muiColourWidth = 3 * uiTypeSize;

	muiSourceAlphaOffset = pcSourceFormatHelper->GetAlphaIndex();
	if (muiSourceAlphaOffset != MAX_SIZE)
	{
		eType = pcSourceFormatHelper->GetAlphaType();
		uiTypeSize = gcTypeNames.GetByteSize(eType);
		muiSourceAlphaOffset *= uiTypeSize;
	}

	muiDestAlphaOffset = pcDestFormatHelper->GetAlphaIndex();
	if (muiDestAlphaOffset != MAX_SIZE)
	{
		//The source may have no alpha at all so the destination alpha offset must be sized by the destination alpha type.
		eType = pcDestFormatHelper->GetAlphaType();
		uiTypeSize = gcTypeNames.GetByteSize(eType);
		muiDestAlphaOffset *= uiTypeSize;
	}
}

