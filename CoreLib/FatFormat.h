#ifndef __FAT_FORMAT_H__
#define __FAT_FORMAT_H__
#include <time.h>
#include "BaseLib/PrimitiveTypes.h"
#include "FileDrive.h"
#include "FatCommon.h"


uint16 FatFormat(EFatFileSystemType fs_type, char* const volume_label, uint32 uiNoOfSectorsPerCluster, CFileDrive* device);


#endif // !__FAT_FORMAT_H__
