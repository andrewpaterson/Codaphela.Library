#ifndef __FAT_IMAGE_FILE_H__
#define __FAT_IMAGE_FILE_H__
#include "PrimitiveTypes.h"


//Writes uiSize bytes as szName (a FAT path, \boot.exe or \Games\modplay.exe) on the FAT volume in the disk image file
//szImage (an SD card's, as an emulator uses it), replacing the file if it is there; the folders it is in are made if
//they aren't.  The image is read whole, changed and written back.
bool FatPutFileOnImage(const char* szImage, const char* szName, void* pvData, size uiSize);

//The same with the bytes of the file szFile (a program built on this computer, a song).
bool FatPutDiskFileOnImage(const char* szImage, const char* szName, const char* szFile);

//The bytes of the file szName on the FAT volume in the disk image file szImage, malloc'ed in *ppvData (free it), and
//how many.  Only the sectors it needs are read.  False (*ppvData NULL) if the image can't be read, has no FAT volume or
//hasn't the file.
bool FatGetFileFromImage(const char* szImage, const char* szName, void** ppvData, size* puiSize);


#endif // __FAT_IMAGE_FILE_H__

