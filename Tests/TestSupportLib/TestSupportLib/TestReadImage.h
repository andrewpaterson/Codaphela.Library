#ifndef __TEST_READ_IMAGE_H__
#define __TEST_READ_IMAGE_H__
#include "StandardLib/Pointer.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageCel.h"
#include "SupportLib/ImageDivider.h"


Ptr<CImage>			TestReadImage(char* szDirectory, char* szFilename);
Ptr<CArrayImageCel> TestReadCels(char* szDirectory, char* szFilename, int iColumnCount, int iRowCount);


#endif // __TEST_READ_IMAGE_H__

