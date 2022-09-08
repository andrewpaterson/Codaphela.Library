#ifndef __FAT_SHARED_BUFFER_H__
#define __FAT_SHARED_BUFFER_H__
#include "BaseLib/PrimitiveTypes.h"
#include "FatCommon.h"


// declare shared buffer
extern uint8 fat_shared_buffer[MAX_SECTOR_LENGTH];
extern uint32 fat_shared_buffer_sector;


#endif // !__FAT_SHARED_BUFFER_H__
