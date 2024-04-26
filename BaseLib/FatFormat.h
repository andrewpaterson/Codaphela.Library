#ifndef __FAT_FORMAT_H__
#define __FAT_FORMAT_H__
#include "FatCommon.h"


class CFileDrive;
EFatCode FatFormat(EFatFileSystemType fs_type, char* const volume_label, uint32 uiNoOfSectorsPerCluster, CFileDrive* device);


#endif // __FAT_FORMAT_H__

