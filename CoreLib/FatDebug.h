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
void		PrintFatFilenames(CChars* psz, char* szPath, CFatVolume* pcVolume);
void		DumpRootFatFilenames(CFatVolume* pcVolume);
void		PrintRootFatFilenames(CChars* psz, CFatVolume* pcVolume);
void		PrintFatDirectories(CChars* psz, char* szPath, CFatVolume* pcVolume);

EFatCode	PrintFATClusters(CChars* psz, CFatVolume* pcVolume, uint32 uiStartClusterIndex, uint32 uiEndClusterIndex);
EFatCode	PrintInterestingFATClusters(CChars* psz, CFatVolume* pcVolume);
void		DumpInterestingFATClusters(CFatVolume* pcVolume);
void		DumpRootDirectoryEntries(CFatVolume* pcVolume, bool bPrintTime = true);


#endif // !__FAT_DEBUG_H__

