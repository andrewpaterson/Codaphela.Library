#ifndef __FAT_DEBUG_H__
#include "FatCommon.h"


class CFatVolume;
class CArrayChars;

EFatCode	PrintRootDirectoryEntries(CChars* psz, CFatVolume* pcVolume, bool bPrintTimes);
void		PrintBiosParameterBlock(CChars* psz, CFatVolume* pcVolume);

void		DumpAllFatFilenames(CFatVolume* pcVolume);
void		PrintAllFatFilenames(CChars* psz, CFatVolume* pcVolume);
EFatCode	RecurseFindFatFilenames(CFatVolume* pcVolume, char* szPath, CArrayChars* paszFiles);
EFatCode	RecurseFindFatDirectories(CFatVolume* pcVolume, char* szPath, CArrayChars* paszDirectories);

void		DumpInterestingFATClusters(CFatVolume* pcVolume);
void		DumpRootDirectoryEntries(CFatVolume* pcVolume, bool bPrintTime = true);

void		DumpRootFatFilenames(CFatVolume* pcVolume);
void		PrintRootFatFilenames(CChars* psz, CFatVolume* pcVolume);

#endif // !__FAT_DEBUG_H__

