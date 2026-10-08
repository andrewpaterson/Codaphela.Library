#ifndef __IMAGE_COLOUR_FORMAT_CONVERTER_H__
#define __IMAGE_COLOUR_FORMAT_CONVERTER_H__
#include "Image.h"


class CImageColourFormatConverter
{
public:
	static Ptr<CImage>	Convert(Ptr<CImage> pImage, CColourFormatHelper* pcHelper);
};


#endif // __IMAGE_COLOUR_FORMAT_CONVERTER_H__

