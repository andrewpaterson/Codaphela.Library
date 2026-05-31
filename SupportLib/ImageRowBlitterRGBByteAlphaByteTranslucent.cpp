#include "BaseLib/TypeNames.h"
#include "ImageRowBlitterRGBByteAlphaByteTranslucent.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterRGBByteAlphaByteTranslucent::Init(Ptr<CImage> pSource, Ptr<CImage> pDest, CColourFormatHelper* pcSourceFormatHelper, CColourFormatHelper* pcDestFormatHelper)
{
	PreInit();

	CImageRowBlitterByteAlignedTranslucent::Init(pSource, pDest, pcSourceFormatHelper, pcDestFormatHelper);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterRGBByteAlphaByteTranslucent::Free(void)
{
	CImageRowBlitterByteAlignedTranslucent::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterRGBByteAlphaByteTranslucent::Class(void)
{
	CImageRowBlitterByteAlignedTranslucent::Class();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageRowBlitterRGBByteAlphaByteTranslucent::Save(CObjectWriter* pcFile)
{
	return CImageRowBlitterByteAlignedTranslucent::Save(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageRowBlitterRGBByteAlphaByteTranslucent::Load(CObjectReader* pcFile)
{
	return CImageRowBlitterByteAlignedTranslucent::Load(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterRGBByteAlphaByteTranslucent::Copy(size iDestX, size iDestY, size iSourceXLeft, size iSourceXRight, size iSourceY)
{
	void*	pvSource;
	void*	pvDest;
	uint8*	puiAlpha;
	uint8*	puiSourceColour;
	uint8*	puiDestColour;
	size	uiSourceColourOffset;
	size	uiDestColourOffset;
	uint16	uiAlpha;
	uint16	uiInverseAlpha;
	uint16	uiSourceColour;
	uint16	uiDestColour;
	size	uiColourIndex;
	size	x;

	pvSource = mpSource->GetData();
	pvSource = RemapSinglePointer(pvSource, iSourceY * miSourceRowStride + iSourceXLeft * miSourcePixelStride);

	pvDest = mpDest->GetData();
	pvDest = RemapSinglePointer(pvDest, iDestY * miDestRowStride + iDestX * miDestPixelStride);

	for (x = iSourceXLeft; x < iSourceXRight; x++)
	{
		puiAlpha = (uint8*)RemapSinglePointer(pvSource, muiSourceAlphaOffset);
		uiAlpha = (uint16)*puiAlpha;
		if (uiAlpha > 128)
		{
			uiAlpha++;
		}
		uiInverseAlpha = 256 - uiAlpha;

		uiSourceColourOffset = muiSourceColourOffset;
		uiDestColourOffset = muiDestColourOffset;
		for (uiColourIndex = 0; uiColourIndex < 3; uiColourIndex++)
		{
			puiSourceColour = (uint8*)RemapSinglePointer(pvSource, uiSourceColourOffset);
			puiDestColour = (uint8*)RemapSinglePointer(pvDest, uiDestColourOffset);

			uiSourceColour = (uint16)*puiSourceColour;
			uiDestColour = (uint16)*puiDestColour;

			uiDestColour = (uiSourceColour * uiAlpha) + (uiDestColour * uiInverseAlpha);
			uiDestColour >>= 8;
			*puiDestColour = (uint8)uiDestColour;

			uiSourceColourOffset++;
			uiDestColourOffset++;
		}

		pvSource = RemapSinglePointer(pvSource, miSourcePixelStride);
		pvDest = RemapSinglePointer(pvDest, miDestPixelStride);
	}
}

