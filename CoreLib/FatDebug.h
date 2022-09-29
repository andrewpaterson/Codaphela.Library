#ifndef __FAT_DEBUG_H__
#include "FatCommon.h"


class CFatVolume;
void PrintInterestingFATClusters(CFatVolume* pcVolume);
EFatCode PrintRootDirectory(CChars* psz, CFatVolume* pcVolume, bool bPrintTimes);

void DumpRootDirectory(CFatVolume* pcVolume);


#endif // !__FAT_DEBUG_H__


