#ifndef __FAT_DEBUG_H__
#include "FatCommon.h"


class CFatVolume;

EFatCode	PrintRootDirectory(CChars* psz, CFatVolume* pcVolume, bool bPrintTimes);
void		PrintBiosParameterBlock(CChars* psz, CFatVolume* pcVolume);

void		DumpInterestingFATClusters(CFatVolume* pcVolume);
void		DumpRootDirectory(CFatVolume* pcVolume, bool bPrintTime = true);


#endif // !__FAT_DEBUG_H__

