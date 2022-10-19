#ifndef __FAT_FILENAME_HELPER_H__
#define __FAT_FILENAME_HELPER_H__
#include <ctype.h>
#include "FatCommon.h"


// performs an ASCII stuff on UTF16 strings
EFatCode GetFatLongNameForEntry(uint16* puiDest, char* szSource);
bool CompareFatShortName(char* szName1, char* szName2);
char CompareFatLongName(uint16* puiName1, uint16* puiName2);
EFatCode GetFatShortNameForEntry(char* szDest, char* szSource, bool bLFNDisabled);
void ConvertFATShortInternalNameInto8Dot3Format(uint8* puiDest, const uint8* puiSource);
EFatCode MatchesFatFileName(bool* pbMatch, bool* pbUsingLFN, char* szConstructedShortFileName, uint16* puiTargetFileLongName, char* szCurrentLevelPath, uint16* auiLongFilename, uint8* auiShortName);
void ConstructFatLongFileNameFromShortName(uint16* auiLongFilename, char* szShortName, bool bLowercaseBase, bool bLowercaseExtension);
char* StepPathOverSlash(char* szPathName);
char* StepPathToSlash(char* szPathName);
void CopyLongFilenameIntoString(char* szDestination, uint16* auiSource);
bool IsIllegalFilenameCharacter(char c);
bool IsIllegalFilename(char* szName, uint16 uiLength);
void GenerateShortNameWithSuffix(uint16 uiNameSuffix, uint8* szShortName);

#endif // __FAT_FILENAME_HELPER_H__

