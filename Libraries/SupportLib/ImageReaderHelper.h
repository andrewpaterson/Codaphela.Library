#ifndef __IMAGE_READER_HELPER_H__
#define __IMAGE_READER_HELPER_H__
#include "Image.h"


void AddDebugChannel(Ptr<CImage> pImage, EPrimitiveType eType, EChannel eChannel, bool bAddDebug);


//The image readers' allocation: named szObjectName when there is one (an image converted from a file into the Codabase).
template<typename ... Args>
Ptr<CImage> ImageMalloc(const char* szObjectName, Args ... args)
{
	if (szObjectName)
	{
		return ONMalloc<CImage>(szObjectName, args...);
	}
	return OMalloc<CImage>(args...);
}


#endif // __IMAGE_READER_HELPER_H__

