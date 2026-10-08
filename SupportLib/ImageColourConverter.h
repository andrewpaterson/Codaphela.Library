#ifndef __IMAGE_COLOUR_FORMAT_CONVERTER_H__
#define __IMAGE_COLOUR_FORMAT_CONVERTER_H__
#include "Image.h"


class CImageColourConverter
{
public:
	static Ptr<CImage>	ConvertFormat(Ptr<CImage> pImage, CColourFormatHelper* pcHelper);
	static Ptr<CImage>	ConvertColour(Ptr<CImage> pImage, EColourOrder eOrder);
};


#endif // __IMAGE_COLOUR_FORMAT_CONVERTER_H__

