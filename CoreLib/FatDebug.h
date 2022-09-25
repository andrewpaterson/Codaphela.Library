#ifndef __FAT_DEBUG_H__
#include "FatCommon.h"


class CFatVolume;
void PrintInterestingFATClusters(CFatVolume* pcVolume);

void PrintRootDirectory(CFatVolume* pcVolume);

#endif // !__FAT_DEBUG_H__



