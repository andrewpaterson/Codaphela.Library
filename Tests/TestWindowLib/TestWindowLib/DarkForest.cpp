#include "SupportLib/ImageColourConverter.h"
#include "WindowLib/ImageCanvasDraw.h"
#include "BorderCanvasDraw.h"
#include "SpaceCrusade.h"
#include "DarkForest.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> ReadDarkForestCel(const char* szFilename, CColourFormatHelper* pcFormat)
{
	Ptr<CImage>		pImage;

	pImage = ReadSpaceImage("DarkForest", (char*)szFilename);
	pImage = CImageColourConverter::ConvertColour(pImage, pcFormat->meOrder);
	return OMalloc<CImageCel>(pImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCanvasDraw> CreateDarkForestDraw(SDarkForestComponent* psComponent, CColourFormatHelper* pcFormat)
{
	Ptr<CImageCel>	pCel;

	if (psComponent->szImage)
	{
		pCel = ReadDarkForestCel(psComponent->szImage, pcFormat);
		return OMalloc<CImageCanvasDraw>(pCel);
	}
	else
	{
		return OMalloc<CBorderCanvasDraw>(psComponent->sBorderColour);
	}
}

