#include "BaseLib/TypeNames.h"
#include "ImageRowBlitterRGBByteAlphaByteTranslucent.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterRGBByteAlphaByteTranslucent::Init(Ptr<CImage> pSource, Ptr<CImage> pDest, CColourFormatHelper* pcSourceFormatHelper, CColourFormatHelper* pcDestFormatHelper)
{
	CImageRowBlitter::Init(pSource, pDest);

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterRGBByteAlphaByteTranslucent::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRowBlitterRGBByteAlphaByteTranslucent::Copy(int32 iDestX, int32 iDestY, int32 iSourceXLeft, int32 iSourceXRight, int32 iSourceY)
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
	size	ui;

	pvSource = mpSource->GetData();
	pvSource = RemapSinglePointer(pvSource, iSourceY * miSourceRowStride + iSourceXLeft * miSourcePixelStride);

	pvDest = mpDest->GetData();
	pvDest = RemapSinglePointer(pvDest, iDestY * miDestRowStride + iDestX * miDestPixelStride);

	uiSourceColourOffset = muiSourceColourOffset;
	uiDestColourOffset = muiDestColourOffset;

	puiAlpha = (uint8*)RemapSinglePointer(pvSource, muiSourceAlphaOffset);

	uiAlpha = (uint16)*puiAlpha;
	if (uiAlpha > 128)
	{
		uiAlpha++;
	}
	uiInverseAlpha = 256 - uiAlpha;
	
	for (ui = 0; ui < 3; ui++)
	{
		puiSourceColour = (uint8*)RemapSinglePointer(pvSource, uiSourceColourOffset);
		puiDestColour = (uint8*)RemapSinglePointer(pvSource, uiDestColourOffset);

		uiSourceColour = (uint16)*puiSourceColour;
		uiDestColour = (uint16)*puiDestColour;

		uiDestColour = (uiSourceColour * uiAlpha) + (uiDestColour * uiInverseAlpha);
		uiDestColour >>= 8;
		*puiDestColour = (uint8)uiDestColour;

		uiSourceColourOffset++;
		uiDestColourOffset++;
	}
}

