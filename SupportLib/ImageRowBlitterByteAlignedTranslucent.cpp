#include "BaseLib/TypeNames.h"
#include "ImageRowBlitterByteAlignedTranslucent.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterByteAlignedTranslucent::Init(Ptr<CImage> pSource, Ptr<CImage> pDest, CColourFormatHelper* pcSourceFormatHelper, CColourFormatHelper* pcDestFormatHelper)
{
	EPrimitiveType	eType;
	size			uiTypeSize;

	CImageRowBlitter::Init(pSource, pDest);

	muiSourceColourOffset = pcSourceFormatHelper->GetFirstColourIndex();
	muiDestColourOffset = pcDestFormatHelper->GetFirstColourIndex();

	eType = pcSourceFormatHelper->GetColourType(0);
	uiTypeSize = gcTypeNames.GetByteSize(eType);

	muiSourceColourOffset *= uiTypeSize;
	muiDestColourOffset *= uiTypeSize;

	muiSourceAlphaOffset = pcSourceFormatHelper->GetAlphaIndex();

	eType = pcSourceFormatHelper->GetAlphaType();
	uiTypeSize = gcTypeNames.GetByteSize(eType);

	muiSourceAlphaOffset *= uiTypeSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterByteAlignedTranslucent::Kill(void)
{
}

