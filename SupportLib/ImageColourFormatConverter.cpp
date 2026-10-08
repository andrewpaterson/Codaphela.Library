#include "ImageCopier.h"
#include "ImageColourFormatConverter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageColourFormatConverter::Convert(Ptr<CImage> pImage, CColourFormatHelper* pcHelper)
{
	ValidatePtr(pImage);

	Ptr<CImage>				pNewImage;
	int						iWidth;
	int						iHeight;

	if (pImage->Matches(pcHelper))
	{
		return pImage;
	}

	iWidth = pImage->GetWidth();
	iHeight = pImage->GetHeight();

	pNewImage = OMalloc<CImage>(iWidth, iHeight, pcHelper);

	CImageCopier::Copy(pImage, pNewImage);

	return pNewImage;
}

